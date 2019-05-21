#pragma once
#pragma pack(push, 1)

#include <cstdint>

typedef struct _LBlock
{
	uint32_t sequence;
	uint32_t timestamp; // �и��� ���� (��: �� ����: 0, 30��: 30000)
	uint16_t charCount;
	uint16_t unk10; // ���ں��� ��ũ�Ǵ� ������ ���(type 3) charCount + 1 ���� ������ ���κ��� ��ũ�� ���(type 1) 0 �� �����
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
	uint8_t length[4]; // uint32_t ��. endianness �ٲ�� �ؼ� uint8_t[4] �� ����...
	char xsyl[4]; // XSYL
	uint32_t unk16;
	uint16_t unk20;
	uint8_t unk22;
	uint8_t pad23;
	char languageCode[3];
	uint8_t pad27;
	char plai[4]; // plai �� �� (text/plain ���� �� 5����, �� 1���ڰ� �߸�)
	uint32_t blockCount;
	uint32_t blockPointer;
	uint8_t pad40[360];
} LMetadata;
static_assert(sizeof(LMetadata) == 400, "Invalid size");

#pragma pack(pop)
