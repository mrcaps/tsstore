/*
 * data.hpp
 *
 *  Created on: Jun 7, 2013
 *      Author: ishafer
 */

#ifndef DATA_HPP_
#define DATA_HPP_

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

enum testdatatype {
	UNIFORM,
	RANDOM,
	SAWTOOTH
};

valuet *get_test_data(int npoints, testdatatype type) {
	boost::random::mt19937 rndgen;
	boost::random::uniform_int_distribution<> dist(1, 100);
	int i;

	valuet *pts = new valuet[npoints];
	switch (type) {
	case UNIFORM:
		for (i = 0; i < npoints; ++i) {
			pts[i] = 2;
		}
		break;
	case RANDOM:
		for (i = 0; i < npoints; ++i) {
			pts[i] = dist(rndgen);
		}
		break;
	case SAWTOOTH:
		for (i = 0; i < npoints; ++i) {
			pts[i] = i % 100;
		}
		break;
	}

	return pts;
}


#endif /* DATA_HPP_ */
