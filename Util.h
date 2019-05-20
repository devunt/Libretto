#pragma once

#include <Windows.h>
#include <TlHelp32.h>

class Util
{
public:
	static DWORD getProcessIdByName(const LPCTSTR name) {
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof(PROCESSENTRY32);

		const auto hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (Process32First(hSnapshot, &pe32))
		{
			do
			{
				if (!lstrcmpi(pe32.szExeFile, name))
				{
					CloseHandle(hSnapshot);
					return pe32.th32ProcessID;
				}
			} while (Process32Next(hSnapshot, &pe32));
		}

		CloseHandle(hSnapshot);
		return 0;
	}

	static HMODULE getModuleHandleByName(const DWORD pid, const LPCTSTR name) {
		MODULEENTRY32 me32;
		me32.dwSize = sizeof(MODULEENTRY32);

		const auto hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
		if (Module32First(hSnapshot, &me32))
		{
			do
			{
				if (!lstrcmpi(me32.szModule, name))
				{
					CloseHandle(hSnapshot);
					return me32.hModule;
				}
			} while (Module32Next(hSnapshot, &me32));
		}

		CloseHandle(hSnapshot);
		return nullptr;
	}
};
