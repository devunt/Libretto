#pragma once

#include <memory>
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
	bool isActive() const;

	[[nodiscard]] std::unique_ptr<LMetadata> getMetadata() const;
	[[nodiscard]] std::vector<LBlock> getBlocks(const LMetadata& metadata) const;
	[[nodiscard]] int getTimestamp() const;
	[[nodiscard]] std::wstring getTitleAndArtist() const;
	[[nodiscard]] std::wstring getAlbum() const;

private:
	HANDLE hProcess = nullptr;
	HMODULE hModule = nullptr;

	DWORD metadataAddr = 0;
	DWORD timestampAddr = 0;
	DWORD titleAndArtistAddr = 0;
	DWORD albumAddr = 0;

	static const Pattern PATTERN_METADATA;
	static const Pattern PATTERN_TIMESTAMP;

	void readMelonMemory(DWORD addr, LPVOID buffer, SIZE_T size) const;
};
