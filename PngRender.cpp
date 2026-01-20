/*
 * PngRender.cpp
 *
 *  Created on: Feb 17, 2020
 *      Author: kosa
 */

#include "PngRender.hpp"
#include <png++/png.hpp>

void drawline(png::image<png::rgb_pixel> &image, point a, point b,
		double scalefactor, scalar minx, scalar miny, int colour) {
	png::rgb_pixel col((colour>>16)&0xff,(colour>>8)&0xff,(colour>>0)&0xff);
	if (a== b) {
		image[image.get_height()-1-int((a.y() - miny) * scalefactor)]
			  [int((a.x() - minx) * scalefactor)] = col;
		return;
	}
	double dx = (b.x() - a.x()) * scalefactor;
	double dy = (b.y() - a.y()) * scalefactor;
	double step;
	if (std::abs(dx) >= std::abs(dy))
		step = std::abs(dx);
	else
		step = std::abs(dy);
	dx = dx / step;
	dy = dy / step;
	double x = (a.x() - minx) * scalefactor;
	double y = (a.y() - miny) * scalefactor;
	int i = 1;
	while (i <= step || i == 1) {
		image[image.get_height()-1-int(y)][int(x)] = col;
		x = x + dx;
		y = y + dy;
		i = i + 1;
	}
}

void PngRender(std::string outfile,
		std::map<int, std::list<std::vector<point>>> pts, double dpi,
		int resolution_in_mm) {
	scalar minx = 0;
	scalar miny = 0;
	scalar maxx = 0;
	scalar maxy = 0;
	double scalefactor = dpi / 25.4 / resolution_in_mm;
	for (auto& [key, value] : pts) {
		for (auto a : value) {
			for (auto b : a) {
				maxx = std::max(maxx, b.x());
				maxy = std::max(maxy, b.y());
				minx = std::min(minx, b.x());
				miny = std::min(miny, b.y());
			}
		}
	}
	int sizex = (maxx - minx) * scalefactor + 1;
	int sizey = (maxy - miny) * scalefactor + 1;
	std::cout << "PNG:" << sizex << "x" << sizey << std::endl;
	png::image<png::rgb_pixel> image(sizex, sizey);
	for (png::uint_32 y = 0; y < image.get_height(); ++y) {
		for (png::uint_32 x = 0; x < image.get_width(); ++x) {
			image[y][x] = png::rgb_pixel(0,0,0);
		}
	}
	for (auto& [key, value] : pts) {
		for (auto poly : value) {
			auto begin = poly.begin();
			for (auto end = begin + 1; end != poly.end(); end++) {
				drawline(image, *begin, *end, scalefactor, minx, miny, key);
				begin = end;
			}
		}
	}
	image.write(outfile);
}
