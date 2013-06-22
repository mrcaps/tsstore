/*
 * zlib.cpp
 *
 *  Created on: Jun 21, 2013
 *      Author: mrcaps
 */

#include "encoder.hpp"

#include <zlib.h>

#include "../util.hpp"

filepost zlib_encode(valuet *in, indext inlen, char *out) {
	uLongf outsize = (long) inlen*SIZEMULT*1.1;
	int rc = compress(reinterpret_cast<Bytef*>(out),
			&outsize, reinterpret_cast<Bytef*>(in), inlen*SIZEMULT);
	if (rc != Z_OK) {
		ERROR("ZLib encode failed; retcode=" << rc);
	}
	return outsize;
}

indext zlib_decode(char *in, filepost inlen, valuet *out, indext outlen) {
	uLongf outsize = (long) outlen*SIZEMULT*1.1;
	int rc = uncompress(reinterpret_cast<Bytef*>(out),
			&outsize, reinterpret_cast<Bytef*>(in),
			inlen);
	if (rc != Z_OK) {
		ERROR("ZLib decode failed; retcode=" << rc);
	}
	return outsize/SIZEMULT;
}
