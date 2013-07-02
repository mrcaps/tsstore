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
#include <boost/tuple/tuple_io.hpp>

#undef DISALLOW_EVIL_CONSTRUCTORS
#define DISALLOW_EVIL_CONSTRUCTORS(TypeName) \
   TypeName(const TypeName&); \
   void operator=(const TypeName&)

typedef uint64_t streamid;
typedef int64_t valuet;
typedef int64_t indext;
typedef int64_t filepost;
const indext NO_INDEX = -1;

typedef char streamt;
const int SIZEMULT = (sizeof(valuet)/sizeof(streamt));

struct dxrange {
	indext start;
	indext len;

	dxrange() : start(0), len(0) {}
	dxrange(indext _start, indext _len) : start(_start), len(_len) {}
};

template<typename T>
inline void print_vector(std::vector<T> vec) {
	std::cout << "[";
	for (typename std::vector<T>::iterator it = vec.begin(); it != vec.end(); it++) {
		std::cout << *it << " ";
	}
	std::cout << "]";
}

#undef TMP_PATH
#define TMP_PATH() \
	(boost::filesystem::unique_path("_%%%%-%%%%-%%%%.tmp"))

#define tup_fst(X) (boost::tuples::get<0>(X))
#define tup_snd(X) (boost::tuples::get<1>(X))

//#define DEBUG_BUILD

#undef DEBUG
#ifdef DEBUG_BUILD
#  define DEBUG(x) std::cout << x << std::endl;
#else
#  define DEBUG(x)
#endif

#define VERBOSE_BUILD

#undef VERBOSE
#ifdef VERBOSE_BUILD
#  define INFO(x) std::cout << x << std::endl;
#else
#  define INFO(x)
#endif

#define ERROR(x) std::cerr << x << std::endl;

#endif /* UTIL_HPP_ */
