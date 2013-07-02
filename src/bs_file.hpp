/*
 * stream.hpp
 *
 *  Created on: Jun 5, 2013
 *      Author: ishafer
 */

#ifndef BS_FILE_HPP_
#define BS_FILE_HPP_

#include <string>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/assert.hpp>
#include <boost/scoped_array.hpp>
#include <boost/tuple/tuple.hpp>

#include "util.hpp"
#include "mds.hpp"
#include "bs.hpp"
#include "encoder/encoder.hpp"

class BSFile : public BS {
private:
	DISALLOW_EVIL_CONSTRUCTORS(BSFile);
	streaminfo info;
	boost::filesystem::basic_fstream<streamt> fs;
	boost::filesystem::path stream_path;
	boost::shared_ptr<MDS> mds;

	void init(streaminfo &info) {
		stream_path = boost::filesystem::path(info.loc);
	}

	void check_open() {
		if (!fs.is_open()) {
			fs.open(stream_path, std::ios_base::in | std::ios_base::out |
					std::ios_base::app | std::ios_base::binary);
		}
	}


public:
	BSFile(MDS mds, streamid id) {
		info = mds.get_info(id);
		init(info);
	}
	BSFile(streaminfo _info) {
		info = _info;
		init(info);
	}
	~BSFile() {};

	/**
	 * Add points to the stream.
	 * Assume that the incoming pts are encoded with the
	 * same encoder as the block store.
	 */
	bool add(valuet *pts, indext npts, indext ndecpts=SAME_DEC_PTS) {
		check_open();

		if (ndecpts == SAME_DEC_PTS) {
			ndecpts = npts;
		}

		filepost prevtail = get_tail_pos();

		if (info.encoder == NONE) {
			//direct write
			fs.write(reinterpret_cast<streamt*>(pts), SIZEMULT*npts);
		} else {
			if (info.encoder == DELTARLE) {
				//TODO: variable-width encode?
				fs.write(reinterpret_cast<streamt*>(pts), SIZEMULT*npts);
			} else if (info.encoder == ZLIB) {
				//TODO: use a chunk size that isn't the size of the flush?
				// (note that deflate() needs an intermediate buffer anyway,
				//	so this shouldn't be too horrible)
				streamt *charr = new streamt[(int) (SIZEMULT*npts*1.1)+12];
				filepost write = zlib_encode(pts, npts, charr);
				fs.write(charr, write);
				delete[] charr;
			}
		}

		//add an index point before the flush
		//  pt0     pt1      maxindex
		// /       /        /
		// |-------|--------
		//
		info.index.push_back(dxpair(info.maxindex, prevtail));

		fs.flush();
		info.maxindex += ndecpts;

		if (mds) {
			//update metadata on flush
			mds->update_maxindex(info.id, info.maxindex);
			mds->update_index(info.id, info.index);
		} else {
			INFO("No MDS to update in stream id=" << info.id)
		}

		return (!fs.fail());
	}

	filepost get_tail_pos() {
		return fs.tellp();
	}

	read_result read(dxrange req) {
		read_result rr;
		rr.blocks = std::vector<value_block>();
		if (info.encoder == NONE) {
			value_block vb;
			vb.data = boost::shared_array<streamt>(new streamt[req.len*SIZEMULT]);
			vb.encoder = NONE;
			vb.range = read_exact(reinterpret_cast<valuet*>(vb.data.get()), req);
			vb.data_len = vb.range.len;
			rr.blocks.push_back(vb);
		} else {
			//find the index point
			dxpair_list::iterator it = std::lower_bound(
					info.index.begin(), info.index.end(),
					dxpair(req.start, 0), compare_dxpair_list);
			print_vector(info.index);
			std::cout << "vector had size " << info.index.size() << std::endl;

			while (it != info.index.end()) {
				std::cout << "hit loop!" << std::endl;
				//don't go past the end of the stream
				if (tup_fst(*it) > req.start + req.len) {
					std::cout << "break loop: fst is " << tup_fst(*it) <<
							"start, len=" << req.start << req.len << std::endl;
					break;
				}

				//number of points in block
				indext pts_in_block = info.maxindex - tup_fst(*it);
				//size of data in block
				filepost blocksize = get_tail_pos() - tup_snd(*it);
				if (it + 1 != info.index.end()) {
					pts_in_block = tup_fst(*(it+1)) - tup_fst(*it);
					blocksize = tup_snd(*(it+1)) - tup_snd(*it);
				}

				//read in a block
				value_block vb;
				vb.data = boost::shared_array<streamt>(new streamt[blocksize]);
				fs.seekg(it->get<1>());
				fs.read(vb.data.get(), blocksize);
				vb.data_len = blocksize;
				vb.encoder = info.encoder;
				vb.range = dxrange(it->get<0>(), pts_in_block);
				rr.blocks.push_back(vb);

				++it;
			}
		}
		return rr;
	}

	/**
	 * @param vb value block to decode
	 * @param buf where to decode to
	 * @return number of decoded points
	 */
	indext decode_block(value_block &vb, valuet *buf) {
		indext ndec;
		switch (vb.encoder) {
		case DELTARLE:
			ndec = delta_rle_decode(
					vb.data.get(),
					vb.data_len,
					buf,
					vb.range.len);
			break;
		case ZLIB:
			ndec = zlib_decode(
					vb.data.get(),
					vb.data_len,
					buf,
					vb.range.len);
			break;
		default:
			ERROR("Unknown encoder " << vb.encoder);
			ndec = 0;
		}

		return ndec;
	}

	dxrange read_exact(valuet *pts, dxrange req) {
		if (info.encoder == NONE) {
			//direct seek if we have no encoder
			if (req.start > info.maxindex) {
				return dxrange(req.start, 0);
			}
			indext dxmin = std::max(req.start, info.minindex);
			indext npts = std::min(static_cast<indext>(req.len),
					(info.maxindex - info.minindex) - dxmin);

			check_open();
			fs.seekg(SIZEMULT*(dxmin - info.minindex));
			fs.read(reinterpret_cast<streamt*>(pts), SIZEMULT*npts);
			if (fs.fail()) {
				return dxrange(req.start, 0);
			} else {
				return dxrange(dxmin, npts);
			}
		} else {
			//need to decode a range
			read_result rr = read(req);
			indext bottom = req.start;

			//copy over relevant points
			valuet *pts_head = pts;
			indext mindx = std::numeric_limits<indext>::max()-1;

			for (std::vector<value_block>::iterator it = rr.blocks.begin();
					it != rr.blocks.end(); ++it) {
				mindx = std::min(mindx, it->range.start);

				// in index space:
				//
				//      a       b
				//  |---|       |---|
				//  |---------------|
				//  c               d
				//
				//  a - bottom
				//  b - req.start + req.len
				//  c - it->range.start
				//  d - it->range.start + it->range.len

				indext before_first = std::max(static_cast<indext>(0),
						bottom - it->range.start);
				indext after_last = std::max(static_cast<indext>(0),
						(it->range.start + it->range.len) - (req.start + req.len));
				indext declen = it->range.len - before_first - after_last;

				if (before_first > 0 || after_last > 0) {
					//need to decompress a partial block
					boost::scoped_array<valuet> scratch(
							new valuet[PAD_SIZE(it->range.len*SIZEMULT)]);
					indext ndec = decode_block(*it, scratch.get());
					BOOST_ASSERT(ndec == it->range.len);
					std::copy(scratch.get() + before_first,
							scratch.get() + before_first + declen,
							pts_head);
					pts_head += declen*SIZEMULT;
				} else {
					//decompress a whole block directly to the out ptr
					indext ndec = decode_block(*it, pts_head);
					BOOST_ASSERT(ndec == it->range.len);
					pts_head += ndec*SIZEMULT;
				}
			}

			//we didn't read anything.
			if (pts_head == pts) {
				mindx = req.start;
			}

			return dxrange(mindx, (pts_head-pts)/SIZEMULT);
		}
	}

	indext index(valuet pt) {
		if (info.sorted) {
			//binary search
			indext lo = info.minindex;
			indext hi = info.maxindex;
			indext mid = 0;
			valuet cur = 0;
			while (lo <= hi) {
				mid = (lo + hi) / 2;
				fs.seekg(SIZEMULT*(mid - info.minindex));
				fs.read(reinterpret_cast<streamt*>(&cur), SIZEMULT*1);
				if (cur > pt) {
					hi = mid - 1;
				} else if (cur < pt) {
					lo = mid + 1;
				} else {
					return mid;
				}
			}

			return -(mid + 1);
		} else {
			//unsorted, unindexed: we need to scan.
			const indext BUFSIZE = 1024;
			valuet buf[BUFSIZE];
			for (indext dx = info.minindex; dx < info.maxindex; dx += BUFSIZE) {
				int npts = std::min(BUFSIZE, info.maxindex - dx);
				read_exact(buf, dxrange(dx, npts));
				//scan the buffer
				for (int i = 0; i < npts; ++i) {
					if (buf[i] == pt) {
						return i + dx;
					}
				}
			}

			return NO_INDEX;
		}
	}

	indext get_maxindex() {
		return info.maxindex;
	}
};

#endif /* BS_FILE_HPP_ */
