/*
 * Contour.cpp
 *
 *  Created on: Feb 17, 2020
 *      Author: kosa
 */

#include "Contour.hpp"

std::list<std::vector<point>> Contour(polygon_set in) {
	std::list<std::vector<point>> ret;
	std::vector<polygon_with_holes> polys;
	polys.clear();
	in.get(polys);
	for (auto it = polys.begin(); it < polys.end(); it++) {
		std::vector<point> pts;
		pts.insert(pts.end(), boost::polygon::begin_points(*it),
				boost::polygon::end_points(*it));
		pts.push_back(*boost::polygon::begin_points(*it));
		for (auto hole = boost::polygon::begin_holes(*it);
				hole != boost::polygon::end_holes(*it); hole++) {
			std::vector<point> pts_hole;
			pts_hole.insert(pts_hole.end(), hole->begin(), hole->end());
			pts_hole.push_back(*hole->begin());
			ret.push_back(pts_hole);
		}
		ret.push_back(pts);
	}
	return ret;
}
