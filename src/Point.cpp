//
// Created by Rafael on 15/04/2016.
//

#include <sstream>
#include <cmath>
#include "Point.h"

Point::Point() : Point(UNDEFINED)
{ }

Point::Point(int label) : data(), magnitude(), label(label)
{ }

Point::Point(char* buffer)
{
	std::copy((float*) buffer, ((float*) buffer) + FEATURES, data);
	magnitude = *(float*) (buffer + DATA_BYTES);
	label = *(int*) (buffer + DATA_BYTES + MAGNITUDE_BYTES);
}

Point::Point(std::istringstream& stream)
{
	std::string token;
	double sum = 0.0;

	// Parse values from stream and sum them to calculate vector magnitude
	for (int col = 0; std::getline(stream, token, DELIMITER); ++col)
	{
		data[col] = (col < 1 || col > 3) ? std::stof(token) : mapper.getMapping(col, token);
		sum += powf(data[col], 2.0f);
	}

	magnitude = (float) sqrt(sum);
	normalizeData();
}

bool Point::operator==(const Point& another) const
{
	return std::equal(begin(), end(), another.begin());
}

bool Point::operator!=(const Point& another) const
{
	return !(*this == another);
}

bool Point::operator<(const Point& another) const
{
	return std::lexicographical_compare(begin(), end(), another.begin(), another.end());
}

float& Point::operator[](int col)
{
	checkBounds(col);
	return data[col];
}

float Point::operator[](int col) const
{
	checkBounds(col);
	return data[col];
}

std::ostream& operator<<(std::ostream& output, const Point& point)
{
	for (int col = 0; col < Point::FEATURES; ++col)
	{
		float value = point[col] * point.magnitude;

		// Resolve rounding issues for integer values
		if (col <= 23 || col == 31 || col == 32)
		{
			value = roundf(value);
		}

		// Convert float to string for columns 1 - 2 - 3
		if (col >= 1 && col <= 3)
		{
			output << point.mapper.getMapping(col, value);
		}
		else
		{
			output << value;
		}

		output << point.DELIMITER;
	}

	return output << point.label;
}

void Point::checkBounds(int col) const
{
	if (col < 0 || col >= FEATURES)
	{
		throw std::out_of_range("Index out of bounds");
	}
}

void Point::normalizeData()
{
	if (magnitude > 0.0f)
	{
		for (auto&& value : data)
		{
			value /= magnitude;
		}
	}
}

const float* Point::begin() const
{
	return data;
}

const float* Point::end() const
{
	return data + FEATURES;
}

// Calculate the Euclidean Distance to another point
double Point::calculateDistance(Point& another) const
{
	double distance = 0.0;

	for (int col = 0; col < FEATURES; ++col)
	{
		distance += powf(another[col] - data[col], 2.0f);
	}

	return sqrt(distance);
}

void Point::setLabel(int label)
{
	this->label = label;
}

int Point::getLabel() const
{
	return label;
}

float Point::getMagnitude() const
{
	return magnitude;
}

void Point::clearData()
{
	std::fill(data, data + FEATURES, 0.0f);
}

float* Point::getDataPointer()
{
	return data;
}

Mapper Point::mapper;