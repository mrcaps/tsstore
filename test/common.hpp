/*
 * common.hpp
 *
 *  Created on: Aug 22, 2013
 *      Author: mrcaps
 */

#ifndef COMMON_HPP_
#define COMMON_HPP_

#define SHORT_TESTS

#ifdef SHORT_TESTS
#define LONG_TEST return;
#else
#define LONG_TEST ;
#endif

#endif /* COMMON_HPP_ */
