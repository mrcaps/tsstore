/*
 * bs.hpp
 *
 *  Created on: Jun 6, 2013
 *      Author: ishafer
 */

#ifndef BS_HPP_
#define BS_HPP_

#include "util.hpp"

const indext SAME_DEC_PTS = -1;

/**
 * A block store
 */
class BS {
public:
	/**
	 * Add the given points into the block store
	 * @param [in] pts data to add
	 * @param [in] ndecpts number of decoded points
	 * @return true on success
	 */
	virtual bool add(valuet *pts, indext npts, indext ndecpts=SAME_DEC_PTS) = 0;
	/**
	 * Read points from the block store
	 * @param [out] pts data to fill
	 * @param dxmin where to start
	 * @param npts how many points to read
	 * @return true on success
	 */
	virtual indext read(valuet *pts, indext dxmin, indext npts) = 0;
	/**
	 * Find the (first) position of the given value
	 * @param val -(insertion_point) or index
	 * @return the index
	 */
	virtual indext index(valuet val) = 0;
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
