#pragma once

#include <vector>

#include <Windows.h>

#include "Struct.h"
#include "Pattern.h"

class Melon
{
public:
	// Melon();
	~Melon();

	bool setPid(DWORD pid);
	void getMetadata(LMetadata& metadata) const;
	std::vector<LBlock> getBlocks(const LMetadata& metadata) const;
	int getTimestamp() const;

private:
	HANDLE hProcess = nullptr;
	HMODULE hModule = nullptr;

	DWORD metadataAddr = 0;
	DWORD timestampAddr = 0;

	static const Pattern PATTERN_METADATA;
	static const Pattern PATTERN_TIMESTAMP;

	void readProcessMemory(DWORD addr, LPVOID buffer, SIZE_T size) const;
};
