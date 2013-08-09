/*
 * mds.hpp
 *
 *  Created on: Jun 6, 2013
 *      Author: ishafer
 */

#ifndef MDS_HPP_
#define MDS_HPP_

#include <string>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

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

struct streaminfo {
	streamid id; //unique id
	std::string loc; //stream location (e.g., path)
	indext minindex; //min value index
	indext maxindex; //max value index
	encodert encoder; //stream encoder
	bool sorted;
	dxpair_list index;

	streaminfo(streamid _id, std::string _loc, indext _minindex, indext _maxindex,
			encodert _encoder, bool _sorted, dxpair_list _index) :
			id(_id), loc(_loc), minindex(_minindex), maxindex(_maxindex),
			encoder(_encoder), sorted(_sorted), index(_index) {

	}

	boost::property_tree::ptree to_ptree() {
		boost::property_tree::ptree pt;
		pt.put("id", id);
		pt.put("loc", loc);
		pt.put("minindex", minindex);
		pt.put("maxindex", maxindex);
		pt.put("encoder", encoder);
		pt.put("sorted", sorted);
		pt.put("index.size", index.size());
		return pt;
	}
};

struct streampair {
	spairid id;
	boost::shared_ptr<streaminfo> ts;
	boost::shared_ptr<streaminfo> vs;

	streampair(spairid _id, boost::shared_ptr<streaminfo> _ts, boost::shared_ptr<streaminfo> _vs) :
		id(_id), ts(_ts), vs(_vs) {

	}

	boost::property_tree::ptree to_ptree() {
		boost::property_tree::ptree pt;
		pt.put("id", id);
		pt.put_child("ts", ts->to_ptree());
		pt.put_child("vs", vs->to_ptree());
		return pt;
	}
};

class MDS {
public:
	streamid lastid;
	std::map<spairid, boost::shared_ptr<streampair> > streampairs;
	std::map<streamid, boost::shared_ptr<streaminfo> > streaminfos;

	MDS() : lastid(0), streampairs() {

	}

	boost::property_tree::ptree to_ptree() {
		boost::property_tree::ptree pt;
		pt.put("lastid", lastid);
		for (std::map<spairid, boost::shared_ptr<streampair> >::iterator it = streampairs.begin();
				it != streampairs.end();
				it++) {
			pt.put_child((boost::format("streampair-%d") % it->first).str(), it->second->to_ptree());
		}

		return pt;
	}

	boost::shared_ptr<streampair> get_info_pair(spairid id) {
		boost::filesystem::path basepath("filestore");

		if (streampairs.find(id) == streampairs.end()) {
			//make new stream pair
			streamid tsid = lastid++;
			boost::shared_ptr<streaminfo> tsinfo(new streaminfo(
				lastid,
				(basepath / boost::filesystem::path((boost::format("%1%.stream") % lastid).str())).string(),
				0,
				0,
				DELTARLE,
				true,
				std::vector<dxpair>()
			));
			streaminfos[tsid] = boost::shared_ptr<streaminfo>(tsinfo);

			streamid vsid = lastid++;
			boost::shared_ptr<streaminfo> vsinfo(new streaminfo(
				lastid,
				(basepath / boost::filesystem::path((boost::format("%1%.stream") % lastid).str())).string(),
				0,
				0,
				NONE,
				false,
				std::vector<dxpair>()
			));
			streaminfos[vsid] = boost::shared_ptr<streaminfo>(vsinfo);

			streampairs[id] = boost::shared_ptr<streampair>(new streampair(
				id,
				tsinfo,
				vsinfo
			));
		}

		return streampairs.at(id);
	}

	streaminfo get_info(streamid id) {
		streaminfo info(
				id,
				std::string("FIXME"),
				0,
				1000,
				NONE,
				false,
				std::vector<dxpair>()
				);
		return info;
	}

	void print_state() {

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
