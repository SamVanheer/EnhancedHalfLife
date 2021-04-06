#pragma once

#include <memory>
#include <string_view>
#include <unordered_map>

class CStringPool final
{
public:
	CStringPool() = default;
	~CStringPool() = default;

	const char* Allocate(const char* string);

	void Clear();

private:
	std::unordered_map<std::string_view, std::unique_ptr<char[]>> _pool;
};