/*
 * test_stream.cpp
 *
 *  Created on: Jun 8, 2013
 *      Author: ishafer
 */

#include <cstdio>
#include <cstring>

#include <boost/test/unit_test.hpp>
#include <boost/shared_array.hpp>

#include "../src/bs.hpp"
#include "../src/bs_file.hpp"
#include "../src/stream.hpp"
#include "data.hpp"

streaminfo get_temp_bs(bool sorted, encodert encoder) {
	static streamid id = 1;
	streaminfo info;
	info.id = ++id;
	info.loc = TMP_PATH().generic_string();
	info.minindex = 0;
	info.maxindex = 0;
	info.index = std::vector<dxpair>();
	info.sorted = sorted;
	info.encoder = encoder;
	return info;
}

BOOST_AUTO_TEST_SUITE( stream_test_suite )

BOOST_AUTO_TEST_CASE( basic_test ) {
	streaminfo info = get_temp_bs(false, NONE);

	ValueStream vs(info, 100);

	int npts = 1000;
	boost::shared_array<valuet> pts = get_test_data(npts, RANDOM);
	vs.add_values(pts.get(), 50);

	boost::shared_array<valuet> ptsret = boost::shared_array<valuet>(new valuet[npts]);
	BOOST_CHECK_EQUAL(50, vs.read(ptsret.get(), dxrange(0, 50)).len);
	BOOST_CHECK( memcmp(pts.get(), ptsret.get(),
			50*SIZEMULT) == 0 );

	vs.flush();

	BOOST_CHECK_EQUAL(50, vs.read(ptsret.get(), dxrange(0, 50)).len);
	BOOST_CHECK( memcmp(pts.get(), ptsret.get(),
			50*SIZEMULT) == 0 );

	vs.add_values(pts.get()+50, 100);

	BOOST_CHECK_EQUAL(150, vs.read(ptsret.get(), dxrange(0, 150)).len);
	BOOST_CHECK( memcmp(pts.get(), ptsret.get(),
			150*SIZEMULT) == 0 );
}

BOOST_AUTO_TEST_CASE( index_test ) {
	streaminfo info = get_temp_bs(true, NONE);

	ValueStream vs(info, 100);

	boost::shared_array<valuet> pts = get_test_data(50, INCREASING_FIXED2);
	vs.add_values(pts.get(), 50);

	BOOST_CHECK_EQUAL(25, vs.index(50));
	BOOST_CHECK_EQUAL(-27, vs.index(51));
	BOOST_CHECK_EQUAL(-51, vs.index(1000));
	BOOST_CHECK_EQUAL(-1, vs.index(-100));

	vs.flush();

	BOOST_CHECK_EQUAL(25, vs.index(50));
	BOOST_CHECK_EQUAL(-27, vs.index(51));
	BOOST_CHECK_EQUAL(-51, vs.index(1000));
	BOOST_CHECK_EQUAL(-1, vs.index(-100));
}

BOOST_AUTO_TEST_CASE( delta_rle_test ) {
	streaminfo info = get_temp_bs(true, DELTARLE);

	ValueStream vs(info, 100);

	boost::shared_array<valuet> pts = get_test_data(10, INCREASING_FIXED2);
	vs.add_values(pts.get(), 10);

	valuet* cont = vs.debug_get_contents();
	valuet arrtest[] = {0, 2, 2, 7};
	BOOST_CHECK( memcmp(cont, arrtest, 4*SIZEMULT) == 0 );

	BOOST_CHECK_EQUAL(-1, vs.index(-4));
	BOOST_CHECK_EQUAL(3, vs.index(6));
	BOOST_CHECK_EQUAL(-4, vs.index(7));
	BOOST_CHECK_EQUAL(-10, vs.index(30));

	boost::shared_array<valuet> ptsret = boost::shared_array<valuet>(new valuet[100]);

	vs.read(ptsret.get(), dxrange(0, 10));
	BOOST_CHECK( memcmp(pts.get(), ptsret.get(), 10*SIZEMULT) == 0);

	//stash some different data in pts
	for (int i = 0; i < 10; ++i) {
		pts.get()[i] = i * 3 + 20;
	}
	vs.add_values(pts.get(), 10);

	cont = vs.debug_get_contents();
	valuet arrtest2[] = {0, 2, 2, 8, 3, 3, 7};
	BOOST_CHECK( memcmp(cont, arrtest2, 7*SIZEMULT) == 0 );

	vs.read(ptsret.get(), dxrange(0, 20));
	BOOST_CHECK( memcmp(pts.get(), ptsret.get() + 10, 10*SIZEMULT) == 0);
}

BOOST_AUTO_TEST_SUITE_END()
