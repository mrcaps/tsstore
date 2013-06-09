/*
 * test_bs_uncompressed_file.cpp
 *
 *  Created on: Jun 6, 2013
 *      Author: ishafer
 */

#include <cstdio>

#include <boost/test/unit_test.hpp>

#include "../src/bs_uncompressed_file.hpp"
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

	valuet *ptsret = new valuet[npts];
	BOOST_CHECK_EQUAL(npts, bs.read(ptsret, 0, npts));
	BOOST_CHECK( memcmp(pts, ptsret, npts*BSUncompressedFile::sizemult) == 0 );

	BOOST_CHECK_EQUAL(100, bs.read(ptsret, 200, 100));
	BOOST_CHECK( memcmp(pts+200, ptsret, 100*BSUncompressedFile::sizemult) == 0 );

	BOOST_CHECK_EQUAL(50, bs.read(ptsret, 950, 100));
}

BOOST_AUTO_TEST_SUITE_END()
