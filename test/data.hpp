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
#include <boost/tokenizer.hpp>

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

inline bool pointt_timestamp_sort(pointt a, pointt b) {
	return a.t < b.t;
}

//http://stackoverflow.com/questions/1894886/parsing-a-comma-delimited-stdstring
struct ToValueT {
	int operator()(std::string const &str) {
		return static_cast<valuet>(atoi(str.c_str()));
	}
};

class CSVDataLoader {
private:
	DISALLOW_EVIL_CONSTRUCTORS(CSVDataLoader);

	boost::filesystem::path src;

	void init(boost::filesystem::path src) {
		this->src = src;
	}

public:
	CSVDataLoader(std::string csvloc) {
		init(csvloc);
	}

	std::vector<mpointt> to_rows() {
		std::vector<mpointt> rows;

		unsigned int MAX_LINE = 1000;
		char line[MAX_LINE];

		int max_cols = 0;

		boost::filesystem::ifstream cif(src);

		while (cif.good()) {
			cif.getline(line, MAX_LINE);
			if (!cif.good()) {
				break;
			}
			std::string strline(line);
			typedef boost::tokenizer< boost::char_separator<char> > Tokenizer;
			Tokenizer tok(strline);
			std::vector<valuet> ints;

			mpointt t;
			t.stream = 0;

			int dx = 0;
			//std::transform(tok.begin(), tok.end(), std::back_inserter(ints), ToValueT());
			for (Tokenizer::iterator it = tok.begin(); it != tok.end(); ++it) {
				valuet v = static_cast<valuet>(atoi((*it).c_str()));
				if (dx == 0) {
					t.t = v;
				} else {
					ints.push_back(v);
				}
			}

			t.v = ints;

			rows.push_back(t);
		}

		cif.close();

		return rows;
	}
};

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
			if (line.size() < 3) {
				continue;
			}
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

	valuet read_value(boost::filesystem::ifstream &ifs, int streamwidth) {
		valuet v;
		int8_t r1;
		int16_t r2;
		int32_t r4;
		int64_t r8;
		switch (streamwidth) {
		case 1:
			ifs.read(reinterpret_cast<streamt*>(&r1), 1);
			v = r1;
			break;
		case 2:
			ifs.read(reinterpret_cast<streamt*>(&r2), 2);
			v = r2;
			break;
		case 4:
			ifs.read(reinterpret_cast<streamt*>(&r4), 4);
			v = r4;
			break;
		case 8:
			ifs.read(reinterpret_cast<streamt*>(&r8), 8);
			v = r8;
			break;
		default:
			ERROR("unsupported stream width:" << streamwidth);
		}
		return v;
	}

	/**
	 * @param loc ts/vs loc of the data stream
	 * @param loc_id id to associate with the loc
	 * @param vwidth value width
	 */
	std::vector<pointt> read_stream_fully_rows(
			data_stream_loc loc,
			streamid loc_id,
			int twidth=4,
			int vwidth=4) {
		boost::filesystem::ifstream ints(loc.tsfile, std::ios_base::in | std::ios_base::binary);
		boost::filesystem::ifstream invs(loc.vsfile, std::ios_base::in | std::ios_base::binary);

		uintmax_t ints_size = boost::filesystem::file_size(loc.tsfile);
		uintmax_t invs_size = boost::filesystem::file_size(loc.vsfile);

		if (ints_size * twidth != invs_size * vwidth) {
			ERROR("timestamp file size " << loc.tsfile.c_str() <<
					" not the same as value file size " << loc.vsfile.c_str());
		}

		std::vector<pointt> points;
		points.reserve(ints_size / twidth + 1);
		while (!ints.eof()) {
			pointt pt;
			pt.stream = loc_id;
			pt.t = read_value(ints, twidth);
			pt.v = read_value(invs, vwidth);
			if (!ints.eof()) {
				points.push_back(pt);
			}
		}

		return points;
	}

	qres read_stream_fully_cols(
			data_stream_loc loc,
			streamid loc_id,
			int twidth=4,
			int vwidth=4) {
		boost::filesystem::ifstream ints(loc.tsfile, std::ios_base::in | std::ios_base::binary);
		boost::filesystem::ifstream invs(loc.vsfile, std::ios_base::in | std::ios_base::binary);

		uintmax_t ints_size = boost::filesystem::file_size(loc.tsfile);
		uintmax_t invs_size = boost::filesystem::file_size(loc.vsfile);

		if (ints_size * twidth != invs_size * vwidth) {
			ERROR("timestamp file size " << loc.tsfile.c_str() <<
					" not the same as value file size " << loc.vsfile.c_str());
		}

		qres qr;
		qr.npoints = ints_size / twidth;
		qr.ts = std::vector<valuet>();
		qr.vs = std::vector<valuet>();
		std::cerr << "resize to" << ints_size / twidth + 1 << std::endl;
		qr.ts.resize(ints_size / twidth + 1);
		qr.vs.resize(invs_size / vwidth + 1);
		indext dx = 0;
		while (!ints.eof()) {
			valuet val = read_value(ints, twidth);
			if (!ints.eof()) {
				qr.ts[dx] = val;
			}
			++dx;
		}
		dx = 0;
		while (!invs.eof()) {
			valuet val = read_value(invs, vwidth);
			if (!invs.eof()) {
				qr.vs[dx] = val;
			}
			++dx;
		}

		return qr;
	}

	/**
	 * Convert all data to time-ordered <stream, timestamp, value> format
	 */
	std::vector<pointt> to_rows() {
		std::vector<pointt> rows;
		streamid id = 0;
		for (std::vector<data_stream_loc>::iterator sit = streams_begin();
				sit != streams_end(); sit++) {
			std::vector<pointt> pts = read_stream_fully_rows(*sit, ++id);
			rows.insert(rows.end(), pts.begin(), pts.end());
		}

		std::sort(rows.begin(), rows.end(), pointt_timestamp_sort);

		return rows;
	}

	qres get_stream(streamid sid) {
		streamid id = 0;
		for (std::vector<data_stream_loc>::iterator sit = streams_begin();
				sit != streams_end(); sit++) {
			++id;
			if (id == sid) {
				return read_stream_fully_cols(*sit, sid);
			}
		}

		ERROR("No stream with id " << sid);
	}
};

#endif /* DATA_HPP_ */
