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
typedef uint64_t spairid;
typedef int64_t valuet;
typedef int64_t indext;
typedef int64_t filepost;
const indext NO_INDEX = -1;

typedef char streamt;
const int SIZEMULT = (sizeof(valuet)/sizeof(streamt));

#undef TMP_PATH
#define TMP_PATH() \
	(boost::filesystem::unique_path("_%%%%-%%%%-%%%%.tmp"))

//#define DEBUG_BUILD

#undef DEBUG
#ifdef DEBUG_BUILD
#  define DEBUG(x) std::cerr << x << std::endl;
#else
#  define DEBUG(x)
#endif

#define VERBOSE_BUILD

#undef VERBOSE
#ifdef VERBOSE_BUILD
#  define INFO(x) std::cerr << x << std::endl;
#else
#  define INFO(x)
#endif

#define ERROR(x) std::cerr << x << std::endl;

typedef struct {
	spairid stream;
	valuet t;
	valuet v;
} pointt;

/**
 * Range query result
 */
typedef struct {
	std::vector<valuet> ts;
	std::vector<valuet> vs;
	indext npoints;
} qres;

inline bool qres_equal(qres qr1, qres qr2) {
	if (qr1.npoints != qr2.npoints) {
		ERROR("qr1 npoints=" << qr1.npoints << " != qr2 npoints=" << qr2.npoints);
		return false;
	}
	for (indext i = 0; i < qr1.npoints; ++i) {
		if ((qr1.ts[i] != qr2.ts[i]) || (qr1.vs[i] != qr2.vs[i])) {
			ERROR("qres not equal @ index " << i);
			return false;
		}
	}

	return true;
}

struct dxrange {
	indext start;
	indext len;

	dxrange() : start(0), len(0) {}
	dxrange(indext _start, indext _len) : start(_start), len(_len) {}
};

template<typename T>
inline void print_vector(std::vector<T> &vec, int elts_max=-1) {
	std::cerr << "[";
	if (elts_max >= 0) {
		int elt = 0;
		for (typename std::vector<T>::iterator it = vec.begin(); it != vec.end(); it++) {
			if (++elt < elts_max) {
				std::cerr << *it << " ";
			} else {
				break;
			}
		}
	} else {
		for (typename std::vector<T>::iterator it = vec.begin(); it != vec.end(); it++) {
			std::cerr << *it << " ";
		}
	}
	std::cerr << "]";
}

#endif /* UTIL_HPP_ */
