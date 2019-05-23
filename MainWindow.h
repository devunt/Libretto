#pragma once

#include <memory>
#include <string>

#include <Windows.h>
#include <gdiplus.h>

#include "Melon.h"

using namespace Gdiplus;

typedef struct _OverlayContent
{
	std::wstring line1;
	std::wstring line2;
	std::wstring line3;
} OverlayContent;


class MainWindow
{
public:
	MainWindow();
	~MainWindow();

	void initialize(HINSTANCE hInstance);
	int runMessageLoop();
	void startBackgroundTasks() const;
	LRESULT wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	void draw(const OverlayContent& content) const;
	void requestDraw(const OverlayContent& content) const;
	void requestDraw(const OverlayContent* content) const;
	void pollMelon() const;


public:
	static MainWindow* pMainWindow;

private:
	HWND hWnd = nullptr;
	ULONG_PTR gdiToken = 0;

	bool isMoving = false;

	const OverlayContent contentPlaceholder {
		L"",
		L"멜론 가사 오버레이 v0.1",
		L"",
	};

	const std::shared_ptr<Melon> melon = std::make_shared<Melon>();
	LMetadata metadata = {};
	std::vector<LBlock> blocks;

	FontFamily* fontFamily;
	StringFormat* stringFormat;
	SolidBrush *brushGlyphPrimary, *brushGlyphTrivial;
	Pen *penOutlinePrimary, *penOutlineTrivial;
};

