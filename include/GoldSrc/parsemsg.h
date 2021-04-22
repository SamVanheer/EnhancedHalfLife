/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#pragma once

#include "Platform.h"

class Buffer
{
public:
	constexpr Buffer() = default;
	constexpr Buffer(const Buffer&) = default;
	constexpr Buffer& operator=(const Buffer&) = default;

	Buffer(Buffer&&) = delete;
	Buffer& operator=(Buffer&&) = delete;

	constexpr Buffer(byte* buffer, std::size_t size, std::size_t offset = 0)
		: _buffer(buffer)
		, _size(size)
		, _startOffset(offset)
		, _offset(offset)
	{
	}

	Buffer(void* buffer, int size)
		: _buffer(reinterpret_cast<byte*>(buffer))
		, _size(static_cast<int>(size))
	{
	}

	constexpr bool HasOverflowed() const { return nullptr == _buffer || _badRead; }

	void SetHasOverflowed(bool state)
	{
		_badRead = state;
	}

	constexpr bool WillOverflowOnBytes(std::size_t byteCount) const
	{
		return HasOverflowed() || (_offset + byteCount > _size);
	}

	constexpr const byte* GetData() const { return _buffer; }

	constexpr byte* GetData() { return _buffer; }

	constexpr const byte* GetCurrentData() const { return _buffer + _offset; }

	constexpr byte* GetCurrentData() { return _buffer + _offset; }

	constexpr std::size_t GetTotalSize() const { return _size; }

	constexpr std::size_t GetStartOffset() const { return _startOffset; }

	constexpr std::size_t GetCurrentOffset() const { return _offset; }

	constexpr void Advance(std::size_t byteCount)
	{
		_offset += byteCount;
	}

private:
	byte* _buffer = nullptr;
	std::size_t _size = 0;
	std::size_t _startOffset = 0;
	std::size_t _offset = 0;
	bool _badRead = false;
};

class BufferReader
{
public:
	constexpr BufferReader() = default;
	constexpr BufferReader(const BufferReader&) = default;
	constexpr BufferReader& operator=(const BufferReader&) = default;

	BufferReader(BufferReader&&) = delete;
	BufferReader& operator=(BufferReader&&) = delete;

	constexpr BufferReader(byte* buffer, std::size_t size, std::size_t offset = 0)
		: _buffer(buffer, size, offset)
	{
	}

	BufferReader(void* buffer, int size)
		: _buffer(buffer, size)
	{
	}

	constexpr bool HasOverflowed() const { return _buffer.HasOverflowed(); }

	constexpr std::size_t GetTotalSize() const { return _buffer.GetTotalSize(); }

	constexpr std::size_t GetBytesRead() const { return _buffer.GetCurrentOffset() - _buffer.GetStartOffset(); }

	constexpr int ReadChar()
	{
		return ReadValue<signed char>();
	}

	constexpr int ReadByte()
	{
		return ReadValue<byte>();
	}

	constexpr int ReadShort()
	{
		return ReadValue<short>();
	}

	constexpr int ReadWord()
	{
		return ReadShort();
	}

	constexpr int ReadLong()
	{
		return ReadValue<int>();
	}

	constexpr float ReadFloat()
	{
		union U
		{
			byte    b[4];
			float   f;
			int     l;
		};

		U u{};

		u.l = ReadLong();

		return u.f;
	}

	const char* ReadString()
	{
		static char string[2048]{};

		std::size_t l = 0;
		do
		{
			if (_buffer.WillOverflowOnBytes(1))
				break; // no more characters

			const int c = ReadChar();
			if (c == -1 || c == '\0')
				break;
			string[l] = c;
			++l;
		}
		while (l < sizeof(string) - 1);

		string[l] = '\0';

		return string;
	}

	constexpr float ReadCoord()
	{
		return (float)(ReadShort() * (1.0 / 8));
	}

	constexpr float ReadAngle()
	{
		return (float)(ReadChar() * (360.0 / 256));
	}

	constexpr float ReadHiresAngle()
	{
		return (float)(ReadShort() * (360.0 / 65536));
	}

private:
	template<std::size_t Size, std::size_t Offset>
	struct ReadValueByte
	{
		static constexpr void Read(Buffer& buffer, byte* destination)
		{
			destination[Offset] = buffer.GetCurrentData()[Offset];
			ReadValueByte<Size, Offset - 1>::Read(buffer, destination);
		}
	};

	template<std::size_t Size>
	struct ReadValueByte<Size, 0>
	{
		static constexpr void Read(Buffer& buffer, byte* destination)
		{
			destination[0] = buffer.GetCurrentData()[0];
		}
	};

	template<typename T>
	constexpr T ReadValue()
	{
		if (_buffer.WillOverflowOnBytes(sizeof(T)))
		{
			_buffer.SetHasOverflowed(true);
			return -1;
		}

		T value{};
		ReadValueByte<sizeof(T), sizeof(T) - 1>::Read(_buffer, reinterpret_cast<byte*>(&value));

		_buffer.Advance(sizeof(T));

		return value;
	}

private:
	Buffer _buffer;
};

class BufferWriter
{
public:
	BufferWriter() = default;

	constexpr BufferWriter(const BufferWriter&) = default;
	constexpr BufferWriter& operator=(const BufferWriter&) = default;

	BufferWriter(BufferWriter&&) = delete;
	BufferWriter& operator=(BufferWriter&&) = delete;

	constexpr BufferWriter(byte* buffer, std::size_t size, std::size_t offset = 0)
		: _buffer(buffer, size, offset)
	{
	}

	BufferWriter(void* buffer, int size)
		: _buffer(buffer, size)
	{
	}

	constexpr bool HasOverflowed() const { return _buffer.HasOverflowed(); }

	constexpr std::size_t GetTotalSize() const { return _buffer.GetTotalSize(); }

	constexpr std::size_t GetBytesWritten() const { return  _buffer.GetCurrentOffset() - _buffer.GetStartOffset(); }

	constexpr void WriteByte(unsigned char data)
	{
		WriteValue(data);
	}

	constexpr void WriteLong(int data)
	{
		WriteValue(data);
	}

	void WriteString(const char* str)
	{
		if (!str)
			str = "";

		const std::size_t len = strlen(str) + 1;

		if (_buffer.WillOverflowOnBytes(len))
		{
			_buffer.SetHasOverflowed(true);
			return;
		}

		safe_strcpy(reinterpret_cast<char*>(_buffer.GetCurrentData()), str, len);

		_buffer.Advance(len);
	}

private:
	template<std::size_t Size, std::size_t Offset>
	struct WriteValueByte
	{
		static constexpr void Write(Buffer& buffer, const byte* source)
		{
			buffer.GetCurrentData()[Offset] = source[Offset];
			WriteValueByte<Size, Offset - 1>::Write(buffer, source);
		}
	};

	template<std::size_t Size>
	struct WriteValueByte<Size, 0>
	{
		static constexpr void Write(Buffer& buffer, const byte* source)
		{
			buffer.GetCurrentData()[0] = source[0];
		}
	};

	template<typename T>
	constexpr void WriteValue(const T& value)
	{
		if (_buffer.WillOverflowOnBytes(sizeof(T)))
		{
			_buffer.SetHasOverflowed(true);
			return;
		}

		WriteValueByte<sizeof(T), sizeof(T) - 1>::Write(_buffer, reinterpret_cast<const byte*>(&value));

		_buffer.Advance(sizeof(T));
	}

private:
	Buffer _buffer;
};
