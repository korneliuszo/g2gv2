/*
 * Fill.hpp
 *
 *  Created on: Feb 17, 2020
 *      Author: kosa
 */

#ifndef INCLUDE_FILL_HPP_
#define INCLUDE_FILL_HPP_

#include "polygons.hpp"
#include <memory>

std::list<std::vector<point>> Fill(polygon_set in, scalar pensize, int resize_points, float overlap);



#endif /* INCLUDE_FILL_HPP_ */
