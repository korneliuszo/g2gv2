/*
 * Fill.cpp
 *
 *  Created on: Feb 17, 2020
 *      Author: kosa
 */

#include "Fill.hpp"
#include "Contour.hpp"
#include <boost/polygon/polygon_set_traits.hpp>

using namespace boost::polygon::operators;


void Fillone(std::list<std::vector<point>> & ret, polygon_set p, scalar pensize, int resize_points, float overlap) {
	std::list<polygon_with_holes> polys;

	while (true) {
		polys.clear();
		p.get(polys);
		if (polys.size() == 0)
		{
			break;
		}
		for(auto poly = polys.begin(); poly != polys.end(); poly++) {
			if (poly->size() > 0) {
				ret.emplace_back(poly->begin(), poly->end());
			}

			for (auto hole = poly->begin_holes();
					hole != poly->end_holes(); hole++) {
				ret.emplace_back(hole->begin(), hole->end());
			}
		}
		p.resize(-pensize * (1-overlap),(resize_points>4), resize_points);
		p.clean();
	}
	return;
}

std::list<std::vector<point>> Fill(polygon_set in, int pensize, int resize_points, float overlap) {
	std::list<std::vector<point>> ret;

	in.resize(-pensize/2,(resize_points>4), resize_points);
	in.clean();
	Fillone(ret, in, pensize, resize_points, overlap);
	return ret;
}

