/*
 * mds.hpp
 *
 *  Created on: Jun 6, 2013
 *      Author: ishafer
 */

#ifndef MDS_HPP_
#define MDS_HPP_

#include <string>

#include "encoder/encoder.hpp"

//might as well break this out instead of using a tuple.
struct dxpair {
	indext dx; //index in the stream
	filepost fpos; //file position
	valuet val; //value at the location

	friend std::ostream& operator<<(std::ostream &os, dxpair &p) {
		return os <<
				"dxpair{dx=" << p.dx <<
				" fpos=" << p.fpos <<
				" val=" << p.val << "}";
	}

	dxpair(indext _dx, filepost _fpos, valuet _val) : dx(_dx), fpos(_fpos), val(_val) {}
};
typedef std::vector<dxpair> dxpair_list;
inline bool compare_dxpair_list_dx(dxpair fst, dxpair snd) {
	return fst.dx < snd.dx;
}
inline bool compare_dxpair_list_val(dxpair fst, dxpair snd) {
	return fst.val < snd.val;
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
		DEBUG("updating maxindex id=" << id << " maxindex=" << maxindex);
		return true;
	}

	bool update_index(streamid id, dxpair_list lst) {
		DEBUG("updating index id=" << id);
		return true;
	}

};


#endif /* MDS_HPP_ */
