/*
 * Fill.cpp
 *
 *  Created on: Feb 17, 2020
 *      Author: kosa
 */

#include "Fill.hpp"

using namespace boost::polygon::operators;

std::list<std::vector<point>> Fillone(polygon_with_holes in, int pensize) {
	std::list<std::vector<point>> ret;
	polygon_set p;
	p += in;

	while (true) {
		std::vector<polygon_with_holes> polys;
		polys.clear();
		p.get(polys);
		if (polys.size() == 0)
			break;
		if (polys.size() == 1) {
			polygon_with_holes poly = polys.front();
			if (poly.size() > 0) {
				std::vector<point> pts;
				std::list<std::vector<point>>::iterator last_pts;
				std::list<std::vector<point>> empty_list = {{point(0,0)}};
				if (ret.size() == 0)
					last_pts= --empty_list.end();
				else
					last_pts = --ret.end();
				pts.insert(pts.end(), boost::polygon::begin_points(poly),
						boost::polygon::end_points(poly));
				pts.push_back(*boost::polygon::begin_points(poly));
				if (boost::polygon::euclidean_distance(
						pts.front(),
						last_pts->back()) < pensize) {
					last_pts->insert(last_pts->end(), pts.begin(), pts.end());
				} else {
					ret.push_back(pts);
				}
			}
			for (auto hole = boost::polygon::begin_holes(poly);
					hole != boost::polygon::end_holes(poly); hole++) {
				std::vector<point> pts_hole;
				pts_hole.insert(pts_hole.end(), hole->begin(), hole->end());
				if (hole->size() > 0)
					pts_hole.push_back(*hole->begin());
				ret.push_front(pts_hole);
			}
			//if (poly.size() < 4)
			//	break;
			//polys.resize(1);
			//p.set(polys.begin(),polys.end());
			p.resize(-pensize / 2,true,5);
		} else {
			for (auto it = polys.begin(); it != polys.end(); it++) {
				auto b = Fillone(*it, pensize);
				ret.insert(ret.begin(), b.begin(), b.end());
			}
			break;
		}
	}
	return ret;
}

std::list<std::vector<point>> Fill(polygon_set in, int pensize) {
	std::list<std::vector<point>> ret;
	std::vector<polygon_with_holes> polys;
	polys.clear();
	in.resize(-pensize / 2, true,5);
	in.get(polys);
	for (auto it = polys.begin(); it < polys.end(); it++) {
		std::list<std::vector<point>> p = Fillone(*it, pensize);
		ret.insert(ret.end(), p.begin(), p.end());
	}
	return ret;
}

