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

#include "util.hpp"
#include "mds.hpp"
#include "bs.hpp"
#include "encoder/encoder.hpp"

class BSFile : public BS {
private:
	DISALLOW_EVIL_CONSTRUCTORS(BSFile);
	streaminfo info;
	boost::filesystem::basic_fstream<streamt> fs;
	//seek pointers for put and get on basic_fstream are not the same.
	filepost fs_ploc;
	filepost fs_gloc;

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
		fs_ploc = fs.tellp();
		fs_gloc = fs.tellg();
	}

	//
	// workarounds for non-separate put/get pointers
	//

	void fs_seekg(filepost loc) {
		fs.seekg(loc);
		fs_gloc = loc;
	}

	filepost fs_tellg() {
		return fs_gloc;
	}

	void fs_seekp(filepost loc) {
		fs.seekp(loc);
		fs_ploc = loc;
	}

	filepost fs_tellp() {
		return fs_ploc;
	}

	void fs_write(streamt *pts, filepost size) {
		fs.write(pts, size);
		if (!fs.fail()) {
			fs_ploc += size;
		}
	}

	void fs_read(streamt *pts, filepost size) {
		fs.read(pts, size);
		if (!fs.fail()) {
			fs_gloc += size;
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
			fs_write(reinterpret_cast<streamt*>(pts), SIZEMULT*npts);
		} else {
			if (info.encoder == DELTARLE) {
				//TODO: variable-width encode?
				fs_write(reinterpret_cast<streamt*>(pts), SIZEMULT*npts);
			} else if (info.encoder == ZLIB) {
				//TODO: use a chunk size that isn't the size of the flush?
				// (note that deflate() needs an intermediate buffer anyway,
				//	so this shouldn't be too horrible)
				streamt *charr = new streamt[(int) (SIZEMULT*npts*1.1)+12];
				filepost write = zlib_encode(pts, npts, charr);
				fs_write(charr, write);
				delete[] charr;
			}
		}

		//add an index point before the flush
		//  pt0     pt1      maxindex
		// /       /        /
		// |-------|--------
		//
		info.index.push_back(dxpair(info.maxindex, prevtail, pts[0]));

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
		return fs_tellp();
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
			//upper_bound: first element which compares greater than val
			//	(furthermost into which search key can be inserted w/o violating ordering)
			dxpair_list::iterator it = std::upper_bound(
					info.index.begin(), info.index.end(),
					dxpair(req.start, 0, 0), compare_dxpair_list_dx);
			//might need to step one block before upper_bound
			if (info.index.size() > 0) {
				it -= 1;
			}

			while (it != info.index.end()) {
				std::pair<indext, filepost> dim = get_block_dim(it);
				if (!is_block_in_range(it->dx, dim.first, req.start, req.len)) {
					break;
				}

				//read in a block
				value_block vb;
				vb.data = boost::shared_array<streamt>(new streamt[dim.second]);
				fs_seekg(it->fpos);
				fs_read(vb.data.get(), dim.second);
				vb.data_len = dim.second;
				vb.encoder = info.encoder;
				vb.range = dxrange(it->dx, dim.first);
				rr.blocks.push_back(vb);

				++it;
			}
		}

		return rr;
	}

	/**
	 * For the block with index pointer p at the beginning, find:
	 * first: the number of points in the block
	 * second: the size of the block on disk
	 */
	std::pair<indext, filepost> get_block_dim(dxpair_list::iterator p) {
		//number of points in block
		indext pts_in_block = info.maxindex - p->dx;
		//size of data in block
		filepost blocksize = get_tail_pos() - p->fpos;
		if (p + 1 != info.index.end()) {
			pts_in_block = (p+1)->dx - p->dx;
			blocksize = (p+1)->fpos - p->fpos;
		}

		return std::pair<indext, filepost>(pts_in_block, blocksize);
	}

	bool is_block_in_range(
			indext block_start,
			indext block_len,
			indext req_start,
			indext req_len) {
		//don't go past the end of the stream
		if (block_start > req_start + req_len) {
			DEBUG("  BSFile::read past end: " <<
					"fst is " << tup_fst(*it) <<
					"start, len=" << req.start << req.len);
			return false;
		}

		//don't go before the beginning of the stream
		if (block_start + block_len < req_start) {
			DEBUG("  BSFile::read before begin: " <<
					"fst=" << tup_fst(*it) << " snd=" << tup_snd(*it) <<
					"start=" << req.start << " len=" << req.len);
			return false;
		}

		return true;
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
			fs_seekg(SIZEMULT*(dxmin - info.minindex));
			fs_read(reinterpret_cast<streamt*>(pts), SIZEMULT*npts);
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
		if (info.encoder == NONE) {
			if (info.sorted) {
				//binary search
				indext lo = info.minindex;
				indext hi = info.maxindex;
				indext mid = 0;
				valuet cur = 0;
				while (lo <= hi) {
					mid = (lo + hi) / 2;
					fs_seekg(SIZEMULT*(mid - info.minindex));
					fs_read(reinterpret_cast<streamt*>(&cur), SIZEMULT*1);
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
		} else {
			//TODO: index for encoded blocks
			if (info.sorted) {
				//find the index point
				//upper_bound: first element which compares greater than val
				//	(furthermost into which search key can be inserted w/o violating ordering)
				dxpair_list::iterator it = std::upper_bound(
						info.index.begin(), info.index.end(),
						dxpair(0, 0, pt), compare_dxpair_list_val);
				//might need to step one block before upper_bound
				if (info.index.size() > 0) {
					it -= 1;
				}

				while (it != info.index.end()) {
					std::pair<indext, filepost> dim = get_block_dim(it);

					//for now, do nothing cleverer than just decode-and-scan
					//TODO: optimize for streaming decoders (deltarle)

					//read in a block
					value_block vb;
					vb.data = boost::shared_array<streamt>(new streamt[dim.second]);
					fs_seekg(it->fpos);
					fs_read(vb.data.get(), dim.second);
					vb.data_len = dim.second;
					vb.encoder = info.encoder;
					vb.range = dxrange(it->dx, dim.first);

					boost::scoped_array<valuet> decvals(new valuet[dim.first]);
					//indext ndec = decode_block(vb, decvals.get());
					valuet* lb = std::lower_bound(decvals.get(), decvals.get() + dim.first, pt);

					//we're within the block
					if (lb < decvals.get() + dim.first) {
						return (lb - decvals.get());
					}

					it++;
				}

				return NO_INDEX;
			} else {
				//TODO: index unsorted blocks
			}
		}
	}

	indext get_maxindex() {
		return info.maxindex;
	}
};

#endif /* BS_FILE_HPP_ */
