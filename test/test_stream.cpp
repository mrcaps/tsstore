/*
 * test_stream.cpp
 *
 *  Created on: Jun 8, 2013
 *      Author: ishafer
 */

#include <cstdio>

#include <boost/test/unit_test.hpp>

#include "../src/bs.hpp"
#include "../src/bs_uncompressed_file.hpp"
#include "../src/stream.hpp"
#include "data.hpp"

using namespace std;

BOOST_AUTO_TEST_SUITE( stream_test_suite )

BOOST_AUTO_TEST_CASE( basic_test ) {
	streaminfo info;
	info.id = 1;
	info.loc = TMP_PATH().generic_string();
	info.minindex = 0;
	info.maxindex = 0;

	ValueStream vs(new BSUncompressedFile(info), 100);

	int npts = 1000;
	valuet *pts = get_test_data(npts, RANDOM);
	vs.add_values(pts, 50);

	valuet *ptsret = new valuet[npts];
	BOOST_CHECK_EQUAL(50, vs.read(ptsret, 0, 50));
	BOOST_CHECK( memcmp(pts, ptsret, 50*BSUncompressedFile::sizemult) == 0 );

	vs.flush();

	BOOST_CHECK_EQUAL(50, vs.read(ptsret, 0, 50));
	BOOST_CHECK( memcmp(pts, ptsret, 50*BSUncompressedFile::sizemult) == 0 );

	vs.add_values(pts+50, 100);

	BOOST_CHECK_EQUAL(150, vs.read(ptsret, 0, 150));
	BOOST_CHECK( memcmp(pts, ptsret, 150*BSUncompressedFile::sizemult) == 0 );
}

BOOST_AUTO_TEST_SUITE_END()
