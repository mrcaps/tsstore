/*
 * test_data.cpp
 *
 *  Created on: Aug 1, 2013
 *      Author: mrcaps
 */

#include <boost/test/unit_test.hpp>
#include <cstdio>

#include "data.hpp"

BOOST_AUTO_TEST_SUITE( data_test_suite )

BOOST_AUTO_TEST_CASE( data_test ) {
	ColumnDataLoader loader("testdata/trivial");

	int nfiles = 0;
	for(std::vector<data_stream_loc>::iterator it = loader.streams_begin();
			it != loader.streams_end(); it++) {
		++nfiles;
	}

	BOOST_CHECK_EQUAL(2, nfiles);
}

BOOST_AUTO_TEST_CASE( to_rows_test ) {
	ColumnDataLoader loader("testdata/trivial");

	std::vector<pointt> rows = loader.to_rows();

	int npoints = 0;
	for (std::vector<pointt>::iterator rs = rows.begin(); rs != rows.end(); rs++) {
		if (npoints < 100) {
			BOOST_TEST_MESSAGE("point " << rs->stream << " " << rs->t << " " << rs->v);
		}

		++npoints;
	}
}

BOOST_AUTO_TEST_SUITE_END()
