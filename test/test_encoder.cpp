/*
 * test_encoder.cpp
 *
 *  Created on: Jun 21, 2013
 *      Author: mrcaps
 */

#include <cstdio>
#include <cstring>

#include <boost/test/unit_test.hpp>
#include <boost/shared_array.hpp>
#include <boost/scoped_array.hpp>

#include "../src/encoder/encoder.hpp"
#include "../src/bs_file.hpp"
#include "data.hpp"

BOOST_AUTO_TEST_SUITE( encoder_test_suite )

static const encdec_pair fns[] = {
	encdec_pair(delta_rle_encode, delta_rle_decode)
};

BOOST_AUTO_TEST_CASE( roundtrip_test ) {
	indext ptslen = 100;
	boost::scoped_array<ofstreamt> buf(new ofstreamt[ptslen*BSFile::sizemult]);
	boost::scoped_array<valuet> ptsret(new valuet[ptslen]);

	for (unsigned int fndx = 0; fndx < sizeof(fns)/sizeof(encdec_pair); ++fndx) {
		encoder_fn enc = fns[fndx].first;
		decoder_fn dec = fns[fndx].second;

		for (unsigned int dt = 0;
				dt <= sizeof(testdatatype_values)/sizeof(testdatatype); dt++) {
			boost::shared_array<valuet> pts = get_test_data(ptslen, testdatatype_values[dt]);
			//out buffer of same size as test data
			indext outlen = enc(pts.get(), ptslen, buf.get());


			indext retlen = dec(buf.get(), outlen, ptsret.get(), ptslen);

			BOOST_CHECK_EQUAL(retlen, ptslen);
			BOOST_CHECK( memcmp(pts.get(), ptsret.get(),
					ptslen*BSFile::sizemult) == 0 );

			std::cout << "orig contents were:" << std::endl;
			print_data(reinterpret_cast<valuet*>(pts.get()), 100);
			std::cout << std::endl;

			std::cout << "mid contents are:" << std::endl;
			print_data(reinterpret_cast<valuet*>(buf.get()), outlen);
			std::cout << std::endl;

			std::cout << "contents are:" << std::endl;
			print_data(reinterpret_cast<valuet*>(ptsret.get()), 100);
			std::cout << std::endl;

			std::cout << std::endl << std::endl;
		}
	}

	BOOST_TEST_MESSAGE("MESSAGE!!!!");
}

BOOST_AUTO_TEST_SUITE_END()
