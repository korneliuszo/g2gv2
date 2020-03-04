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
	std::vector<std::vector<point>> holes_path;
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
			if (holes_path.size() != poly.size_holes())
			{
				for (auto hole: holes_path)
				{
					if (hole.size())
					{
						ret.push_front(hole);
					}
				}
				holes_path.clear();
				holes_path.resize(poly.size_holes());
			}
			for (auto [hole, i ] = std::tuple{boost::polygon::begin_holes(poly), 0};
					hole != boost::polygon::end_holes(poly); hole++, i++) {
				std::vector<point> pts_hole;
				pts_hole.insert(pts_hole.end(), hole->begin(), hole->end());
				if (hole->size() > 0)
					pts_hole.push_back(*hole->begin());
				if (holes_path[i].size() && pts_hole.size() &&
						boost::polygon::euclidean_distance(
						pts_hole.front(),
						holes_path[i].back()) < pensize) {
					std::vector<point> *last_pts = &holes_path[i];
					last_pts->insert(last_pts->end(),pts_hole.begin(),pts_hole.end());
				} else {
					if (holes_path[i].size())
						ret.push_front(holes_path[i]);
					holes_path[i].clear();
					holes_path[i] = pts_hole;
				}

			}
			p.resize(-pensize / 2,true,5);
		} else {
			ret.insert(ret.begin(),holes_path.begin(),holes_path.end());
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

