//
// Created by Zig on 22/04/2016.
//

#ifndef KDD_CENTROID_H
#define KDD_CENTROID_H

#include "Point.h"

class Centroid : public Point
{
	private:
	int clusterSize;

	public:
	Centroid();

	Centroid(int label);

	Centroid(int label, Point const& another);

	friend std::ostream& operator<<(std::ostream&, const Centroid&);

	void clearCluster();

	void move();

	int operator++();

	int operator++(int increment);

	int size() const;

	int* getSizePointer();
};

#endif //KDD_CENTROID_H