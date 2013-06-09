/*
 * data.cpp
 *
 *  Created on: Jun 8, 2013
 *      Author: ishafer
 */

#include "data.hpp"

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include <iostream>

using namespace std;

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

void print_data(valuet* dta, int npoints) {
	cout << "[ ";
	for (int i = 0; i < npoints; ++i) {
		cout << dta[i] << " ";
	}
	cout << "]";
}
