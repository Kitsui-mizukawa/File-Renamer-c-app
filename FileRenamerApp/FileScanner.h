#pragma once
// Header file untuk pemindaian file
#include <string>
#include <vector>
#include <windows.h>
// Representasi informasi file
struct FileInfo {
    std::wstring name;
    std::wstring fullPath;
    ULONGLONG size;
    FILETIME creationTime;
};
// Pindai folder dan kembalikan daftar file
std::vector<FileInfo> scanFolder(const std::wstring& folderPath);
