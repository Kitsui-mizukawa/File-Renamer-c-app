#pragma once
// Header file untuk pola penamaan file
#include <string>
// Generate file name based on pattern
std::wstring generateName(const std::wstring& pattern, int index, int padding);
// Validate renaming pattern
bool validatePattern(const std::wstring& pattern, std::wstring& errorMessage);
//	Apply original file extension to the generated name
std::wstring applyExtensionToName(const std::wstring& baseName, const std::wstring& originalExt);
