#pragma once
#pragma pack(push, 1)

#include <cstdint>

typedef struct _LBlock
{
	uint32_t sequence;
	uint32_t timestamp; // 밀리초 단위 (예: 곡 시작: 0, 30초: 30000)
	uint16_t charCount;
	uint16_t unk10; // 글자별로 싱크되는 가사일 경우(type 3) charCount + 1 값을 가지고 라인별로 싱크될 경우(type 1) 0 이 저장됨
	uint8_t unk12[308];
	char text[150];
	uint8_t unk470[150];
} LBlock;
static_assert(sizeof(LBlock) == 620, "Invalid size");

typedef struct _LMetadata
{
	char magic[3]; // ID3
	uint8_t pad3;
	uint8_t unk4;
	uint8_t unk5;
	uint8_t unk6;
	uint8_t pad7;
	uint8_t length[4]; // uint32_t 임. endianness 바꿔야 해서 uint8_t[4] 로 저장...
	char xsyl[4]; // XSYL
	uint32_t unk16;
	uint16_t unk20;
	uint8_t unk22;
	uint8_t pad23;
	char languageCode[3];
	uint8_t pad27;
	char plai[4]; // plai 가 들어감 (text/plain 에서 앞 5글자, 뒤 1글자가 잘림)
	uint32_t blockCount;
	uint32_t blockPointer;
	uint8_t pad40[360];
} LMetadata;
static_assert(sizeof(LMetadata) == 400, "Invalid size");

#pragma pack(pop)
