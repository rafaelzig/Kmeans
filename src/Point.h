//
// Created by Zig on 15/04/2016.
//

#ifndef KDD_RECORD_H
#define KDD_RECORD_H

#include <unordered_map>
#include <iostream>
#include "Mapper.h"

class Point
{
	private:
	void checkBounds(int col) const;

	void normalizeData();

	public:
	Point();

	Point(int label);

	Point(char* buffer);

	Point(std::istringstream& stream);

	bool operator==(const Point& another) const;

	bool operator!=(const Point& another) const;

	bool operator<(const Point& another) const;

	float& operator[](int col);

	float operator[](int col) const;

	friend std::ostream& operator<<(std::ostream& output, const Point& point);

	const float* begin() const;

	const float* end() const;

	double calculateDistance(Point& another) const;

	void setLabel(int label);

	int getLabel() const;

	float getMagnitude() const;

	void clearData();

	float* getDataPointer();

	static const int FEATURES = 41;

	static const int DATA_BYTES = sizeof(float) * FEATURES;

	static const int MAGNITUDE_BYTES = sizeof(float);

	static const int LABEL_BYTES = sizeof(int);

	static const int TOTAL_BYTES = DATA_BYTES + MAGNITUDE_BYTES + LABEL_BYTES;

	static const int UNDEFINED = -1;

	protected:
	static const char DELIMITER = ',';
	static Mapper mapper;
	float data[FEATURES];
	float magnitude;
	int label;
};

#endif //KDD_RECORD_H