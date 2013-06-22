/*
 * stream.hpp
 *
 *  Created on: Jun 6, 2013
 *      Author: ishafer
 */

#ifndef STREAM_HPP_
#define STREAM_HPP_

#include <boost/container/vector.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/range/algorithm/lower_bound.hpp>

#include "bs_file.hpp"
#include "util.hpp"

/**
 * A buffered value stream
 */
class ValueStream {
private:
	DISALLOW_EVIL_CONSTRUCTORS(ValueStream);
	boost::shared_ptr<BS> valuestore;

	int bufsize;
	boost::circular_buffer<valuet> buf;

	boost::mutex mutex;
	valuet lastvalue;
	indext values_in_buffer; //size of buffer if encoded
	streaminfo info;

	void init(int _bufsize) {
		bufsize = _bufsize;
		buf = boost::circular_buffer<valuet>();
		buf.set_capacity(_bufsize);
		lastvalue = 0;
		values_in_buffer = 0;
	}

public:
	ValueStream(MDS mds, streamid id, int _bufsize) : info(mds.get_info(id)) {
		valuestore = boost::shared_ptr<BS>(new BSFile(mds, id));
		buf.set_capacity(bufsize);
		init(_bufsize);
	}
	ValueStream(streaminfo _info, int _bufsize) : info(_info) {
		valuestore = boost::shared_ptr<BS>(new BSFile(_info));
		init(_bufsize);
	}
	~ValueStream() {
	}

	void debug_print() {
		std::cout << "[";
		std::copy(buf.begin(), buf.end(), std::ostream_iterator<valuet>(std::cout, " "));
		std::cout << "]" << std::endl;
	}

	/**
	 * Don't use me for anything other than debugging.
	 */
	valuet* debug_get_contents() {
		return buf.linearize();
	}

	void add_values(valuet *vals, int nvals) {
		add_start();
		for (int i = 0; i < nvals; ++i) {
			add_value(vals[i]);
		}
		add_end();
	}

	/**
	 * Start adding a batch of values
	 */
	void add_start() {
		mutex.lock();
	}

	/**
	 * Add a single value
	 */
	void add_value(valuet val) {
		++values_in_buffer;

		if (info.encoder == NONE) {
			buf.push_back(val);
		} else if (info.encoder == DELTARLE) {
			DEBUG("add val=" << val);
			indext size = buf.size();
			valuet delta = val - lastvalue;
			lastvalue = val;
			if (size == 0) {
				DEBUG("  case firstval");
				buf.push_back(val);
			} else if (size <= 2) {
				DEBUG("  case begin");
				buf.push_back(delta);
			} else {
				if ( (buf[size-2] == buf[size-3]) ) {
					DEBUG("  case inrun");
					if (delta == buf[size-2]) {
						//increment run count
						buf[size-1]++;
					} else {
						buf.push_back(delta);
					}
				} else if ( (buf[size-1] == buf[size-2]) ) {
					DEBUG("  case startrun");
					//if incoming val is same as last two, we're in a run.
					if (delta == buf[size-1]) {
						buf.push_back(1);
					} else {
						buf.push_back(0);
						buf.push_back(delta);
					}
				} else {
					DEBUG("  case newdelta");
					buf.push_back(delta);
				}
			}
		}
	}

	/**
	 * Finish adding a batch of values
	 */
	void add_end() {
		mutex.unlock();
	}

	/**
	 * Look up the position of a value
	 * @return index if found, -(index+1) if an insertion point in sorted arr.
	 */
	indext index(valuet val) {
		//TODO: hold mutex?
		indext lb;

		if (info.sorted) {
			if (info.encoder == NONE) {
				if (buf.empty() || val < buf.front()) {
					//off the bottom
					return valuestore->index(val);
				}
				else if (val > buf.back()) {
					//off the top
					return -(valuestore->get_maxindex() + buf.size() + 1);
				} else {
					lb = boost::range::lower_bound(buf, val) - buf.begin();
					if (buf[lb] == val) {
						return lb + valuestore->get_maxindex();
					} else {
						return -(lb + valuestore->get_maxindex() + 1);
					}
				}
			} else if (info.encoder == DELTARLE) {
				//scan the buffer
				valuet cur = buf[0];
				if (val < cur) {
					return valuestore->index(val);
				} else {
					indext decpos = valuestore->get_maxindex();
					for (indext i = 1; i < buf.size(); ++i) {
						cur += buf[i]; ++decpos;
						if (cur > val) {
							return decpos;
						}
						if (i > 1 && buf[i] == buf[i-1]) {
							++i; //advance to the run count
							//now buf[i] is the run count, buf[i-1] is the increment
							if (val <= cur + buf[i]*buf[i-1]) {
								indext pos = decpos + (val-cur)/buf[i-1];
								if ((val - cur) % buf[i-1] == 0) {
									return pos;
								} else {
									return -(pos+1);
								}
							} else {
								decpos += buf[i];
								cur += buf[i]*buf[i-1];
							}
						}
					}
					return -(decpos+1);
				}
			} else {
				//unknown encoder
				ERROR("Unknown encoder" << info.encoder);
				return -1;
			}
		} else {
			//scan the buffer first
			for (indext i = 0; i < buf.size(); ++i) {
				if (val == buf[i]) {
					return i + valuestore->get_maxindex();
				}
			}

			return valuestore->index(val);
		}
	}

	/**
	 * Flush out the entire circular buffer.
	 * 	TODO: consider different flush sizes
	 */
	bool flush() {
		boost::mutex::scoped_lock lock(mutex);

		bool success = true;

		//if we could break it up into two writes...
		/*
		boost::circular_buffer<valuet>::array_range ar = buf.array_one();
		success &= valuestore->add(ar.first, ar.second, 0);
		ar = buf.array_two();
		success &= valuestore->add(ar.first, ar.second, values_in_buffer);
		*/

		//instead, write in one chunk.
		valuestore->add(buf.linearize(), buf.size(), values_in_buffer);

		buf.clear();
		values_in_buffer = 0;

		lock.unlock();

		return success;
	}

	dxrange read(valuet *pts, dxrange req) {
		indext dxmin = req.start;
		indext npts = req.len;
		//try to get points from the stored stream
		dxrange vsres = valuestore->read_exact(pts, dxrange(dxmin, npts));
		indext vs_maxdx = valuestore->get_maxindex();

		if (dxmin + npts > vs_maxdx) {
			//read vals from buffer
			indext start = std::max((indext) 0, dxmin - vs_maxdx);
			indext end = std::min((indext) buf.size(), (dxmin + npts) - vs_maxdx);

			if (info.encoder == NONE) {
				for (indext i = start; i < end; ++i) {
					pts[vsres.len++] = buf[i];
				}
			} else if (info.encoder == DELTARLE) {
				if (end > 0) {
					valuet cur = buf[0];
					//i is position in the buffer, decpos is decoded pos
					indext decpos = 0;
					if (decpos >= start) {
						pts[vsres.len++] = cur;
					}
					for (indext i = 1; i < buf.size(); ++i) {
						cur += buf[i]; ++decpos;
						if (decpos >= start) {
							pts[vsres.len++] = cur;
						}
						if (i > 1 && buf[i] == buf[i-1]) {
							++i; //advance to the run count
							for (indext rdx = 0; rdx < buf[i]; ++rdx) {
								cur += buf[i-1]; ++decpos;
								if (decpos >= start) {
									pts[vsres.len++] = cur;
								}
							}
						}
					}
				}
			} //ENCODER
		}

		return vsres;
	}
};

#endif /* STREAM_HPP_ */
