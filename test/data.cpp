/*
 * data.cpp
 *
 *  Created on: Jun 8, 2013
 *      Author: ishafer
 */

#include "data.hpp"

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/shared_array.hpp>

#include <iostream>

boost::shared_array<valuet> get_test_data(int npoints, testdatatype type) {
	boost::random::mt19937 rndgen;
	boost::random::uniform_int_distribution<> dist(1, 100);
	indext i;

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
	case INCREASING_FIXED2:
		for (i = 0; i < npoints; ++i) {
			pts[i] = i * 2;
		}
		break;
	case MIXED:
		for (indext ldx = 0; ldx < npoints - 160; ldx += 160) {
			for (i = ldx; i < ldx+40; ++i) {
				pts[i] = i*2;
			}
			for (i = ldx+40; i < ldx+80; ++i) {
				pts[i] = i*3;
			}
			for (i = ldx+80; i < ldx+120; ++i) {
				pts[i] = dist(rndgen);
			}
			for (i = ldx+120; i < ldx+160; ++i) {
				pts[i] = i % 100;
			}
		}
		//fill in the last bit (not very carefully)
		for (i = std::max(0, npoints - 160); i < npoints; ++i) {
			pts[i] = i % 100;
		}
	}

	return boost::shared_array<valuet>(pts);
}

void print_data(valuet *dta, int npoints) {
	std::cerr << "[ ";
	for (int i = 0; i < npoints; ++i) {
		std::cerr << dta[i] << " ";
	}
	std::cerr << "]";
}
