/*
 * stream.hpp
 *
 *  Created on: Jun 5, 2013
 *      Author: ishafer
 */

#ifndef BS_UNCOMPRESSED_FILE_HPP_
#define BS_UNCOMPRESSED_FILE_HPP_

#include <string>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "util.hpp"
#include "mds.hpp"
#include "bs.hpp"

typedef char ofstreamt;

using namespace std;

class BSUncompressedFile : public BS {
private:
	DISALLOW_EVIL_CONSTRUCTORS(BSUncompressedFile);
	streaminfo info;
	boost::filesystem::basic_fstream<ofstreamt> fs;
	boost::filesystem::path stream_path;
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
	static const int sizemult = (sizeof(valuet)/sizeof(ofstreamt));

	BSUncompressedFile(MDS mds, streamid id) {
		info = mds.get_info(id);
		init(info);
	}
	BSUncompressedFile(streaminfo _info) {
		info = _info;
		init(info);
	}
	~BSUncompressedFile() {};

	bool add(valuet *pts, int64_t npts) {
		check_open();
		fs.write(reinterpret_cast<char*>(pts), sizemult*npts);
		info.maxindex += npts;
		return (!(fs.fail()));
	}

	int64_t read(valuet *pts, int64_t dxmin, int64_t npts) {
		if (dxmin > info.maxindex) {
			return 0;
		}
		dxmin = max(dxmin, info.minindex);
		npts = min(static_cast<int64_t>(npts), (info.maxindex - info.minindex) - dxmin);

		check_open();
		fs.seekg(sizemult*dxmin);
		fs.read(reinterpret_cast<char*>(pts), sizemult*npts);
		if (fs.fail()) {
			return 0;
		} else {
			return npts;
		}
	}

	int64_t get_maxindex() {
		return info.maxindex;
	}

	bool flush() {
		fs.flush();
		//TODO: update end index
		return (!fs.fail());
	}
};

#endif /* BS_UNCOMPRESSED_FILE_HPP_ */
