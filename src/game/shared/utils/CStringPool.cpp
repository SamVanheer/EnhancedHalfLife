#include <cstring>

#include "CStringPool.hpp"

const char* CStringPool::Allocate(const char* string)
{
	//Treat null pointers as empty strings
	//TODO: need to warn about this!
	if (!string)
	{
		return "";
	}

	const std::string_view source{string};

	if (auto it = _pool.find(source); it != _pool.end())
	{
		return it->second.get();
	}

	auto destination{std::make_unique<char[]>(source.size() + 1)};

	std::strncpy(destination.get(), source.data(), source.size());
	destination[source.size()] = '\0';

	const std::string_view key{destination.get(), source.size()};

	_pool.emplace(key, std::move(destination));

	return key.data();
}

void CStringPool::Clear()
{
	_pool.clear();
}
