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
	int bufsize;
	boost::shared_ptr<MDS> mds;
	std::map<streamid, boost::shared_ptr<ValueStream> > streams;

public:
	/**
	 * @param _bufsize buffer size
	 */
	TSServer(int _bufsize) : bufsize(_bufsize), mds(new MDS()) {

	}

	boost::shared_ptr<MDS> get_mds_ref() {
		return mds;
	}

	void add_points(std::vector<pointt> rows) {
		for (std::vector<pointt>::iterator pit = rows.begin(); pit != rows.end(); pit++) {
			boost::shared_ptr<streampair> pair = mds->get_info_pair(pit->stream, 1);
			boost::shared_ptr<ValueStream> tsvs = get_stream(pair->ts->id);
			boost::shared_ptr<ValueStream> vsvs = get_stream(pair->vs[0]->id);

			tsvs->add_value(pit->t);
			vsvs->add_value(pit->v);
		}
	}

	void add_mpoints(std::vector<mpointt> rows) {
		for (std::vector<mpointt>::iterator pit = rows.begin(); pit != rows.end(); pit++) {

		}
	}

	qres query(spairid sid, valuet tstart, valuet tend) {
		boost::shared_ptr<streampair> pair = mds->get_info_pair(sid, 1);
		//find index range
		qres qr;
		qr.ts = std::vector<valuet>();
		qr.vs = std::vector<valuet>();

		boost::shared_ptr<ValueStream> tstream = get_stream(pair->ts->id);
		indext istart = std::abs(tstream->index(tstart));
		indext iend = std::abs(tstream->index(tend));
		//TODO: off-by-one details for query?
		indext npts = iend - istart;
		qr.npoints = npts;
		qr.ts.resize(npts);
		qr.vs.resize(npts);
		dxrange req;
		req.start = istart;
		req.len = npts;
		tstream->read(&(*qr.ts.begin()), req);

		boost::shared_ptr<ValueStream> vstream = get_stream(pair->vs[0]->id);
		vstream->read(&(*qr.vs.begin()), req);

		return qr;
	}

	boost::shared_ptr<ValueStream> get_stream(streamid id) {
		if (streams.find(id) == streams.end()) {
			//instantiate stream
			streams[id] = boost::shared_ptr<ValueStream>(new ValueStream(mds, id, bufsize));
		}

		return streams.at(id);
	}
};

#endif /* SERVER_HPP_ */
