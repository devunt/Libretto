#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

#include <Windows.h>

enum PatternOffsetType
{
	PatternOffsetTypeRelativeToBegin,
	PatternOffsetTypeRelativeToEnd,
};

struct Pattern
{
	std::vector<uint8_t> bytes;
	PatternOffsetType offsetType;
	int offset;
};

class PatternSearcher
{
public:
	void setHandle(const HANDLE hProcess)
	{
		this->hProcess = hProcess;
	}

	void setRegion(const DWORD baseAddr, const int size)
	{
		this->baseAddr = baseAddr;
		this->size = size;
	}

	DWORD search(const Pattern& pattern) const
	{
		const auto endAddr = this->baseAddr + this->size;
		const auto patternSize = pattern.bytes.size();

		uint8_t buf[CHUNK_SIZE];
		for (auto readOffset = this->baseAddr; readOffset < endAddr; readOffset += CHUNK_SIZE)
		{
			const auto readSize = (std::min)(CHUNK_SIZE, endAddr - readOffset);
			ReadProcessMemory(this->hProcess, reinterpret_cast<void*>(readOffset), buf, readSize, nullptr);

			for (auto offset = 0u; offset < readSize - patternSize; offset++)
			{
				auto i = 0u;
				for (; i < patternSize; i++)
					if (pattern.bytes[i] != 0xff && pattern.bytes[i] != buf[offset + i])
						break;

				if (i == patternSize)
				{
					auto matchedOffset = readOffset + offset;

					if (pattern.offsetType == PatternOffsetTypeRelativeToEnd)
						matchedOffset += patternSize;

					matchedOffset += pattern.offset;

					return matchedOffset;
				}
			}
		}

		return 0;
	}

private:
	static const DWORD CHUNK_SIZE = 4096;

	HANDLE hProcess = nullptr;
	DWORD baseAddr = 0;
	int size = 0;
};
