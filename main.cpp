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
#include "polygons.hpp"
#include "boost/program_options.hpp"

using namespace boost::polygon::operators;

int main(int argc, char **argv) {
	scalar resolution_in_mm = 1000;
	double dpi = 300;
	double pensize = 0.18;
	double delusesfixeddia = -1;
	double zup=5, zdown=0;

	namespace po = boost::program_options;
	po::options_description desc("Options");
	desc.add_options()("help,h", "Print help messages")
			("add,a", po::value<std::vector<std::string> >(), "add gerber")
			("del,d", po::value<std::vector<std::string> >(), "del gerber")
			("resolution", po::value<scalar>(&resolution_in_mm), "resolution in mm")
			("pensize,P", po::value<double>(&pensize), "pen size in mm")
			("dpi", po::value<double>(&dpi), "png dpi")
			("zup,z", po::value<double>(&zup), "upper position of pen in mm")
			("zdown,Z", po::value<double>(&zdown), "lower position of pen in mm")
			("outfile,o", po::value<std::string>(), "out gcode")
			("pngfile,p", po::value<std::string>(), "out png")
			("delusesfixeddia,D", po::value<double>(&delusesfixeddia), "del uses fixed diameter")
			("transform,t", po::value<int>(), "boost polygon transform (SWAP_XY=4)");

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help")) {
		std::cout << "g2gv2" << std::endl << desc << std::endl;
		return 0;
	}

	polygon_set set;

	if (vm.count("add")) {
		std::vector<std::string> add = vm["add"].as<std::vector<std::string>>();
		for(auto a: add)
		{
			std::cout << "Reading: " << a << std::endl;
			set += GerbvDecoder(a,resolution_in_mm);
		}
	}

	if (vm.count("del")) {
		std::vector<std::string> del = vm["del"].as<std::vector<std::string>>();
		for(auto d: del)
		{
			std::cout << "Reading: " << d << std::endl;
			if (delusesfixeddia < 0)
				set -= GerbvDecoder(d,resolution_in_mm);
			else
				set -= GerbvFixedDecoder(d,resolution_in_mm,delusesfixeddia);
		}
	}
	if (vm.count("transform"))
	{
		boost::polygon::transformation<scalar> tr(
				boost::polygon::axis_transformation::ATR(vm["transform"].as<int>()));
		boost::polygon::transform(set,tr);
	}
	std::cout << "Polygon set created" << std::endl;
	std::list<std::vector<point>> outline;
	if (vm.count("outfile") || vm.count("pngfile"))
	{
		std::cout << "Tracing contour" << std::endl;
		outline=Contour(set);
	}
	std::list<std::vector<point>> fill;
	if (vm.count("outfile"))
	{
		std::cout << "Tracing path" << std::endl;
		fill=Fill(set, pensize*resolution_in_mm);
	}
	if (vm.count("outfile"))
	{
		std::cout << "Saving gcode to " << vm["outfile"].as<std::string>() << std::endl;
		GcodePrinter(vm["outfile"].as<std::string>(),fill,resolution_in_mm, zup, zdown);
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
