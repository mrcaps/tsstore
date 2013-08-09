/*
 * iface.hpp
 *
 *  Created on: Aug 8, 2013
 *      Author: mrcaps
 */

#ifndef IFACE_HPP_
#define IFACE_HPP_

#include <map>
#include <boost/shared_ptr.hpp>

#include "../src/util.hpp"

class TSServer {
private:
	MDS mds;
	std::map<streamid, boost::shared_ptr<ValueStream> > streams;

public:
	TSServer() {
		this->mds = MDS();
	}

	void add_points(std::vector<pointt> rows) {
		for (std::vector<pointt>::iterator pit = rows.begin(); pit != rows.end(); pit++) {

		}
	}

	ValueStream get_stream(streamid id) {
		if (streams.find(id) == streams.end()) {
			//instantiate stream
			streams[id] = new ValueStream(mds, id, 1024);
		}

		return streams.at(id);
	}
};

#endif /* IFACE_HPP_ */
