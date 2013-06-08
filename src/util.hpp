/*
 * util.hpp
 *
 *  Created on: Jun 5, 2013
 *      Author: mrcaps
 */

#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <inttypes.h>

#include <boost/filesystem.hpp>

#undef DISALLOW_EVIL_CONSTRUCTORS
#define DISALLOW_EVIL_CONSTRUCTORS(TypeName) \
   TypeName(const TypeName&); \
   void operator=(const TypeName&)

typedef uint64_t streamid;
typedef int32_t timet;
typedef int64_t valuet;

#undef TMP_PATH
#define TMP_PATH() \
	(boost::filesystem::unique_path("%%%-%%%%-%%%%.tmp"))

#endif /* UTIL_HPP_ */
