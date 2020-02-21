/*
 * GcodePrinter.hpp
 *
 *  Created on: Feb 20, 2020
 *      Author: kosa
 */

#ifndef INCLUDE_GCODEPRINTER_HPP_
#define INCLUDE_GCODEPRINTER_HPP_

#include "polygons.hpp"
#include <string>
#include <memory>

void GcodePrinter(std::string outfile, std::list<std::vector<point>> pts, int resolution_in_mm, double zup, double zdown);





#endif /* INCLUDE_GCODEPRINTER_HPP_ */
