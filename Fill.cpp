/*
 * Fill.cpp
 *
 *  Created on: Feb 17, 2020
 *      Author: kosa
 */

#include "Fill.hpp"
#include "Contour.hpp"
#include <boost/polygon/interval_concept.hpp>
#include <boost/polygon/isotropy.hpp>
#include <boost/polygon/polygon_set_data.hpp>
#include <boost/polygon/polygon_set_traits.hpp>
#include <cstdio>
#include <list>
#include <vector>

using namespace boost::polygon::operators;

static inline bool modify_pt(point &pt1, point &pt2, point &pt3,
                             const point &prev_pt, const point &current_pt,
                             const point &next_pt, scalar pensize,
                             scalar overlap, bool firstrun) {
  std::pair<boost::polygon::point_data<long double>,
            boost::polygon::point_data<long double>>
      he1 = {},
      ho1, he2 = {}, ho2;
  he1.first.x((long double)(prev_pt.x()));
  he1.first.y((long double)(prev_pt.y()));
  he1.second.x((long double)(current_pt.x()));
  he1.second.y((long double)(current_pt.y()));
  he2.first.x((long double)(current_pt.x()));
  he2.first.y((long double)(current_pt.y()));
  he2.second.x((long double)(next_pt.x()));
  he2.second.y((long double)(next_pt.y()));
  ho1 = he1;
  ho2 = he2;
  long double distance =
      (firstrun) ? (pensize / 2) : ((pensize / 2 - overlap / 2));
  polygon_set::compute_offset_edge(he1.first, he1.second, prev_pt, current_pt,
                                   (distance), -1);
  polygon_set::compute_offset_edge(he2.first, he2.second, current_pt, next_pt,
                                   (distance), -1);
  typedef boost::polygon::scanline_base<long double>::compute_intersection_pack
      pack;
  boost::polygon::point_data<long double> rpt, rpt1, rpt2, rpt3;
  boost::polygon::point_data<long double> bisectorpt(
      (he1.second.x() + he2.first.x()) / 2,
      (he1.second.y() + he2.first.y()) / 2);

  boost::polygon::point_data<long double> orig_pt((long double)current_pt.x(),
                                                  (long double)current_pt.y());
  long double bisector_distance =
      boost::polygon::euclidean_distance(bisectorpt, orig_pt);

  std::pair<boost::polygon::point_data<long double>,
            boost::polygon::point_data<long double>>
      hebisector(orig_pt, bisectorpt);

  if (bisector_distance < distance / 2) {
    if (!pack::compute_lazy_intersection(rpt, he1, he2, true, false)) {

      rpt = he1.second; // colinear offset edges use shared point
      pt1.x((scalar)(std::floor(rpt.x() + 0.5)));
      pt1.y((scalar)(std::floor(rpt.y() + 0.5)));
      //pt2 = pt1;
      pt3 = pt1;
      return false;
    }
  } else {
    rpt = bisectorpt;
  }
  long double scale2 = boost::polygon::euclidean_distance(rpt, orig_pt);

  std::pair<boost::polygon::point_data<long double>,
            boost::polygon::point_data<long double>>
      heb(he1.second, he2.first);

  rpt2 = rpt;
  boost::polygon::deconvolve(rpt, orig_pt);
  boost::polygon::scale_up(rpt, (distance) / scale2);
  boost::polygon::convolve(rpt, orig_pt);
  long double scale4 = boost::polygon::euclidean_distance(orig_pt, rpt);
  long double scale5 = boost::polygon::euclidean_distance(rpt, bisectorpt);

  // if (pack::compute_lazy_intersection(rpt1, he1, he2, false, false)) {
  boost::polygon::deconvolve(heb.first, bisectorpt);
  boost::polygon::deconvolve(heb.second, bisectorpt);
  boost::polygon::scale_up(
      heb.first, (distance) / ((distance - bisector_distance + distance)));
  boost::polygon::scale_up(
      heb.second, (distance) / ((distance - bisector_distance + distance)));
  boost::polygon::convolve(heb.first, rpt);
  boost::polygon::convolve(heb.second, rpt);

  rpt1 = heb.first;
  // rpt2 = rpt;
  rpt3 = heb.second;
  pt3.x((scalar)(std::floor(rpt3.x() + 0.5)));
  pt3.y((scalar)(std::floor(rpt3.y() + 0.5)));
  // pt2.x((scalar)(std::floor(rpt2.x() + 0.5)));
  // pt2.y((scalar)(std::floor(rpt2.y() + 0.5)));
  pt1.x((scalar)(std::floor(rpt1.x() + 0.5)));
  pt1.y((scalar)(std::floor(rpt1.y() + 0.5)));

  return (boost::polygon::scanline_base<scalar>::intersects(he1, he2));
  ;
}

static std::vector<point> plot_poly(const std::vector<point> &poly,
                                    scalar pensize, scalar overlap,
                                    bool no_fillins, bool &non_inverting_edge) {
  std::vector<point> ret;
  ret.reserve(poly.size() * 3 + 1);
  point first_pt = poly[0];
  point second_pt = poly[1];
  point prev_pt = poly[0];
  point current_pt = poly[1];
  point pt1, pt2, pt3;
  for (std::size_t i = 2; i < poly.size() - 1; ++i) {
    point next_pt = poly[i];
    if (modify_pt(pt1, pt2, pt3, prev_pt, current_pt, next_pt, pensize, overlap,
                  no_fillins))
      non_inverting_edge = true;
    ret.push_back(pt1);
    // ret.push_back(pt2);
    ret.push_back(pt3);
    prev_pt = current_pt;
    current_pt = next_pt;
  }
  point next_pt = first_pt;
  if (modify_pt(pt1, pt2, pt3, prev_pt, current_pt, next_pt, pensize, overlap,
                no_fillins))
    non_inverting_edge = true;
  ret.push_back(pt1);
  // ret.push_back(pt2);
  ret.push_back(pt3);
  prev_pt = current_pt;
  current_pt = next_pt;
  next_pt = second_pt;
  if (modify_pt(pt1, pt2, pt3, prev_pt, current_pt, next_pt, pensize, overlap,
                no_fillins))
    non_inverting_edge = true;
  ret.push_back(pt1);
  // ret.push_back(pt2);
  ret.push_back(pt3);
  ret.push_back(ret.front());
  return ret;
}

static inline bool Fillone(std::list<std::vector<point>> &ret, polygon_set &p,
                           scalar pensize, int resize_points, scalar overlap,
                           bool firstrun, bool plot) {
  std::vector<point> resized;
  {
    std::list<polygon_with_holes> polys;

    p.get(polys);
    p.clear();
    if (polys.size() == 0) {
      return false;
    }
    for (std::list<polygon_with_holes>::iterator poly = polys.begin();
         poly != polys.end(); poly++) {
      if (poly->size() > 0) {
        bool not_empty = false;
        {
          resized = plot_poly((*poly).self_.coords_, pensize, overlap, firstrun,
                              not_empty);
          if (plot)
            ret.push_back(resized);
          if (not_empty)
            p.insert_vertex_sequence(resized.begin(), resized.end(),
                                     boost::polygon::COUNTERCLOCKWISE,
                                     false); // inserts without holes
        }
        for (std::list<polygon>::iterator hole = (*poly).holes_.begin();
             hole != (*poly).holes_.end(); ++hole) {

          bool not_empty_h;
          resized = plot_poly((*hole).coords_, pensize, overlap, firstrun,
                              not_empty_h);
          if (plot)
            ret.push_back(resized);
          if (not_empty)
            p.insert_vertex_sequence(resized.begin(), resized.end(),
                                     boost::polygon::CLOCKWISE,
                                     true); // inserts without holes
        }
      }
    }
  }
  return true;
}

std::list<std::vector<point>> Fill(polygon_set in, scalar pensize,
                                   int resize_points, float overlap) {
  std::list<std::vector<point>> ret;

  scalar overlap_ = pensize / 2 * overlap;

  in.clean();
  boost::polygon::simplify(in, 1000);

  in.clean();
  Fillone(ret, in, pensize, resize_points, overlap_, true, true);
  for (int i = 0; true; i++) {
    in.clean();
    while (boost::polygon::simplify(in, 1000))
      in.clean();
    in.clean();
    in.resize(-(pensize - overlap_) / 2, true, resize_points);
    in.clean();
    if (in.empty())
      break;
    if (Fillone(ret, in, pensize, resize_points, overlap_, false, true)) {
    } else {

      break;
    };
    in.clean();
  }
  return ret;
}
