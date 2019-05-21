#include "MainWindow.h"

#include <map>
#include <thread>

#include <gdiplus.h>

#include "Util.h"

#define WM_APP_LIBRETTO_DRAW WM_APP + 1

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
	case WM_APP_LIBRETTO_DRAW:
		{
			const auto content = reinterpret_cast<OverlayContent*>(lParam);
			this->draw(*content);
			delete content;
		}
		return 0;
	case WM_LBUTTONDOWN:
		SendMessage(this->hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
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
	wc.lpszClassName = L"LibrettoMainWindow";
	RegisterClassEx(&wc);

	this->hWnd = CreateWindowEx(
		WS_EX_NOACTIVATE | WS_EX_TOPMOST | WS_EX_LAYERED,
		L"LibrettoMainWindow",
		L"LibrettoMainWindowTitle",
		WS_POPUP,
		500, 500,
		1, 1,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	ShowWindow(this->hWnd, SW_SHOWNORMAL);
	UpdateWindow(this->hWnd);

	OverlayContent content {
		L"hello world",
		L"자 이제 떠나요 공항으로",
		L"ONE",
	};
	this->draw(content);
}

void MainWindow::startBackgroundTasks()
{
	const auto pid = Util::getProcessIdByName(L"melon.exe");
	this->melon->setPid(pid);

	std::thread check([&]() { this->pollMelon(); });
	check.detach();
}

void MainWindow::pollMelon() const
{
	using namespace std::chrono_literals;

	std::map<int, std::wstring> lyricMap;
	std::unique_ptr<LMetadata> currentMetadata;
	auto currentIt = lyricMap.begin();

	auto firstIteration = true;
	while (true)
	{
		auto metadata = this->melon->getMetadata();

		if (firstIteration || currentMetadata->blockPointer != metadata->blockPointer)
		{
			currentMetadata = std::move(metadata);
			const auto blocks = this->melon->getBlocks(*currentMetadata);

			lyricMap.clear();
			for (const auto &block : blocks)
				lyricMap[block.timestamp] = std::wstring(block.text);
		}

		const auto timestamp = melon->getTimestamp();
		auto it = lyricMap.upper_bound(timestamp);
		if (firstIteration || it != currentIt)
		{
			const auto content = new OverlayContent;
			currentIt = it;

			if (it == lyricMap.begin())
			{
				// 아직 첫번째 가사 안 나옴
			}
			else
			{
				if (it != lyricMap.end())
					content->line3 = it->second;

				content->line2 = (--it)->second;

				if (it != lyricMap.begin())
					content->line1 = (--it)->second;
			}

			PostMessage(this->hWnd, WM_APP_LIBRETTO_DRAW, 0, reinterpret_cast<LPARAM>(content));
		}

		firstIteration = false;
		std::this_thread::sleep_for(100ms);
	}
}

void MainWindow::draw(const OverlayContent& content) const
{
	GraphicsPath path;

	FontFamily fontFamily(L"나눔고딕");
	StringFormat format;
	format.SetAlignment(StringAlignmentCenter);
	format.SetLineAlignment(StringAlignmentNear);
	path.AddString(content.line1.c_str(), content.line1.length(), &fontFamily, FontStyleBold, 24, Point(0, 0), &format);

	Rect bounds;
	path.GetBounds(&bounds);
	path.AddString(content.line2.c_str(), content.line2.length(), &fontFamily, FontStyleBold, 24, Point(0, bounds.GetBottom() + 3), &format);

	path.GetBounds(&bounds);
	path.AddString(content.line3.c_str(), content.line3.length(), &fontFamily, FontStyleBold, 24, Point(0, bounds.GetBottom() + 3), &format);

	path.GetBounds(&bounds);

	Matrix matrix;
	matrix.Translate(static_cast<REAL>(-bounds.X), static_cast<REAL>(-bounds.Y));
	path.Transform(&matrix);

	path.GetBounds(&bounds);

	const auto hdc = GetDC(this->hWnd);

	const auto hMemDC = CreateCompatibleDC(hdc);
	const auto hBitmap = CreateCompatibleBitmap(hdc, bounds.Width, bounds.Height);
	const auto hOldBitmap = static_cast<HBITMAP>(SelectObject(hMemDC, hBitmap));

	Graphics g(hMemDC);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
	g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

	Pen penOutline(Color::Black, 6);
	penOutline.SetLineJoin(LineJoinRound);
	g.DrawPath(&penOutline, &path);

	SolidBrush brushGlyph(Color::White);
	g.FillPath(&brushGlyph, &path);

	BLENDFUNCTION bf = { AC_SRC_OVER, NULL, 255, AC_SRC_ALPHA };
	POINT pointZero = { 0, 0 };
	SIZE size = { bounds.Width, bounds.Height };
	UpdateLayeredWindow(this->hWnd, hdc, nullptr, &size, hMemDC, &pointZero, 0, &bf, ULW_ALPHA);

	SelectObject(hMemDC, hOldBitmap);
	DeleteDC(hMemDC);
	ReleaseDC(this->hWnd, hdc);
	DeleteObject(hBitmap);
}
