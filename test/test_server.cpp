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

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

BOOST_AUTO_TEST_SUITE( server_test_suite )

BOOST_AUTO_TEST_CASE( server_test ) {
	ColumnDataLoader loader("testdata/trivial");
	TSServer server;

	std::vector<pointt> rows = loader.to_rows();
	server.add_points(rows);

	//to find out what's in the metadata store
	/*
	boost::property_tree::json_parser::write_json(std::cerr,
			server.get_mds_ref()->to_ptree(),
			true);
	*/

	qres s1 = loader.get_stream(1);
	BOOST_TEST_MESSAGE(s1.ts.size());
	BOOST_TEST_MESSAGE(s1.vs.size());
}

BOOST_AUTO_TEST_SUITE_END()
