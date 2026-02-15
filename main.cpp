/*
 * main.cpp
 *
 *  Created on: Feb 10, 2020
 *      Author: kosa
 */

#include "GerbvDecoder.hpp"
#include "Contour.hpp"
#include "Fill.hpp"
#include "PngRender.hpp"
#include "GcodePrinter.hpp"
#include "Optimize.hpp"
#include "polygons.hpp"
#include "boost/program_options.hpp"
#include <fstream>

using namespace boost::polygon::operators;

int main(int argc, char **argv) {
	scalar resolution_in_mm = 1000000;
	double dpi = 300;
	double pensize = 0.18;
	float overlap = 0.5;
	double delusesfixeddia = -1;
	double outline_fanout = 0;
	std::string zup="";
	std::string zdown="";
	std::string preamble="";
	std::string postamble="";
	int circle_points=50;
	int resize_points=8;

	namespace po = boost::program_options;
	po::options_description desc("Options");
	desc.add_options()("help,h", "Print help messages")
			("add,a", po::value<std::vector<std::string> >(), "add gerber")
			("del,d", po::value<std::vector<std::string> >(), "del gerber")
			("resolution", po::value<scalar>(&resolution_in_mm), "resolution in mm")
			("pensize,P", po::value<double>(&pensize), "pen size in mm")
			("overlap", po::value<float>(&overlap), "overlap - 0-no overlap, 1 - full overlap")
			("dpi", po::value<double>(&dpi), "png dpi")
			("zup,z", po::value<std::string>(&zup), "up command")
			("zdown,Z", po::value<std::string>(&zdown), "down command")
			("outfile,o", po::value<std::string>(), "out gcode")
			("outlinefile", po::value<std::string>(), "out outline gcode")
			("outlinefanout", po::value<double>(&outline_fanout), "outline fanout")
			("pngfile,p", po::value<std::string>(), "out png")
			("delusesfixeddia,D", po::value<double>(&delusesfixeddia), "del uses fixed diameter")
			("transform,t", po::value<int>(), "boost polygon transform (SWAP_XY=4)")
			("x,x", po::value<int>(), "transform X in resolution")
			("y,y", po::value<int>(), "transform Y in resolution")
			("xy0", po::value<std::string>()->implicit_value(""), "move gcode to 0")
			("xyr", po::value<std::string>(), "move gcode to reference")
			("circlepoints,c", po::value<int>(&circle_points), "circle_points")
			("resizepoints,C", po::value<int>(&resize_points), "resize_points")
			("preamble,m", po::value<std::string>(&preamble), "preamble file")
			("postamble,M", po::value<std::string>(&postamble), "postamble file")
			("hackSize", "hack widths to be more than pensize");


	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help")) {
		std::cout << "g2gv2" << std::endl << desc << std::endl;
		return 0;
	}

	polygon_set set;
	scalar pensize_scalar = pensize*resolution_in_mm;
	scalar minsize = vm.count("hackSize") ? pensize_scalar+resolution_in_mm/500 : 0;
	if (vm.count("add")) {
		std::vector<std::string> add = vm["add"].as<std::vector<std::string>>();
		for(auto a: add)
		{
			std::cout << "Reading: " << a << std::endl;
			set += GerbvDecoder(a,resolution_in_mm, minsize, circle_points);
		}
	}

	if (vm.count("del")) {
		std::vector<std::string> del = vm["del"].as<std::vector<std::string>>();
		for(auto d: del)
		{
			std::cout << "Reading: " << d << std::endl;
			if (delusesfixeddia < 0)
				set -= GerbvDecoder(d,resolution_in_mm, minsize, circle_points);
			else
				set -= GerbvFixedDecoder(d,resolution_in_mm,delusesfixeddia, circle_points);
		}
	}
	if (vm.count("transform"))
	{
		boost::polygon::transformation<scalar> tr(
				boost::polygon::axis_transformation::ATR(vm["transform"].as<int>()));
		boost::polygon::transform(set,tr);
	}
	if (vm.count("xy0"))
	{
		rectangle rect;
		set.extents(rect);
		boost::polygon::transformation<scalar> tr(boost::polygon::ll(rect));
		boost::polygon::transform(set,tr);
		if (!vm["xy0"].as<std::string>().empty())
		{
			std::ofstream reference(vm["xy0"].as<std::string>());
			reference << boost::polygon::ll(rect).x() << " " << boost::polygon::ll(rect).y();
		}
	}
	if (vm.count("xyr"))
	{
		std::ifstream reference(vm["xyr"].as<std::string>());
		scalar x,y;
		reference >> x >> y;
		point pt(x,y);
		boost::polygon::transformation<scalar> tr(pt);
		boost::polygon::transform(set,tr);
	}
	if (vm.count("x") && vm.count("y"))
	{
		boost::polygon::transformation<scalar> tr(point(vm["x"].as<int>(),vm["y"].as<int>()));
		boost::polygon::transform(set,tr);
	}
	std::cout << "Polygon set created" << std::endl;
	std::list<std::vector<point>> outline;
	if (vm.count("outlinefile") || vm.count("pngfile"))
	{
		std::cout << "Tracing contour" << std::endl;
		polygon_set tm = set;
		tm.resize(outline_fanout*resolution_in_mm, true, resize_points);
		outline=Contour(tm);
	}
	std::list<std::vector<point>> fill;
	if (vm.count("outfile"))
	{
		std::cout << "Tracing path" << std::endl;
		fill=Fill(set, pensize_scalar, resize_points, overlap);
		std::cout << "Optimizing" << std::endl;
		Optimize(fill,pensize_scalar);
	}
	if (vm.count("outfile"))
	{
		std::cout << "Saving gcode to " << vm["outfile"].as<std::string>() << std::endl;
		GcodePrinter(vm["outfile"].as<std::string>(),fill,resolution_in_mm, zup, zdown, preamble, postamble);
	}
	if (vm.count("outlinefile"))
	{
		std::cout << "Saving gcode to " << vm["outlinefile"].as<std::string>() << std::endl;
		GcodePrinter(vm["outlinefile"].as<std::string>(),outline,resolution_in_mm, zup, zdown, preamble, postamble);
	}
	if (vm.count("pngfile"))
	{
		std::cout << "Saving png to " << vm["pngfile"].as<std::string>() << std::endl;
		std::map<int,std::list<std::vector<point>>> map=
		{
				{0xff0000,outline},
				{0xffffff,fill}
		};
		PngRender(vm["pngfile"].as<std::string>(),map,dpi,resolution_in_mm);
	}
	return 0;

}
