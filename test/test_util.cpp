/*
 * test_util.cpp
 *
 *  Created on: Jun 5, 2013
 *      Author: mrcaps
 */

#include <boost/test/unit_test.hpp>
#include <cstdio>

#include "../src/util.hpp"

using namespace std;

BOOST_AUTO_TEST_SUITE( util_test_suite )

BOOST_AUTO_TEST_CASE( types_test ) {
	BOOST_CHECK_EQUAL(sizeof(valuet), (size_t) 8);
}

BOOST_AUTO_TEST_CASE( max_open_files_test ) {
	int i = 0;
	for (i = 0; i < 10000; ++i) {
		FILE *f = fopen("/dev/null", "r");
		if (!f) {
			break;
		}
	}
	BOOST_TEST_MESSAGE( "Max # of open files: " << i );
}

BOOST_AUTO_TEST_SUITE_END()
