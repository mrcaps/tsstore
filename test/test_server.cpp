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

	boost::property_tree::json_parser::write_json(std::cerr,
			server.get_mds_ref()->to_ptree(),
			true);
	std::cerr << "SERVER TEST!!!" << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()
