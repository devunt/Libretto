#include <iostream>
#include <map>
#include <memory>

#include "Melon.h"
#include "Util.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
	case WM_CHAR: //this is just for a program exit besides window's borders/task bar
		if (wparam == VK_ESCAPE)
		{
			DestroyWindow(hwnd);
		}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, message, wparam, lparam);
	}
	return 0;
}

int main2()
{
	WNDCLASSEX wcex;
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = &WndProc;
	wcex.hInstance = NULL;
	wcex.hIconSm = NULL;
	// wcex.hIcon = static_api_ptr_t<ui_control>()->get_main_icon();
	wcex.hCursor = LoadCursor(0, IDC_ARROW);
	wcex.hbrBackground = NULL;
	wcex.lpszClassName = TEXT("UILyricWindow");
	// wcex.cbWndExtra = sizeof(UIWnd*);
	RegisterClassEx(&wcex);

	// HINSTANCE hInstance = {0, };
	const auto m_hWnd = CreateWindowEx(
		WS_EX_TOPMOST |
		// WS_EX_TRANSPARENT |
		// WS_EX_LAYERED |
		WS_EX_TOOLWINDOW,
		TEXT("UILyricWindow"),
		TEXT("가사 창"),
		WS_POPUP | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT,
		500, 200,
		NULL,
		0,
		0,
		NULL);
	ShowWindow(m_hWnd, SW_NORMAL);
	MSG messages;
	while (GetMessage(&messages, NULL, 0, 0) > 0)
	{
		TranslateMessage(&messages);
		DispatchMessage(&messages);
	}
	DeleteObject(m_hWnd); //doing it just in case
	return messages.wParam;
	return 0;
}

int main()
{
	const auto pid = Util::getProcessIdByName(L"melon.exe");
	std::cout << "pid: " << pid << std::endl;

	const auto melon = std::make_unique<Melon>();
	melon->setPid(pid);

	LMetadata metadata;
	melon->getMetadata(metadata);
	printf("Magic: %s\n", metadata.magic);
	printf("BlockPointer: %X\n", metadata.blockPointer);
	printf("BlockCount: %d\n\n", metadata.blockCount);

	std::map<int, std::string> lyricMap;
	const auto blocks = melon->getBlocks(metadata);
	for (const auto &block : blocks)
	{
		char buf[512];
		const auto len = WideCharToMultiByte(CP_ACP, 0, reinterpret_cast<LPCWCH>(block.text), -1, nullptr, 0, nullptr, nullptr);
		WideCharToMultiByte(CP_ACP, 0, reinterpret_cast<LPCWCH>(block.text), -1, buf, len, nullptr, nullptr);
		lyricMap[block.timestamp] = std::string(buf, len);
	}

	const auto timestamp = melon->getTimestamp();
	printf("Timestamp: %d\n\n", timestamp);

	std::string previous, current, next;
	auto it = lyricMap.upper_bound(timestamp);
	if (it == lyricMap.begin())
	{
		// 아직 첫번째 가사 안 나옴
	}
	else
	{
		if (it != lyricMap.end())
			next = it->second;

		current = (--it)->second;

		if (it != lyricMap.begin())
			previous = (--it)->second;
	}

	std::cout << "Previous: " << previous << std::endl;
	std::cout << "Current: " << current << std::endl;
	std::cout << "Next: " << next << std::endl;

	return 0;
}
