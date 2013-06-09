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

#include "bs_uncompressed_file.hpp"
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

public:
	ValueStream(MDS mds, streamid id, int _bufsize) : bufsize(_bufsize), buf() {
		valuestore = boost::shared_ptr<BS>(new BSUncompressedFile(mds, id));
		buf.set_capacity(bufsize);
	}
	ValueStream(BS *bs, int _bufsize) : bufsize(_bufsize), buf() {
		valuestore = boost::shared_ptr<BS>(bs);
		buf.set_capacity(bufsize);
	}
	~ValueStream() {
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
	inline void add_value(valuet val) {
		buf.push_back(val);
	}

	/**
	 * Finish adding a batch of values
	 */
	void add_end() {
		mutex.unlock();
	}

	/**
	 * Flush out the entire circular buffer.
	 * 	TODO: consider different flush sizes
	 */
	bool flush() {
		boost::mutex::scoped_lock lock(mutex);

		bool success = true;

		boost::circular_buffer<valuet>::array_range ar = buf.array_one();
		success &= valuestore->add(ar.first, ar.second);
		ar = buf.array_two();
		success &= valuestore->add(ar.first, ar.second);

		lock.unlock();

		success &= valuestore->flush();

		return success;
	}

	int64_t read(valuet *pts, int64_t dxmin, int64_t npts) {
		//try to get points from the stored stream
		int64_t ptsread = valuestore->read(pts, dxmin, npts);
		int64_t vs_maxdx = valuestore->get_maxindex();

		if (dxmin + npts > vs_maxdx) {
			//read vals from buffer
			int64_t addl = min((int64_t) buf.size(), (dxmin + npts) - vs_maxdx);

			for (int64_t i = 0; i < addl; ++i) {
				pts[ptsread++] = buf[i];
			}
		}

		return ptsread;
	}
};

#endif /* STREAM_HPP_ */
