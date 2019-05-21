#include <iostream>
#include <map>
#include <memory>

#include "MainWindow.h"
#include "Melon.h"
#include "Util.h"

int APIENTRY WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd )
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MainWindow mainWindow;
	mainWindow.initialize(hInstance);
	mainWindow.startBackgroundTasks();

	return mainWindow.runMessageLoop();
}

int main()
{
	const auto pid = Util::getProcessIdByName(L"melon.exe");
	std::cout << "pid: " << pid << std::endl;

	const auto melon = std::make_unique<Melon>();
	melon->setPid(pid);

	const auto metadata = melon->getMetadata();
	printf("Magic: %s\n", metadata->magic);
	printf("BlockPointer: %X\n", metadata->blockPointer);
	printf("BlockCount: %d\n\n", metadata->blockCount);

	std::map<int, std::string> lyricMap;
	const auto blocks = melon->getBlocks(*metadata);
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
