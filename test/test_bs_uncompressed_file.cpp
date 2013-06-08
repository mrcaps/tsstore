/*
 * test_bs_uncompressed_file.cpp
 *
 *  Created on: Jun 6, 2013
 *      Author: ishafer
 */

#include "../src/bs_uncompressed_file.hpp"

#include <boost/test/unit_test.hpp>
#include <cstdio>

#include "data.hpp"

using namespace std;

BOOST_AUTO_TEST_SUITE( uncompressed_file_test_suite )

BOOST_AUTO_TEST_CASE( basic_test ) {
	streaminfo info;
	info.id = 1;
	info.loc = TMP_PATH().generic_string();
	info.minindex = 0;
	info.maxindex = 0;
	BSUncompressedFile bs(info);

	int npts = 1000;
	valuet *pts = get_test_data(npts, RANDOM);
	bs.add(pts, npts);
	bs.flush();

	BOOST_TEST_MESSAGE( "done!" );
}

BOOST_AUTO_TEST_SUITE_END()
