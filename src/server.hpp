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
#include <boost/weak_ptr.hpp>

#include "../src/util.hpp"
#include "mds.hpp"
#include "stream.hpp"

class TSServer {
private:
	boost::shared_ptr<MDS> mds;
	std::map<streamid, boost::shared_ptr<ValueStream> > streams;

public:
	TSServer() : mds(new MDS()) {
	}

	boost::shared_ptr<MDS> get_mds_ref() {
		return mds;
	}

	void add_points(std::vector<pointt> rows) {
		for (std::vector<pointt>::iterator pit = rows.begin(); pit != rows.end(); pit++) {
			boost::shared_ptr<streampair> pair = mds->get_info_pair(pit->stream);
			boost::shared_ptr<ValueStream> tsvs = get_stream(pair->ts->id);
			boost::shared_ptr<ValueStream> vsvs = get_stream(pair->vs->id);

			tsvs->add_value(pit->t);
			vsvs->add_value(pit->v);
		}
	}

	qres query(spairid sid, valuet tstart, valuet tend) {
		boost::shared_ptr<streampair> pair = mds->get_info_pair(sid);
		//find index range
		qres qr;
		qr.ts = std::vector<valuet>();
		qr.vs = std::vector<valuet>();

		boost::shared_ptr<ValueStream> tstream = get_stream(pair->ts->id);
		indext istart = tstream->index(tstart);
		indext iend = tstream->index(tend);
		indext npts = iend - istart;
		qr.npoints = npts;
		qr.ts.reserve(npts);
		qr.vs.reserve(npts);
		dxrange req;
		req.start = istart;
		req.len = npts;
		tstream->read(&(*qr.ts.begin()), req);

		boost::shared_ptr<ValueStream> vstream = get_stream(pair->vs->id);
		vstream->read(&(*qr.vs.begin()), req);

		return qr;
	}

	boost::shared_ptr<ValueStream> get_stream(streamid id) {
		if (streams.find(id) == streams.end()) {
			//instantiate stream
			streams[id] = boost::shared_ptr<ValueStream>(new ValueStream(mds, id, 1024));
		}

		return streams.at(id);
	}
};

#endif /* SERVER_HPP_ */
