/*
 * mds.hpp
 *
 *  Created on: Jun 6, 2013
 *      Author: ishafer
 */

#ifndef MDS_HPP_
#define MDS_HPP_

#include <string>
#include <boost/tuple/tuple.hpp>

#include "encoder/encoder.hpp"

//index
typedef boost::tuple<indext, filepost> dxpair;
typedef std::vector<dxpair> dxpair_list;
inline bool compare_dxpair_list(dxpair fst, dxpair snd) {
	return fst.get<0>() < snd.get<0>();
}
typedef struct {
	streamid id; //unique id
	std::string loc; //stream location (e.g., path)
	indext minindex; //min value index
	indext maxindex; //max value index
	encodert encoder; //stream encoder
	bool sorted;
	dxpair_list index;
} streaminfo;

class MDS {
public:
	streaminfo get_info(streamid id) {
		streaminfo info;
		info.id = id;
		info.loc = std::string("foobar");
		info.minindex = 0;
		info.maxindex = 1000;
		info.sorted = false;
		info.index = std::vector<dxpair>();
		return info;
	}

	bool update_maxindex(streamid id, indext maxindex) {
		std::cout << "updating maxindex id=" << id << " maxindex=" << maxindex << std::endl;
		return true;
	}

	bool update_index(streamid id, dxpair_list lst) {
		std::cout << "updating index id=" << id << std::endl;
		return true;
	}

};


#endif /* MDS_HPP_ */
