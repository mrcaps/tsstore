/*
 * test_bs_uncompressed_file.cpp
 *
 *  Created on: Jun 6, 2013
 *      Author: ishafer
 */

#include <cstdio>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/random.hpp>

#include "../src/bs_file.hpp"
#include "../src/stream.hpp"
#include "data.hpp"

BOOST_AUTO_TEST_SUITE( bs_file_test_suite )

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
	streamid sid = 0;
	boost::random::mt19937 rndgen;
	boost::random::uniform_int_distribution<> dist100(1, 100);
	boost::random::uniform_int_distribution<> dist400(1, 400);

	for (unsigned int etdx = 0;
			etdx <= sizeof(encodert_values)/sizeof(encodert); ++etdx) {
		streaminfo info = {
			++sid,
			TMP_PATH().generic_string(),
			0,
			0,
			encodert_values[etdx],
			false
		};

		ValueStream vs(info, 1000);

		indext npts = 1600;
		//add some stuff
		boost::shared_array<valuet> pts =
				boost::shared_array<valuet>(get_test_data(npts, RANDOM));
		boost::shared_array<valuet> ptsret =
				boost::shared_array<valuet>(new valuet[npts]);

		indext ptsadd1 = dist400(rndgen);
		vs.add_values(pts.get(), ptsadd1);

		BOOST_CHECK_EQUAL(vs.read(ptsret.get(), dxrange(0, ptsadd1)).len, ptsadd1);
		BOOST_CHECK( memcmp(pts.get(), ptsret.get(), ptsadd1*SIZEMULT) == 0 );
	}

}

BOOST_AUTO_TEST_SUITE_END()
