/*
 * data.hpp
 *
 *  Created on: Jun 7, 2013
 *      Author: ishafer
 */

#ifndef DATA_HPP_
#define DATA_HPP_

#include <boost/shared_array.hpp>

#include "../src/util.hpp"

enum testdatatype {
	UNIFORM,
	RANDOM,
	SAWTOOTH,
	INCREASING_FIXED2, //increasing by 2
	MIXED
};
const testdatatype testdatatype_values[] = {
	UNIFORM,
	RANDOM,
	SAWTOOTH,
	INCREASING_FIXED2,
	MIXED
};

boost::shared_array<valuet> get_test_data(int npoints, testdatatype type);
void print_data(valuet *dta, int npoints);

#endif /* DATA_HPP_ */
