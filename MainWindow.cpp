#include "MainWindow.h"

#include <map>
#include <thread>
#include <optional>

#include "Util.h"

constexpr int WM_APP_LIBRETTO_DRAW = (WM_APP + 1);

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

	PrivateFontCollection pfc{};
	pfc.AddFontFile(L"NanumSquareEB.ttf");

	auto _ = 0;
	pfc.GetFamilies(1, &this->fontFamily, &_);

	this->brushGlyphPrimary = new SolidBrush(Color(255, 255, 255, 255));
	this->penOutlinePrimary = new Pen(Color(255, 0, 0, 0), 5);
	this->penOutlinePrimary->SetLineJoin(LineJoinRound);

	this->brushGlyphTrivial = new SolidBrush(Color(128, 255, 255, 255));
	this->penOutlineTrivial = new Pen(Color(128, 0, 0, 0), 5);
	this->penOutlineTrivial->SetLineJoin(LineJoinRound);

	this->stringFormat = new StringFormat();
	this->stringFormat->SetAlignment(StringAlignmentCenter);
	this->stringFormat->SetLineAlignment(StringAlignmentNear);
}

MainWindow::~MainWindow()
{
	delete this->brushGlyphPrimary;
	delete this->penOutlinePrimary;
	delete this->brushGlyphTrivial;
	delete this->penOutlineTrivial;
	delete this->stringFormat;
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
	wc.style = CS_HREDRAW | CS_VREDRAW;
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
		L"그저 친구라는 수많은 여자친구 right",
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
	std::optional<std::map<int, std::wstring>::iterator> currentIt = std::nullopt;

	while (true)
	{
		auto metadata = this->melon->getMetadata();

		if (currentMetadata == nullptr || currentMetadata->blockPointer != metadata->blockPointer)
		{
			currentMetadata = std::move(metadata);
			const auto blocks = this->melon->getBlocks(*currentMetadata);

			lyricMap.clear();
			currentIt.reset();
			for (const auto &block : blocks)
				lyricMap[block.timestamp] = std::wstring(block.text);
		}

		const auto timestamp = melon->getTimestamp();
		auto it = lyricMap.upper_bound(timestamp);
		if (!currentIt.has_value() || it != *currentIt)
		{
			const auto content = new OverlayContent;
			*currentIt = it;

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

			SendMessage(this->hWnd, WM_APP_LIBRETTO_DRAW, 0, reinterpret_cast<LPARAM>(content));
		}

		std::this_thread::sleep_for(100ms);
	}
}

void MainWindow::draw(const OverlayContent& content) const
{
	constexpr auto windowPadding = 10;
	constexpr auto fontSize = 24;
	constexpr auto lineHeight = 30;
	constexpr auto lineSpacing = 3;

	GraphicsPath pathPrimary, pathTrivial, pathMerged;

	pathTrivial.AddString(content.line1.c_str(), content.line1.length(), &this->fontFamily, FontStyleRegular, fontSize, Point(0, 0), this->stringFormat);
	pathPrimary.AddString(content.line2.c_str(), content.line2.length(), &this->fontFamily, FontStyleRegular, fontSize, Point(0, lineHeight + lineSpacing), this->stringFormat);
	pathTrivial.AddString(content.line3.c_str(), content.line3.length(), &this->fontFamily, FontStyleRegular, fontSize, Point(0, (lineHeight + lineSpacing) * 2), this->stringFormat);

	pathMerged.AddPath(&pathPrimary, false);
	pathMerged.AddPath(&pathTrivial, false);

	Rect bounds;
	pathMerged.GetBounds(&bounds);

	Matrix matrix;
	matrix.Translate(static_cast<REAL>(-bounds.X + windowPadding), static_cast<REAL>(-bounds.Y + windowPadding));
	pathMerged.Transform(&matrix);
	pathPrimary.Transform(&matrix);
	pathTrivial.Transform(&matrix);

	pathMerged.GetBounds(&bounds);

	const auto hdc = GetDC(this->hWnd);

	const auto hMemDC = CreateCompatibleDC(hdc);
	const auto hBitmap = CreateCompatibleBitmap(hdc, bounds.Width + windowPadding * 2, bounds.Height + windowPadding * 2);
	const auto hOldBitmap = static_cast<HBITMAP>(SelectObject(hMemDC, hBitmap));

	Graphics g(hMemDC);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
	g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

	g.DrawPath(this->penOutlinePrimary, &pathPrimary);
	g.FillPath(this->brushGlyphPrimary, &pathPrimary);

	g.DrawPath(this->penOutlineTrivial, &pathTrivial);
	g.FillPath(this->brushGlyphTrivial, &pathTrivial);

	BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
	POINT pointZero = { 0, 0 };
	SIZE size = { bounds.Width + windowPadding * 2, bounds.Height + windowPadding * 2 };
	UpdateLayeredWindow(this->hWnd, hdc, nullptr, &size, hMemDC, &pointZero, 0, &bf, ULW_ALPHA);

	SelectObject(hMemDC, hOldBitmap);
	DeleteDC(hMemDC);
	ReleaseDC(this->hWnd, hdc);
	DeleteObject(hBitmap);
}
