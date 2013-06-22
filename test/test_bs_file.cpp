/*
 * test_bs_uncompressed_file.cpp
 *
 *  Created on: Jun 6, 2013
 *      Author: ishafer
 */

#include <cstdio>

#include <boost/test/unit_test.hpp>

#include "../src/bs_file.hpp"
#include "data.hpp"

BOOST_AUTO_TEST_SUITE( file_test_suite )

BOOST_AUTO_TEST_CASE( basic_test ) {
	streaminfo info;
	info.id = 1;
	info.loc = TMP_PATH().generic_string();
	info.minindex = 0;
	info.maxindex = 0;
	info.encoder = NONE;
	info.sorted = false;
	BSFile bs(info);

	int npts = 1000;
	boost::shared_array<valuet> pts = boost::shared_array<valuet>(get_test_data(npts, RANDOM));
	bs.add(pts.get(), npts);

	boost::shared_array<valuet> ptsret = boost::shared_array<valuet>(new valuet[npts]);
	BOOST_CHECK_EQUAL(npts, bs.read_exact(ptsret.get(), dxrange(0, npts)).len);
	BOOST_CHECK( memcmp(pts.get(), ptsret.get(),
			npts*SIZEMULT) == 0 );

	BOOST_CHECK_EQUAL(100, bs.read_exact(ptsret.get(), dxrange(200, 100)).len);
	BOOST_CHECK( memcmp(pts.get()+200, ptsret.get(),
			100*SIZEMULT) == 0 );

	BOOST_CHECK_EQUAL(50, bs.read_exact(ptsret.get(), dxrange(950, 100)).len);
}

BOOST_AUTO_TEST_CASE( encoders_test ) {
	for (unsigned int etdx = 0;
			etdx <= sizeof(encodert_values)/sizeof(encodert); ++etdx) {

	}
	streaminfo info;
	info.id = 1;
	info.loc = TMP_PATH().generic_string();
	info.minindex = 0;
	info.maxindex = 0;
	info.encoder = NONE;
	info.sorted = false;
	BSFile bs(info);

	int npts = 1000;
	boost::shared_array<valuet> pts = boost::shared_array<valuet>(get_test_data(npts, RANDOM));
	bs.add(pts.get(), npts);

	boost::shared_array<valuet> ptsret = boost::shared_array<valuet>(new valuet[npts]);
	BOOST_CHECK_EQUAL(npts, bs.read_exact(ptsret.get(), dxrange(0, npts)).len);
	BOOST_CHECK( memcmp(pts.get(), ptsret.get(),
			npts*SIZEMULT) == 0 );

	BOOST_CHECK_EQUAL(100, bs.read_exact(ptsret.get(), dxrange(200, 100)).len);
	BOOST_CHECK( memcmp(pts.get()+200, ptsret.get(),
			100*SIZEMULT) == 0 );

	BOOST_CHECK_EQUAL(50, bs.read_exact(ptsret.get(), dxrange(950, 100)).len);
}

BOOST_AUTO_TEST_SUITE_END()
