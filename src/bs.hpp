/*
 * bs.hpp
 *
 *  Created on: Jun 6, 2013
 *      Author: ishafer
 */

#ifndef BS_HPP_
#define BS_HPP_

#include <boost/shared_array.hpp>

#include "util.hpp"
#include "encoder/encoder.hpp"

const indext SAME_DEC_PTS = -1;

//TODO: return value blocks
typedef struct {
	dxrange range;
	boost::shared_array<streamt> data;
	encodert encoder;
} value_block;

typedef struct {
	std::vector<value_block> blocks;
} read_result;

/**
 * A block store
 */
class BS {
public:
	/**
	 * Add the given points into the block store.
	 * Also flush any underlying buffers and update metadata
	 * @param [in] pts data to add
	 * @param [in] ndecpts number of decoded points
	 * @return true on success
	 */
	virtual bool add(valuet *pts, indext npts, indext ndecpts=SAME_DEC_PTS) = 0;
	/**
	 * Read an exact range of points from the block store
	 * May require a copy.
	 * @param [out] pts data to fill
	 * @param [in] dxmin where to start, how many ponits
	 * @return true on success
	 */
	virtual dxrange read_exact(valuet *pts, dxrange req) = 0;
	/**
	 * Read possibly more points (surrounding blocks) from the block store
	 * @param [in] dxmin index to start reading
	 * @param [in] npts how many points
	 * @return some blocks of data.
	 */
	virtual read_result read(dxrange req) = 0;
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

	virtual ~BS() {};
};


#endif /* BS_HPP_ */
