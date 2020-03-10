/*
 * polygons.hpp
 *
 *  Created on: Feb 10, 2020
 *      Author: kosa
 */

#ifndef POLYGONS_HPP_
#define POLYGONS_HPP_

#include <boost/polygon/polygon.hpp>
#include <cstdint>

typedef int64_t scalar;
typedef boost::polygon::point_data<scalar> point;
typedef boost::polygon::polygon_set_data<scalar> polygon_set;
typedef boost::polygon::polygon_with_holes_data<scalar> polygon_with_holes;
typedef boost::polygon::polygon_data<scalar> polygon;
typedef boost::polygon::rectangle_data<scalar> rectangle;
//typedef std::pair<point, point> edge;




#endif /* POLYGONS_HPP_ */
