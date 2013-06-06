/*
 * stream.hpp
 *
 *  Created on: Jun 5, 2013
 *      Author: ishafer
 */

#ifndef STREAM_HPP_
#define STREAM_HPP_

#include "util.hpp"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

class BSUncompressedFile {
private:
	uint64_t minindex;
	uint64_t maxindex;
	boost::filesystem::ofstream of;

public:
	bool flush(valuet* pts, int npts) {

	}
};

#endif /* STREAM_HPP_ */
