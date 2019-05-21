#pragma once

#include <Windows.h>
#include <d2d1.h>
#include <dwrite.h>

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")

template<class Interface>
void safeRelease(Interface** ppInterfaceToRelease)
{
	if (*ppInterfaceToRelease != NULL)
	{
		(*ppInterfaceToRelease)->Release();

		(*ppInterfaceToRelease) = NULL;
	}
}

class MainWindow
{
public:
	MainWindow();
	~MainWindow();

	void initialize(HINSTANCE hInstance);
	int runMessageLoop();
	LRESULT wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	void createDeviceIndependentResources();
	void createDeviceResources();
	void discardDeviceResources();

	void onPaint();
	void onResize(UINT width, UINT height) const;


public:
	static MainWindow* pMainWindow;

private:
	HWND hWnd = nullptr;

	ID2D1Factory* pDirect2DFactory = nullptr;
	IDWriteFactory* pDirectWriteFactory = nullptr;
	ID2D1DCRenderTarget* pRenderTarget = nullptr;

	IDWriteTextFormat* pTextFormat = nullptr;

	ID2D1SolidColorBrush* pBlackBrush = nullptr;
	// ID2D1SolidColorBrush* pCornflowerBlueBrush = nullptr;
};

