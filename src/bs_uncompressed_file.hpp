/*
 * stream.hpp
 *
 *  Created on: Jun 5, 2013
 *      Author: ishafer
 */

#ifndef STREAM_HPP_
#define STREAM_HPP_

#include "util.hpp"
#include "mds.hpp"
#include "bs.hpp"

#include <string>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

typedef char ofstreamt;

using namespace std;

class BSUncompressedFile : BS {
private:
	streaminfo info;
	boost::filesystem::basic_ofstream<ofstreamt> of;
	boost::filesystem::path stream_path;
	void init(streaminfo &info) {
		stream_path = boost::filesystem::path(info.loc);
	}

public:
	BSUncompressedFile(MDS mds, streamid id) {
		info = mds.get_info(id);
		init(info);
	}
	BSUncompressedFile(streaminfo info) {
		init(info);
	}
	~BSUncompressedFile() {};

	bool add(valuet* pts, int npts) {
		if (!of.is_open()) {
			of.open(stream_path,
					std::ios_base::out | std::ios_base::app | std::ios_base::binary);
		}
		of.write(reinterpret_cast<char*>(pts), sizeof(valuet)/sizeof(ofstreamt)*npts);

		return (!(of.failbit || of.badbit));
	}

	bool flush() {
		of.flush();
		//TODO: update end index
		return (!(of.failbit || of.badbit));
	}
};

#endif /* STREAM_HPP_ */
