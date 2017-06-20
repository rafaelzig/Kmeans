#include "Centroid.h"
#include <sstream>
#include <fstream>
#include <random>
#include <mpi.h>
#include <set>

static const char* const OUTPUT_BIN = "temp.bin";
static const char* const INPUT = "kddcup.testdata.unlabeled.txt";
static const char* const OUTPUT = "kddcup.testdata.unlabeled.mpi.result.csv";
//static const char* const INPUT = "kddcup.testdata.unlabeled_10_percent.txt";
//static const char* const OUTPUT = "kddcup.testdata.unlabeled_10_percent.mpi.result.csv";
static const int K = 5;
//static const int K = 2;
static const int ROOT = 0;

using namespace std;

size_t getOffset(size_t totalPoints, int nodeID, int nodeCount)
{
	size_t partitionOffset = 0;

	if (nodeID >= 0)
	{
		if (nodeID < totalPoints % nodeCount)
		{
			++partitionOffset;
		}

		partitionOffset = (partitionOffset + totalPoints / nodeCount) * (nodeID + 1);
	}

	return partitionOffset;
}

void loadData(const char* const& filePath, vector<Point>& partition, size_t& totalPoints, int nodeID, int nodeCount)
{
	ifstream file(filePath);

	if (file.is_open())
	{
		double start = (nodeID == ROOT) ? MPI_Wtime() : 0;

		set<string> uniqueLines;
		string line;

		// Eliminate duplicates
		while (getline(file, line))
		{
			uniqueLines.emplace(line);
		}

		file.close();

		totalPoints = uniqueLines.size();
		istringstream stream;
		vector<Point> dataSet;
		dataSet.reserve(totalPoints);

		// Parse unique lines from input file into Point objects
		for (auto&& uniqueLine : uniqueLines)
		{
			stream.str(uniqueLine);
			dataSet.emplace_back(stream);
			stream.clear();
		}

		// Partition dataSet across nodes
		size_t previousOffset = getOffset(totalPoints, nodeID - 1, nodeCount);
		size_t currentOffset = getOffset(totalPoints, nodeID, nodeCount);
		partition.reserve(currentOffset);
		move(dataSet.begin() + previousOffset, dataSet.begin() + currentOffset, back_inserter(partition));

		if (nodeID == ROOT)
		{
			cout << totalPoints << " unique points loaded in " << MPI_Wtime() - start << " seconds." << endl;
		}
	}
	else
	{
		cerr << "Node " << nodeID << ": " << "Unable to open input file at '" << filePath << "'" << endl;
	}
}

void generateRandomCentroids(vector<Centroid>& centroids, vector<Point>& partition, int nodeID, int nodeCount)
{
	centroids.reserve(K);
	default_random_engine generator((unsigned int) (MPI_Wtime() * 1000));
	int previousNode = -1;

	for (int i = 0; i < K; ++i)
	{
		int selectedNode;

		// Let the root select a random node from the group
		if (nodeID == ROOT)
		{
			uniform_int_distribution<int> distribution(0, nodeCount - 1);

			do
			{
				selectedNode = distribution(generator);
			}
			while (selectedNode == previousNode && nodeID != ROOT);
		}

		// Broadcast the selected node to the group
		MPI_Bcast(&selectedNode, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
		previousNode = selectedNode;

		// Let the selected node select a random point from its partition
		if (nodeID == selectedNode)
		{
			centroids.emplace_back(i, partition[uniform_int_distribution<size_t>(0, partition.size() - 1)(generator)]);
		}
		else // Let the other nodes create a placeholder object
		{
			centroids.emplace_back(i);
		}

		// Broadcast the selected point to the group
		MPI_Bcast(centroids[i].getDataPointer(), Point::FEATURES, MPI_FLOAT, selectedNode, MPI_COMM_WORLD);
	}
}

void assignPoints(vector<Point>& partition, vector<Centroid>& centroids)
{
	// Clears points assigned to label in previous iterations
	for (auto&& centroid : centroids)
	{
		centroid.clearCluster();
	}

	// For each point in the partition
	for (auto&& point : partition)
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

void moveCentroids(const vector<Point>& partition, vector<Centroid>& centroids)
{
	// Clear old position of centroids
	for (auto&& centroid : centroids)
	{
		centroid.clearData();
	}

	// For each point in the partition
	for (auto&& point : partition)
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
		// Save a copy to be used as a send buffer during broadcast
		float* dataCopy = Centroid {centroid}.getDataPointer();
		int clusterSizeCopy = centroid.size();

		// Sum all the vectors of the points and cluster sizes across all nodes
		MPI_Allreduce(dataCopy, centroid.getDataPointer(), Point::FEATURES, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
		MPI_Allreduce(&clusterSizeCopy, centroid.getSizePointer(), 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
		centroid.move();
	}
}

void calculateClusters(vector<Point>& partition, int nodeID, int nodeCount)
{
	double start = (nodeID == ROOT) ? MPI_Wtime() : 0.0;
	vector<Centroid> oldCentroids;
	vector<Centroid> centroids;
	generateRandomCentroids(centroids, partition, nodeID, nodeCount);
	int iterations = 0;

	do
	{
		// Save state of last iteration
		oldCentroids = centroids;
		assignPoints(partition, centroids);
		moveCentroids(partition, centroids);

		if (nodeID == ROOT)
		{
			cout << "Iteration " << ++iterations << " results:" << endl;

			for (auto&& centroid : centroids)
			{
				cout << centroid << endl;
			}

			cout << endl;
		}
	}
	while (oldCentroids != centroids);

	if (nodeID == ROOT)
	{
		double elapsed = MPI_Wtime() - start;
		cout << "Calculated in " << elapsed << "s (Average " << (elapsed / iterations) << "s per iteration)" << endl;
	}
}

void saveBinaryOutput(const char* const filePath, vector<Point>& partition, size_t totalPoints, int& nodeID,
					  int& nodeCount)
{
	size_t previousOffset = getOffset(totalPoints, nodeID - 1, nodeCount) * Point::TOTAL_BYTES;

	// Initialise MPI variables
	MPI_File file;
	MPI_Status status;

	// Initiate distributed file writing
	if (!MPI_File_open(MPI_COMM_WORLD, filePath, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &file))
	{
		// For each point in partition
		for (int i = 0; i < partition.size(); ++i)
		{
			int label = partition[i].getLabel();
			float magnitude = partition[i].getMagnitude();

			// Calculate the current offset and write to file
			MPI_Offset begin = previousOffset + i * Point::TOTAL_BYTES;
			MPI_File_write_at(file, begin, partition[i].getDataPointer(), Point::FEATURES, MPI_FLOAT, &status);
			MPI_File_write_at(file, begin + Point::DATA_BYTES, &magnitude, 1, MPI_FLOAT, &status);
			MPI_File_write_at(file, begin + Point::DATA_BYTES + Point::MAGNITUDE_BYTES, &label, 1, MPI_INT, &status);
		}

		MPI_File_close(&file);
	}
	else
	{
		cerr << "Unable to write to file at\n'" << filePath << "'" << endl;
	}
}

void saveOutput(const char* const inputPath, const char* const outputPath, size_t totalPoints)
{
	double start = MPI_Wtime();
	ofstream output(outputPath);
	ifstream input(inputPath, ios::in | ios::binary);

	if (input.is_open() && output.is_open())
	{
		for (int i = 0; i < totalPoints; ++i)
		{
			char buffer[Point::TOTAL_BYTES];
			input.read(buffer, Point::TOTAL_BYTES);
			output << Point {buffer} << '\n';
		}

		input.close();
		output.close();

		if (remove(OUTPUT_BIN))
		{
			cerr << "Error deleting " << OUTPUT_BIN << endl;
		}

		cout << "Output file\n'" << outputPath << "'\ncreated in " << MPI_Wtime() - start << " seconds." << endl;
	}
	else
	{
		cerr << "Unable to create output file." << endl;
	}
}


int main(int argc, char** argv)
{
	int nodeID;
	int nodeCount;
	size_t totalPoints;

	if (!MPI_Init(&argc, &argv) && !MPI_Comm_rank(MPI_COMM_WORLD, &nodeID) &&
		!MPI_Comm_size(MPI_COMM_WORLD, &nodeCount))
	{
		vector<Point> partition;
		loadData(INPUT, partition, totalPoints, nodeID, nodeCount);

		if (partition.size() > 0)
		{
			calculateClusters(partition, nodeID, nodeCount);
			saveBinaryOutput(OUTPUT_BIN, partition, totalPoints, nodeID, nodeCount);

			if (nodeID == ROOT)
			{
				saveOutput(OUTPUT_BIN, OUTPUT, totalPoints);
			}

			MPI_Finalize();
			return EXIT_SUCCESS;
		}
	}

	MPI_Finalize();
	return EXIT_FAILURE;
}