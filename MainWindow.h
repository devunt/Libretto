#pragma once

#include <Windows.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus")

class MainWindow
{
public:
	MainWindow();
	~MainWindow();

	void initialize(HINSTANCE hInstance);
	int runMessageLoop();
	LRESULT wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	void onPaint() const;
	void onResize(UINT width, UINT height) const;
	void draw() const;


public:
	static MainWindow* pMainWindow;

private:
	HWND hWnd = nullptr;

	ULONG_PTR gdiToken = 0;
};

