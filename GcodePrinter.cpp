/*
 * GcodePrinter.cpp
 *
 *  Created on: Feb 20, 2020
 *      Author: kosa
 */

#include "GcodePrinter.hpp"
#include <fstream>
#include <iomanip>

bool isPowerOfTen(int input) {
	while (input > 9 && input % 10 == 0)
		input /= 10;
	return input == 1;
}

struct pos {

	int val;
	int resolution_in_mm;
	pos(int _val, int _resolution_in_mm) :
			val(_val), resolution_in_mm(_resolution_in_mm) {
	}

	friend std::ostream& operator<<(std::ostream &s, pos const &f) {
		int mm = f.val / f.resolution_in_mm;
		int res = f.val % f.resolution_in_mm;
		int resd = log10(f.resolution_in_mm);
		return s << mm << "." << std::setfill('0') << std::setw(resd) << res
				<< std::setw(0);
	}
};

void insertfile(std::ofstream &file, std::string name)
{
	char ch;
	std::ifstream fs;
	fs.open(name);
	if(!fs)
	{
		std::cerr<<"Error in opening file..!!" << std::endl;
		exit(2);
	}
	file<<fs.rdbuf();
	fs.close();
}

void GcodePrinter(std::string outfile, std::list<std::vector<point>> pts,
		int resolution_in_mm, std::string zup, std::string zdown,
		std::string preamble, std::string postamble) {

	if (!isPowerOfTen(resolution_in_mm)) {
		std::cerr << "Resolution is not power of 10!" << std::endl;
		return;
	}

	std::ofstream file(outfile);
	if (!preamble.empty())
	{
		insertfile(file,preamble);
		file << std::endl;
	}
	file << "G21        ;metric values" << std::endl;
	for (auto contour : pts) {
		auto firstpt = contour.begin();
		if (!zup.empty())
			file << zup << std::endl;
		file << "G0X" << pos(firstpt->x(),resolution_in_mm)
				<< "Y" << pos(firstpt->y(),resolution_in_mm) << std::endl;
		if (!zdown.empty())
			file << zdown << std::endl;
		for(auto pt=firstpt+1; pt != contour.end(); pt++)
		{
			file << "G1X" << pos(pt->x(),resolution_in_mm)
					<< "Y" << pos(pt->y(),resolution_in_mm) << std::endl;
		}
	}
	if (!postamble.empty())
		insertfile(file,postamble);
}
