/*
 * test_server.cpp
 *
 *  Created on: Jun 5, 2013
 *      Author: mrcaps
 */

#include <boost/test/unit_test.hpp>
#include <cstdio>
#include <vector>

#include "../src/util.hpp"
#include "../src/server.hpp"
#include "data.hpp"

#include "../src/timer.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

BOOST_AUTO_TEST_SUITE( server_test_suite )

BOOST_AUTO_TEST_CASE( server_test ) {
	ColumnDataLoader loader("testdata/trivial");
	TSServer server(128);

	BSFile::remove_all_files();

	std::vector<pointt> rows = loader.to_rows();

	Timer timer;
	server.add_points(rows);
	BOOST_TEST_MESSAGE("insertion took " << timer.elapsed());

	//to find out what's in the metadata store
	/*
	boost::property_tree::json_parser::write_json(std::cerr,
			server.get_mds_ref()->to_ptree(),
			true);
	*/

	print_data(server.get_stream(3)->debug_get_contents(), 10);

	spairid sid = 1;
	qres truth = loader.get_stream(sid);

	//check entire stream equality
	BOOST_TEST_MESSAGE("npoints:" << truth.npoints);

	timer.restart();
	qres res = server.query(sid, truth.ts[0], truth.ts[truth.npoints-1] + 1);
	BOOST_TEST_MESSAGE("full query took " << timer.elapsed());

	BOOST_ASSERT(qres_equal(truth, res));
}

BOOST_AUTO_TEST_CASE( server_insert_speed ) {
	int bufsizes[] = {
		24, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768
	};

	ColumnDataLoader loader("testdata/trivial");

	std::vector<pointt> rows = loader.to_rows();

	BOOST_TEST_MESSAGE("size,itime");

	for (unsigned int bdx = 0; bdx < sizeof(bufsizes)/sizeof(int); ++bdx) {
		BSFile::remove_all_files();

		TSServer server(bufsizes[bdx]);

		Timer timer;
		server.add_points(rows);
		BOOST_TEST_MESSAGE(bufsizes[bdx] << "," << timer.elapsed());
	}
}

BOOST_AUTO_TEST_SUITE_END()
