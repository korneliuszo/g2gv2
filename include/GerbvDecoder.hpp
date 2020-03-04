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

polygon_set GerbvDecoder(std::string infile, scalar resolution_in_mm, scalar minsize, int circle_points);
polygon_set GerbvFixedDecoder(std::string infile, scalar resolution_in_mmm, double fixed_dia, int circle_points);

#endif /* GERBVDECODER_HPP_ */
