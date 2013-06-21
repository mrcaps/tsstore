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
	boost::filesystem::basic_fstream<ofstreamt> fs;
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

		if (info.encoder == NONE) {
			//direct write
			fs.write(reinterpret_cast<ofstreamt*>(pts), SIZEMULT*npts);
		} else {
			if (info.encoder == DELTARLE) {
				//TODO: variable-width encode?
				fs.write(reinterpret_cast<char*>(pts), SIZEMULT*npts);
			}
		}

		info.maxindex += ndecpts;
		return (!(fs.fail()));
	}

	filepost get_tail_pos() {
		return fs.tellp();
	}

	indext read(valuet *pts, indext dxmin, indext npts) {
		//direct seek if we have no encoder
		if (info.encoder == NONE) {
			if (dxmin > info.maxindex) {
				return 0;
			}
			dxmin = std::max(dxmin, info.minindex);
			npts = std::min(static_cast<indext>(npts),
					(info.maxindex - info.minindex) - dxmin);

			check_open();
			fs.seekg(SIZEMULT*(dxmin - info.minindex));
			fs.read(reinterpret_cast<ofstreamt*>(pts), SIZEMULT);
			if (fs.fail()) {
				return 0;
			} else {
				return npts;
			}
		} else {
			//find the index point
			dxpair_list::iterator it = std::lower_bound(
					info.index.begin(), info.index.end(),
					dxpair(dxmin, 0), compare_dxpair_list);
			indext npts_read = 0; //number of total we read

			while (it != info.index.end()) {
				//number of points in block
				indext maxinblock = info.maxindex - it->first;
				//size of data in block
				filepost blocksize = get_tail_pos() - it->second;
				if (it + 1 != info.index.end()) {
					maxinblock = (it+1)->first - it->first;
					blocksize = (it+1)->second - it->second;
				}

				fs.seekg(it->second);
				//read up to (npts - npts_read) points

				//read
				boost::scoped_array<valuet> readarr(new valuet[blocksize/sizeof(valuet)]);
				ofstreamt* backing_ptr = reinterpret_cast<ofstreamt*>(readarr.get());
				fs.read(backing_ptr, blocksize);
				//decode the chunk
				indext ndec = 0;
				if (info.encoder == DELTARLE) {
					//TODO: other encoders
					ndec = delta_rle_decode(backing_ptr, blocksize, pts, maxinblock);
				}
				npts_read += ndec;
				pts += ndec;

				++it;
			}

			return npts_read;
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
				fs.read(reinterpret_cast<ofstreamt*>(&cur), SIZEMULT*1);
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
				read(buf, dx, npts);
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

	bool flush() {
		fs.flush();

		//add an index point
		info.index.push_back(dxpair(info.maxindex, get_tail_pos()));

		if (mds) {
			//update metadata on flush
			mds->update_maxindex(info.id, info.maxindex);
			mds->update_index(info.id, info.index);
		} else {
			INFO("No MDS to update in stream id=" << info.id)
		}

		//TODO: update end index
		return (!fs.fail());
	}
};

#endif /* BS_FILE_HPP_ */
