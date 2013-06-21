/*
 * encoder.hpp
 *
 *  Created on: Jun 21, 2013
 *      Author: mrcaps
 */

#ifndef ENCODER_HPP_
#define ENCODER_HPP_

#include "../util.hpp"

typedef indext (*encoder_fn)(valuet*, indext, char*);
typedef indext (*decoder_fn)(char*, indext, valuet*, indext);
typedef std::pair<encoder_fn, decoder_fn> encdec_pair;

indext delta_rle_encode(valuet *in, indext inlen, char *out);
indext delta_rle_decode(char *in, indext inlen, valuet *out, indext outlen);


#endif /* ENCODER_HPP_ */
