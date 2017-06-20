//
// Created by Zig on 21/04/2016.
//

#include "Mapper.h"

Mapper::Mapper()
{ }

float Mapper::getMapping(int col, std::string& key)
{
	std::unordered_map<std::string, float>& current = dictionaries[col - 1];
	const auto& pair = current.emplace(key, current.size());

	if (pair.second)
	{
		reverseDictionaries[col - 1].emplace(pair.first->second, key);
	}

	return pair.first->second;
}

std::string Mapper::getMapping(int col, float value) const
{
	return reverseDictionaries[col - 1].at(value);
}