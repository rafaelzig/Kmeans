//
// Created by Zig on 21/04/2016.
//

#ifndef KDD_STRINGMAPPER_H
#define KDD_STRINGMAPPER_H

#include <unordered_map>

class Mapper
{
	public:
	Mapper();

	float getMapping(int col, std::string& key);

	std::string getMapping(int col, float value) const;

	private:
	static const int STR_COLS = 3;
	std::unordered_map<std::string, float> dictionaries[STR_COLS];
	std::unordered_map<float, std::string> reverseDictionaries[STR_COLS];
};

#endif //KDD_STRINGMAPPER_H