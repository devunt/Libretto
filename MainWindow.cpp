#include "MainWindow.h"

#include <dwmapi.h>
#pragma comment(lib, "dwmapi")


MainWindow* MainWindow::pMainWindow = nullptr;

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return MainWindow::pMainWindow->wndProc(hWnd, msg, wParam, lParam);
}

MainWindow::MainWindow()
{
	pMainWindow = this;
}

MainWindow::~MainWindow()
{
	safeRelease(&pDirect2DFactory);
	safeRelease(&pDirectWriteFactory);
	safeRelease(&pRenderTarget);
	safeRelease(&this->pBlackBrush);
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

	createDeviceIndependentResources();

	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW; // CS_NOCLOSE
	wc.lpfnWndProc = &WndProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = CreateSolidBrush(RGB(255, 0, 0));
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

	SetLayeredWindowAttributes(this->hWnd, RGB(255, 0, 0), 0, LWA_COLORKEY);

	MARGINS margins = { -1 };
	DwmExtendFrameIntoClientArea(hWnd, &margins);

	ShowWindow(this->hWnd, SW_SHOWNORMAL);
	UpdateWindow(this->hWnd);
}

void MainWindow::createDeviceIndependentResources()
{
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &this->pDirect2DFactory);
	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&this->pDirectWriteFactory));
	this->pDirectWriteFactory->CreateTextFormat(
		L"³ª´®°íµñ",
		nullptr,
		DWRITE_FONT_WEIGHT_BOLD,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		72.0f,
		L"ko-kr",
		&this->pTextFormat
	);
	this->pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	this->pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	this->pTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
}

void MainWindow::createDeviceResources()
{
	if (!this->pRenderTarget)
	{
		const auto renderTargetProperties = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
		this->pDirect2DFactory->CreateDCRenderTarget(&renderTargetProperties, &this->pRenderTarget);
		this->pRenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_ALIASED);

		this->pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow), &this->pBlackBrush);
	}
}

void MainWindow::discardDeviceResources()
{
	safeRelease(&this->pRenderTarget);
	safeRelease(&this->pBlackBrush);
}

void MainWindow::onPaint()
{
	PAINTSTRUCT ps;
	BeginPaint(this->hWnd, &ps);

	const auto text = L"¹è°íÇÁ´Ù";

	RECT rc;
	GetClientRect(this->hWnd, &rc);
	/*const auto rect = D2D1::RectF(
		static_cast<FLOAT>(rc.top),
		static_cast<FLOAT>(rc.left),
		static_cast<FLOAT>(rc.right - rc.left),
		static_cast<FLOAT>(rc.bottom - rc.top)
	);*/

	this->createDeviceResources();

	this->pRenderTarget->BindDC(ps.hdc, &rc);

	this->pRenderTarget->BeginDraw();
	// this->pRenderTarget->SetTransform(D2D1::IdentityMatrix());
	// this->pRenderTarget->Clear(D2D1::ColorF(RGB(255, 0, 0)));
	this->pRenderTarget->Clear(nullptr);
	// this->pRenderTarget->DrawTextW(text, wcslen(text), this->pTextFormat, rect, this->pBlackBrush);
	this->pRenderTarget->EndDraw();

	EndPaint(this->hWnd, &ps);
}

void MainWindow::onResize(const UINT width, const UINT height) const
{
	if (this->pRenderTarget)
	{
		// this->pRenderTarget->Resize(D2D1::SizeU(width, height));
	}
}
