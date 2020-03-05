/*
 * Optimize.cpp
 *
 *  Created on: Mar 5, 2020
 *      Author: kosa
 */

#include "Optimize.hpp"

void Optimize(std::list<std::vector<point>> &list)
{
	for(auto it=list.begin();it != --list.end(); it++)
	{
		auto minit = it;
		minit++;
		double mindist=boost::polygon::euclidean_distance(it->back(),minit->front());
		for(auto it2=minit;it2 != list.end(); it2++)
		{
			double testdist;
			if (mindist > (testdist = boost::polygon::euclidean_distance(it->back(),it2->front())))
			{
				minit=it2;
				mindist=testdist;
			}
		}
		auto pos=it;
		list.insert(++pos,*minit);
		list.erase(minit);
	}
}
