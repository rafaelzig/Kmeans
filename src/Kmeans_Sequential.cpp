#include <sysinfoapi.h>
#include "Centroid.h"
#include <sstream>
#include <fstream>
#include <random>
#include <set>

static const char* const INPUT = "kddcup.testdata.unlabeled.txt";
static const char* const OUTPUT = "kddcup.testdata.unlabeled.result.csv";
//static const char* const INPUT = "kddcup.testdata.unlabeled_10_percent.txt";
//static const char* const OUTPUT = "kddcup.testdata.unlabeled_10_percent.result.csv";
static const int K = 5;
//static const int K = 2;

using namespace std;

void loadData(const char* const filePath, vector<Point>& dataSet)
{
	ifstream file(filePath);

	if (file.is_open())
	{
		DWORD start = GetTickCount();
		set<string> uniqueLines;
		string line;

		// Eliminate duplicates
		while (getline(file, line))
		{
			uniqueLines.emplace(line);
		}

		file.close();

		istringstream stream;
		dataSet.reserve(uniqueLines.size());

		// Parse unique lines from input file into Point objects
		for (auto&& uniqueLine : uniqueLines)
		{
			stream.str(uniqueLine);
			dataSet.emplace_back(stream);
			stream.clear();
		}

		cout << dataSet.size() << " unique records successfully loaded in " << GetTickCount() - start << "ms." << endl;
	}
	else
	{
		cout << "Unable to open input file at '" << filePath << "'" << endl;
	}
}

void generateRandomCentroids(vector<Centroid>& centroids, vector<Point>& dataSet)
{
	centroids.reserve(K);
	default_random_engine generator(GetTickCount());
	uniform_int_distribution<size_t> distribution(0, dataSet.size() - 1);
	size_t previous = numeric_limits<size_t>::max();

	for (int i = 0; i < K; ++i)
	{
		size_t random;

		do
		{
			random = distribution(generator);
		}
		while (random == previous);

		centroids.emplace_back(i, dataSet[random]);
		previous = random;
	}
}

void assignPoints(vector<Point>& dataSet, vector<Centroid>& centroids)
{
	// Clears points assigned to centroid in previous iterations
	for (auto&& centroid : centroids)
	{
		centroid.clearCluster();
	}

	// For each point in the dataSet
	for (auto&& point : dataSet)
	{
		double closest = numeric_limits<double>::max();

		// For each centroid
		for (auto&& centroid : centroids)
		{
			// Calculate the distance of current point to current label
			double distance = centroid.calculateDistance(point);

			if (distance < closest)
			{
				point.setLabel(centroid.getLabel());
				closest = distance;
			}
		}
	}
}

void moveCentroids(const vector<Point>& dataSet, vector<Centroid>& centroids)
{
	// Clear old position of centroids
	for (auto&& centroid : centroids)
	{
		centroid.clearData();
	}

	// For each point in the partition
	for (auto&& point : dataSet)
	{
		Centroid& centroid = centroids[point.getLabel()];
		++centroid;

		// Add the value of each column to the label the point belongs
		for (int col = 0; col < Point::FEATURES; ++col)
		{
			centroid[col] += point[col];
		}
	}

	// Move each label to the mean of all their points
	for (auto&& centroid : centroids)
	{
		centroid.move();
	}
}

void calculateClusters(vector<Point>& dataSet)
{
	DWORD start = GetTickCount();
	vector<Centroid> oldCentroids;
	vector<Centroid> centroids;
	generateRandomCentroids(centroids, dataSet);
	int iterations = 0;

	do
	{
		// Save state of last iteration
		oldCentroids = centroids;

		assignPoints(dataSet, centroids);
		moveCentroids(dataSet, centroids);

		cout << "Iteration " << ++iterations << " results:" << endl;

		for (auto&& centroid : centroids)
		{
			cout << centroid << endl;

		}

		cout << endl;
	}
	while (oldCentroids != centroids);

	DWORD elapsed = GetTickCount() - start;
	cout << "Calculated in " << elapsed << "ms (Average " << (elapsed / iterations) << "ms per iteration)" << endl;
}

void saveData(const char* const filePath, vector<Point>& dataSet)
{
	ofstream file(filePath);

	if (file.is_open())
	{
		DWORD start = GetTickCount();

		for (auto&& point : dataSet)
		{
			file << point << "\n";
		}

		file.close();

		cout << "Output file\n'" << filePath << "'\nsuccessfully created in " << GetTickCount() - start << "ms." <<
		endl;
	}
	else
	{
		cout << "Unable to write to file at\n'" << filePath << "'" << endl;
	}
}

int main()
{
	vector<Point> dataSet;
	loadData(INPUT, dataSet);

	if (dataSet.size() > 0)
	{
		calculateClusters(dataSet);
		saveData(OUTPUT, dataSet);

		return EXIT_SUCCESS;
	}

	return EXIT_FAILURE;
}