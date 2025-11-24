#include "FileScanner.h"

std::vector<FileInfo> scanFolder(const std::wstring& folderPath) {
    std::vector<FileInfo> files;
    std::wstring searchPath = folderPath + L"\\*";
    WIN32_FIND_DATAW findData;


	// Mulai pencarian file
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) return files;
	// Iterasi file dalam folder
    do {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

		// Kumpulkan info file
        FileInfo info;
        info.name = findData.cFileName;
        info.fullPath = folderPath + L"\\" + findData.cFileName;
        info.size = (static_cast<ULONGLONG>(findData.nFileSizeHigh) << 32) | findData.nFileSizeLow;
        info.creationTime = findData.ftCreationTime;
        files.push_back(std::move(info));
        
    } while (FindNextFileW(hFind, &findData));
	// Akhiri pencarian
    FindClose(hFind);
    return files;
}