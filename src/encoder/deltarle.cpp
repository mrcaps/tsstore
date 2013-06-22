/*
 * deltarle.cpp
 *
 *  Created on: Jun 21, 2013
 *      Author: mrcaps
 */

#include "encoder.hpp"

#include "../util.hpp"

/**
 * Encode a block with delta-RLE (currently with fixed-width encoding)
 * @return the size written to the output
 */
filepost delta_rle_encode(valuet *in, indext inlen, char *out) {
	valuet *outvt = reinterpret_cast<valuet*>(out);

	if (inlen == 0) {
		return 0;
	}
	outvt[0] = in[0];
	if (inlen == 1) {
		return 1;
	}
	outvt[1] = in[1] - in[0];
	if (inlen == 2) {
		return 2;
	}
	outvt[2] = in[2] - in[1];
	if (inlen == 3) {
		return 3;
	}

	indext pw = 3; //points written
	for (indext i = 3; i < inlen; ++i) {
		valuet delta = in[i] - in[i-1];
		if (outvt[pw-2] == outvt[pw-3]) {
			if (delta == outvt[pw-2]) {
				outvt[pw-1]++;
			} else {
				outvt[pw++] = delta;
			}
		} else if (outvt[pw-1] == outvt[pw-2]) {
			if (delta == outvt[pw-1]) {
				outvt[pw++] = 1;
			} else {
				outvt[pw++] = 0;
				outvt[pw++] = delta;
			}
		} else {
			outvt[pw++] = delta;
		}
	}

	return pw*SIZEMULT;
}

/**
 * Decode a block with delta-RLE
 * @return the number of values decoded
 */
indext delta_rle_decode(char *in, filepost inlen, valuet *out, indext outlen) {
	valuet *invt = reinterpret_cast<valuet*>(in);

	if (inlen == 0) {
		return 0;
	}
	indext ptsread = 0;
	valuet cur = invt[0];
	out[ptsread++] = cur;
	for (indext i = 1; i < inlen/SIZEMULT; ++i) {
		cur += invt[i];
		out[ptsread++] = cur;
		if (i > 1 && invt[i] == invt[i-1]) {
			++i;
			for (indext rdx = 0; rdx < invt[i]; ++rdx) {
				cur += invt[i-1];
				out[ptsread++] = cur;
			}
		}
	}

	return ptsread;
}
