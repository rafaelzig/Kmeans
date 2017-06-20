//
// Created by Rafael on 22/04/2016.
//

#include "Centroid.h"

Centroid::Centroid() : clusterSize(0)
{ }

Centroid::Centroid(int label) : Point(label)
{
	clusterSize = 0;
}

Centroid::Centroid(int label, Point const& another) : Point(another)
{
	this->label = label;
	clusterSize = 0;
}

std::ostream& operator<<(std::ostream& output, const Centroid& centroid)
{
	return output << "Cluster " << centroid.getLabel() << " -> " << centroid.clusterSize << " points";
}

void Centroid::clearCluster()
{
	clusterSize = 0;
}

void Centroid::move()
{
	for (auto&& value : data)
	{
		value /= clusterSize;
	}
}

// prefix ++ operator
int Centroid::operator++()
{
	return ++clusterSize;
}

// postfix ++ operator
int Centroid::operator++(int)
{
	int old = clusterSize;
	++clusterSize;

	return old;
}

int Centroid::size() const
{
	return clusterSize;
}

int* Centroid::getSizePointer()
{
	return &clusterSize;
}