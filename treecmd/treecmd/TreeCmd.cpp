#include <windows.h>
#include <stdio.h>
#include <string>
#include <tchar.h>

class TreeCmd {
	wchar_t * directory;
	wchar_t *command;
	wchar_t * program;
public:
	TreeCmd(wchar_t*p) {
		program = p;
		directory = nullptr;
		command = nullptr;
	}
	void printHelp() {
		if (program == 0)
			program = L"treecmd.exe";
		wprintf(L"Version 1.0");
		wprintf(L"\n %s\n treecmd <directory> <command>\n enclose command in apostrophes to include spaces",program);
	}
	void process(wchar_t*argv) {
		
		if (!directory) {
			if (argv && ( !_wcsicmp(argv,L"-help") || !_wcsicmp(argv, L"-h") || 
						!_wcsicmp(argv, L"/help") || !_wcsicmp(argv, L"/help"))) {
				printHelp();
			}
			else
				directory = argv;
		}
		else if (!command) {
			command = argv;
		}
	}
	int postProcess() {
		if (directory)
			walktree(directory, command);
		else
			printHelp();
		return 0;
	}
	std::wstring getCurrent() {
		std::wstring buffer;
		buffer.reserve(4096);
		if (GetCurrentDirectoryW(buffer.capacity(), (LPWSTR)buffer.c_str())) {
			return std::wstring(buffer.c_str());
		}
		return L"";
	}

	int walktree(const wchar_t * directory,const wchar_t * command) {
		std::wstring dir = directory;
		dir += L"\\*";
		std::wstring cmd = command ? command : L"";
		WIN32_FIND_DATAW findData;
		
		HANDLE hFile = FindFirstFile(dir.c_str(), &findData);
		if  (hFile != INVALID_HANDLE_VALUE) {
			std::wstring current = getCurrent();
			SetCurrentDirectoryW(directory);

			do {
				std::wstring fname = findData.cFileName;
				if (fname == L".." || fname == L".")
					continue;
				if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					std::wstring walk = fname;
					walktree(walk.c_str(), command);
				}
				else {
					// TODO implement replacement strategy %N filename without extension %E extension %F complete file name %P path
					if (cmd.empty()) {
						wprintf(L"%s\n", fname.c_str());
					}
					else {
						std::wstring com = cmd + L" " + fname;
						wchar_t*buffer = new wchar_t[com.length() + 1];
						wsprintf(buffer, L"%s", com.c_str());
						_wsystem(buffer);
					}
				}
			} while (FindNextFileW(hFile, &findData));
			FindClose(hFile);
			SetCurrentDirectoryW(current.c_str());
		}
		
		return 0;
	}
};

int wmain(int argc,wchar_t **argv) {
	TreeCmd cmd(*argv);
	while (--argc) {
		cmd.process(*++argv);
	}
	return cmd.postProcess();
}