#include "MainWindow.h"

#include <dwmapi.h>
#pragma comment(lib, "dwmapi")

using namespace Gdiplus;

MainWindow* MainWindow::pMainWindow = nullptr;

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return MainWindow::pMainWindow->wndProc(hWnd, msg, wParam, lParam);
}

MainWindow::MainWindow()
{
	pMainWindow = this;

	GdiplusStartupInput input;
	GdiplusStartup(&this->gdiToken, &input, nullptr);
}

MainWindow::~MainWindow()
{
	GdiplusShutdown(this->gdiToken);
}

int MainWindow::runMessageLoop()
{
	MSG msg = {};

	while (GetMessage(&msg, nullptr, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return static_cast<int>(msg.wParam);
}

LRESULT MainWindow::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
	case WM_DISPLAYCHANGE:
		this->onPaint();
		return 0;
	case WM_SIZE:
		this->onResize(LOWORD(lParam), HIWORD(lParam));
		return 0;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

void MainWindow::initialize(HINSTANCE hInstance)
{
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW; // CS_NOCLOSE
	wc.lpfnWndProc = &WndProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	// wc.hbrBackground = nullptr;
	wc.hbrBackground = CreateSolidBrush(RGB(0, 255, 0));
	// wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
	wc.lpszClassName = L"LibrettoMainWindow";
	RegisterClassEx(&wc);

	this->hWnd = CreateWindowEx(
		WS_EX_NOACTIVATE | WS_EX_TOPMOST | WS_EX_LAYERED,
		L"LibrettoMainWindow",
		L"hello",
		WS_POPUP,
		500, 500,
		500, 200,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	// SetLayeredWindowAttributes(this->hWnd, RGB(0, 255, 0), 0, LWA_COLORKEY);

	// MARGINS margins = { -5 };
	// DwmExtendFrameIntoClientArea(hWnd, &margins);

	ShowWindow(this->hWnd, SW_SHOWNORMAL);
	UpdateWindow(this->hWnd);

	this->draw();
}

void MainWindow::draw() const
{
	const auto hdc = GetDC(this->hWnd);

	const auto text = L"자 이제 떠나요 공항으로";

	RECT rect;
	GetWindowRect(this->hWnd, &rect);

	const auto hMemDC = CreateCompatibleDC(hdc);
	const auto hBitmap = CreateCompatibleBitmap(hdc, rect.right - rect.left, rect.bottom - rect.top);
	const auto hOldBitmap = static_cast<HBITMAP>(SelectObject(hMemDC, hBitmap));

	Graphics g(hMemDC);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
	g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

	GraphicsPath path;

	FontFamily fontFamily(L"나눔고딕");
	StringFormat format;
	format.SetAlignment(StringAlignmentNear);
	format.SetLineAlignment(StringAlignmentCenter);
	path.AddString(text, wcslen(text), &fontFamily, FontStyleBold, 24, Gdiplus::Point(10, 10), &format);

	Pen penOutline(Color::Black, 6);
	penOutline.SetLineJoin(LineJoinRound);
	g.DrawPath(&penOutline, &path);

	SolidBrush brushGlyph(Color::White);
	g.FillPath(&brushGlyph, &path);

	BLENDFUNCTION bf = { AC_SRC_OVER, NULL, 255, AC_SRC_ALPHA };
	POINT pointZero = { 0, 0};
	SIZE size = { rect.right - rect.left, rect.bottom - rect.top};
	UpdateLayeredWindow(this->hWnd, hdc, nullptr, &size, hMemDC, &pointZero, 0, &bf, ULW_ALPHA);

	SelectObject(hMemDC, hOldBitmap);
	DeleteDC(hMemDC);
	ReleaseDC(this->hWnd, hdc);
	DeleteObject(hBitmap);
}

void MainWindow::onPaint() const
{
}

void MainWindow::onResize(const UINT width, const UINT height) const
{
}
