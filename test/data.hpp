/*
 * data.hpp
 *
 *  Created on: Jun 7, 2013
 *      Author: ishafer
 */

#ifndef DATA_HPP_
#define DATA_HPP_

#include <boost/shared_array.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <string>
#include <map>
#include <set>

#include "../src/util.hpp"

enum testdatatype {
	UNIFORM,
	RANDOM,
	SAWTOOTH,
	INCREASING_FIXED2, //increasing by 2
	MIXED
};
const testdatatype testdatatype_values[] = {
	UNIFORM,
	RANDOM,
	SAWTOOTH,
	INCREASING_FIXED2,
	MIXED
};

boost::shared_array<valuet> get_test_data(int npoints, testdatatype type);
void print_data(valuet *dta, int npoints);

typedef struct {
	boost::filesystem::path tsfile;
	boost::filesystem::path vsfile;
} data_stream_loc;

class ColumnDataLoader {
private:
	DISALLOW_EVIL_CONSTRUCTORS(ColumnDataLoader);

	boost::filesystem::path tsdir;
	boost::filesystem::path vsdir;

	void init(boost::filesystem::path tsdir,
			boost::filesystem::path vsdir,
			boost::filesystem::path mergepath) {
		this->tsdir = tsdir;
		this->vsdir = vsdir;

		boost::filesystem::ifstream mergein(mergepath, std::ios_base::in);

		std::string line;
		while (!mergein.eof()) {
			std::getline(mergein, line);
			std::string part1 = line.substr(0, line.find(" "));
			std::string part2 = line.substr(line.find(" ") + 1);
			data_stream_loc loc;
			loc.tsfile = tsdir / boost::filesystem::path(part1);
			loc.vsfile = vsdir / boost::filesystem::path(part2);
			locs.push_back(loc);
		}
	}

public:
	std::vector<data_stream_loc> locs;

	ColumnDataLoader(std::string base_dir) {
		boost::filesystem::path basepath(base_dir);
		init(basepath / boost::filesystem::path("ts"),
			basepath / boost::filesystem::path("vs"),
			basepath / boost::filesystem::path("mergemap.txt"));
	}

	std::vector<data_stream_loc>::iterator streams_begin() {
		return locs.begin();
	}

	std::vector<data_stream_loc>::iterator streams_end() {
		return locs.end();
	}
};

#endif /* DATA_HPP_ */
