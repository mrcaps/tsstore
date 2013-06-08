/*
 * bs.hpp
 *
 *  Created on: Jun 6, 2013
 *      Author: ishafer
 */

#ifndef BS_HPP_
#define BS_HPP_

/**
 * A block store
 */
class BS {
public:
	virtual bool add(valuet* pts, int npts) = 0;
	virtual bool flush() = 0;
	virtual ~BS() {};
};


#endif /* BS_HPP_ */
