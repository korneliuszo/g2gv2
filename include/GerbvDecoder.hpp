/*
 * GerbvDecoder.hpp
 *
 *  Created on: Feb 10, 2020
 *      Author: kosa
 */

#ifndef GERBVDECODER_HPP_
#define GERBVDECODER_HPP_

#include "polygons.hpp"
#include <string>

polygon_set GerbvDecoder(std::string infile, scalar resolution_in_mm, scalar minsize);
polygon_set GerbvFixedDecoder(std::string infile, scalar resolution_in_mmm, double fixed_dia);

#endif /* GERBVDECODER_HPP_ */
