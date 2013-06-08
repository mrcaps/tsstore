/*
 * mds.hpp
 *
 *  Created on: Jun 6, 2013
 *      Author: ishafer
 */

#ifndef MDS_HPP_
#define MDS_HPP_

#include <string>

using namespace std;

typedef struct {
	streamid id; //unique id
	string loc; //stream location (e.g., path)
	uint64_t minindex; //min value index
	uint64_t maxindex; //max value index
} streaminfo;

class MDS {
public:
	streaminfo get_info(streamid id) {
		streaminfo info;
		info.id = id;
		info.loc = string("foobar");
		info.minindex = 0;
		info.maxindex = 1000;
		return info;
	}
};


#endif /* MDS_HPP_ */
