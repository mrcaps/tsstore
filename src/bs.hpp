/*
 * bs.hpp
 *
 *  Created on: Jun 6, 2013
 *      Author: ishafer
 */

#ifndef BS_HPP_
#define BS_HPP_

#include "util.hpp"

/**
 * A block store
 */
class BS {
public:
	/**
	 * Add the given points into the block store
	 * @param [in] pts data to add
	 * @return true on success
	 */
	virtual bool add(valuet* pts, int64_t npts) = 0;
	/**
	 * Read points from the block store
	 * @param [out] pts data to fill
	 * @param dxmin where to start
	 * @param npts how many points to read
	 * @return true on success
	 */
	virtual int64_t read(valuet* pts, int64_t dxmin, int64_t npts) = 0;
	/**
	 * Get the max index stored in this block store
	 */
	virtual int64_t get_maxindex() = 0;

	/**
	 * Flush any underlying buffers and update metadata
	 * @return true on success
	 */
	virtual bool flush() = 0;
	virtual ~BS() {};
};


#endif /* BS_HPP_ */
