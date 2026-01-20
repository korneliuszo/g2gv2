/*
 * Optimize.cpp
 *
 *  Created on: Mar 5, 2020
 *      Author: kosa
 */

#include "Optimize.hpp"

void Optimize(std::list<std::vector<point>> &list, scalar pensize)
{
	for(auto it=list.begin();it != --list.end();)
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
		if(mindist < pensize*2+2)
		{
			it->insert(it->end(),minit->begin(),minit->end());
		}
		else {
			list.insert(++pos,*minit);
			it++;
		}
		list.erase(minit);
	}
}
