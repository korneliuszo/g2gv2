/*
 * PngRender.hpp
 *
 *  Created on: Feb 17, 2020
 *      Author: kosa
 */

#ifndef INCLUDE_PNGRENDER_HPP_
#define INCLUDE_PNGRENDER_HPP_
#include <memory>
#include "polygons.hpp"

void PngRender(std::string outfile, std::map<int ,std::list<std::vector<point>>> pts, double dpi, scalar resolution_in_mm);






#endif /* INCLUDE_PNGRENDER_HPP_ */
