/*
 * stream.hpp
 *
 *  Created on: Jun 6, 2013
 *      Author: ishafer
 */

#ifndef STREAM_HPP_
#define STREAM_HPP_

#include <boost/container/vector.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/array.hpp>

#include "util.hpp"

/**
 * A buffered value stream
 */
class ValueStream {
private:
	DISALLOW_EVIL_CONSTRUCTORS(ValueStream);
	boost::shared_ptr<BS> valuestore;
	int bufsize;
	boost::circular_buffer<valuet> buf;
	boost::mutex mutex;

public:
	ValueStream(MDS mds, int _bufsize) : bufsize(_bufsize), buf() {
		buf.set_capacity(bufsize);
	}
	~ValueStream() {
		delete buf;
	}

	/**
	 * Start adding a batch of values
	 */
	void add_start() {
		mutex.lock();
	}

	/**
	 * Add a single value
	 */
	inline void add_value(valuet val) {
		buf.push_back(val);
	}

	/**
	 * Finish adding a batch of values
	 */
	void add_end() {
		mutex.unlock();
	}

	/**
	 * Flush out the entire circular buffer.
	 * 	TODO: consider different flush sizes
	 */
	bool flush() {
		boost::mutex::scoped_lock lock(mutex);

		bool success = true;

		boost::circular_buffer::array_range ar = buf.array_one();
		success &= valuestore->add(ar.first, ar.second);
		ar = buf.array_two();
		success &= valuestore->add(ar.first, ar.second);

		lock.unlock();

		success &= valuestore->flush();

		return success;
	}
};


#endif /* STREAM_HPP_ */
