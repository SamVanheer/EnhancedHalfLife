#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>

#include "Platform.hpp"

/**
*	@brief Allows reading binary data from a buffer
*/
class BinaryReader
{
public:
	constexpr BinaryReader() noexcept = default;
	constexpr BinaryReader(const byte* data, std::size_t sizeInBytes) noexcept
		: _data(data)
		, _sizeInBytes(sizeInBytes)
	{
	}

	~BinaryReader() noexcept = default;

	[[nodiscard]] constexpr const byte* GetData() const noexcept { return _data; }

	[[nodiscard]] constexpr std::size_t GetSizeInBytes() const noexcept { return _sizeInBytes; }

	[[nodiscard]] constexpr std::size_t GetOffset() const noexcept { return _offset; }

	[[nodiscard]] std::size_t Read(byte* destination, std::size_t destinationSizeInBytes) noexcept
	{
		if (!destination || destinationSizeInBytes == 0)
		{
			return 0;
		}

		const std::size_t bytesToRead = std::min((_sizeInBytes - _offset), destinationSizeInBytes);

		std::memcpy(destination, _data + _offset, bytesToRead);

		_offset += bytesToRead;

		return bytesToRead;
	}

	std::int32_t ReadInt32()
	{
		return ReadValueOrThrow<std::int32_t>();
	}

	std::uint32_t ReadUInt32()
	{
		return ReadValueOrThrow<std::uint32_t>();
	}

	float ReadSingle()
	{
		return ReadValueOrThrow<float>();
	}

	template<typename T>
	T* ReadArray(T* array, std::size_t elementCount)
	{
		const std::size_t bytesToRead = sizeof(T) * elementCount;
		const std::size_t bytesRead = Read(reinterpret_cast<byte*>(array), bytesToRead);

		if (bytesRead != bytesToRead)
		{
			throw std::runtime_error("End of buffer reached");
		}

		return array;
	}

private:
	template<typename T>
	[[nodiscard]] T ReadValueOrThrow()
	{
		T value{};

		const std::size_t bytesToRead = sizeof(T);
		const std::size_t bytesRead = Read(reinterpret_cast<byte*>(&value), bytesToRead);

		if (bytesRead != bytesToRead)
		{
			throw std::runtime_error("End of buffer reached");
		}

		return value;
	}

private:
	const byte* const _data = nullptr;
	const std::size_t _sizeInBytes = 0;
	std::size_t _offset = 0;
};
