/*
 * server.hpp
 *
 *  Created on: Aug 8, 2013
 *      Author: mrcaps
 */

#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <map>
#include <boost/shared_ptr.hpp>

#include "../src/util.hpp"
#include "mds.hpp"
#include "stream.hpp"

class TSServer {
private:
	MDS mds;
	std::map<streamid, boost::shared_ptr<ValueStream> > streams;

public:
	TSServer() {
		mds = MDS();
	}

	MDS get_mds() {
		return mds;
	}

	void add_points(std::vector<pointt> rows) {
		for (std::vector<pointt>::iterator pit = rows.begin(); pit != rows.end(); pit++) {
			boost::shared_ptr<streampair> pair = mds.get_info_pair(pit->stream);
			boost::shared_ptr<ValueStream> tsvs = get_stream(pair->ts->id);
			boost::shared_ptr<ValueStream> vsvs = get_stream(pair->vs->id);

			tsvs->add_value(pit->t);
			vsvs->add_value(pit->v);
		}
	}

	boost::shared_ptr<ValueStream> get_stream(streamid id) {
		if (streams.find(id) == streams.end()) {
			//instantiate stream
			streams[id] = boost::shared_ptr<ValueStream>(new ValueStream(boost::shared_ptr<MDS>(&mds), id, 1024));
		}

		return streams.at(id);
	}
};

#endif /* SERVER_HPP_ */
