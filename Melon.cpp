#include <iostream>

#include <Windows.h>
#include <Psapi.h>

#include "Melon.h"
#include "Pattern.h"
#include "Util.h"

const Pattern Melon::PATTERN_METADATA {
	{
		0x68, 0xff, 0xff, 0xff, 0xff,	// push [LyricMetadata]
		0xe8, 0xff, 0xff, 0xff, 0xff,	// call memset
		0x83, 0xc4, 0x0c,				// add esp, 0xc
		0x8b, 0x4d, 0xd8,				// mov ecx, dword ptr [ebp + local_2c]
		0x0f, 0xb6, 0x11,				// movzx edx, byte ptr [ecx]
	},
	PatternOffsetTypeRelativeToBegin,
	1
};

const Pattern Melon::PATTERN_TIMESTAMP {
	{
		0x55,								// push ebp
		0x8b, 0xec,							// mov ebp, esp
		0xdd, 0x05, 0xff, 0xff, 0xff, 0xff,	// fld qword ptr [CurrentTimestamp]
		0x5d,								// pop ebp
		0xc3,								// ret
	},
	PatternOffsetTypeRelativeToBegin,
	5
};

bool Melon::setPid(const DWORD pid)
{
	this->hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
	this->hModule = Util::getModuleHandleByName(pid, L"SMLFDLL.dll");

	if (this->hModule == nullptr)
	{
		CloseHandle(this->hProcess);
		return false;
	}

	// printf("hProcess: %p, hModule: %p\n", this->hProcess, this->hModule);

	/*
	MODULEINFO modInfo;
	GetModuleInformation(this->hProcess, this->hModule, &modInfo, sizeof(modInfo));
	// printf("BaseAddress: %p, Size: %lu\n", modInfo.lpBaseOfDll, modInfo.SizeOfImage);

	PatternSearcher patternSearcher;
	patternSearcher.setHandle(this->hProcess);
	patternSearcher.setRegion(reinterpret_cast<DWORD>(modInfo.lpBaseOfDll), modInfo.SizeOfImage);

	const auto metadataPointerAddr = patternSearcher.search(PATTERN_METADATA);
	const auto timestampPointerAddr = patternSearcher.search(PATTERN_TIMESTAMP);
	// printf("metadataPointerAddr: %X, timestampPointerAddr: %X\n", metadataPointerAddr, timestampPointerAddr);

	this->readMelonMemory(metadataPointerAddr, &this->metadataAddr, sizeof(DWORD));
	this->readMelonMemory(timestampPointerAddr, &this->timestampAddr, sizeof(DWORD));
	// printf("metadataAddr: %X, timestampAddr: %X\n", metadataAddr, timestampAddr);

	// printf("\n");
	*/

	const auto baseAddr = reinterpret_cast<DWORD>(this->hModule);
	this->metadataAddr = baseAddr + 0x19058;
	this->timestampAddr = baseAddr + 0x191F8;
	this->titleAndArtistAddr = baseAddr + 0x19458;
	this->albumAddr = baseAddr + 0x1945C;

	return true;
}

bool Melon::isActive() const
{
	if (this->hProcess == nullptr)
		return false;

	DWORD exitCode;
	return GetExitCodeProcess(this->hProcess, &exitCode) && exitCode == STILL_ACTIVE;
}

std::unique_ptr<LMetadata> Melon::getMetadata() const
{
	auto metadata = std::make_unique<LMetadata>();
	this->readMelonMemory(this->metadataAddr, metadata.get(), sizeof(LMetadata));
	return metadata;
}

std::vector<LBlock> Melon::getBlocks(const LMetadata& metadata) const
{
	std::vector<LBlock> blocks{};

	if (metadata.blockPointer == 0)
		return blocks;

	for (auto i = 0u; i < metadata.blockCount; i++)
	{
		LBlock block;
		this->readMelonMemory(metadata.blockPointer + (i * sizeof(LBlock)), &block, sizeof(LBlock));
		blocks.push_back(block);
	}

	return blocks;
}

int Melon::getTimestamp() const
{
	double n;
	this->readMelonMemory(this->timestampAddr, &n, sizeof(double));
	return static_cast<int>(n * 1000);
}

std::wstring Melon::getTitleAndArtist() const
{
	DWORD addr;
	this->readMelonMemory(this->titleAndArtistAddr, &addr, sizeof(DWORD));

	std::vector<wchar_t> v;
	wchar_t c;
	do
	{
		this->readMelonMemory(addr, &c, sizeof(wchar_t));
		v.push_back(c);
		addr += sizeof(wchar_t);
	} while (c > 0);

	return std::wstring(v.data());
}

std::wstring Melon::getAlbum() const
{
	DWORD addr;
	this->readMelonMemory(this->albumAddr, &addr, sizeof(DWORD));

	std::vector<wchar_t> v;
	wchar_t c;
	do
	{
		this->readMelonMemory(addr, &c, sizeof(wchar_t));
		v.push_back(c);
		addr += sizeof(wchar_t);
	} while (c > 0);

	return std::wstring(v.data());
}

void Melon::readMelonMemory(const DWORD addr, const LPVOID buffer, const SIZE_T size) const
{
	ReadProcessMemory(this->hProcess, reinterpret_cast<void*>(addr), buffer, size, nullptr);
}

Melon::~Melon()
{
	CloseHandle(this->hProcess);
}
