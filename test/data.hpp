/*
 * data.hpp
 *
 *  Created on: Jun 7, 2013
 *      Author: ishafer
 */

#ifndef DATA_HPP_
#define DATA_HPP_

#include "../src/util.hpp"

enum testdatatype {
	UNIFORM,
	RANDOM,
	SAWTOOTH
};

valuet *get_test_data(int npoints, testdatatype type);
void print_data(valuet* dta, int npoints);

#endif /* DATA_HPP_ */
