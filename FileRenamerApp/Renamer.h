#pragma once
#include <string>
#include <vector>
// Representasi langkah rename
struct RenameStep {
    std::wstring oldPath;
    std::wstring newPath;
};
// Lakukan rename file
bool renameFile(const std::wstring& oldPath, const std::wstring& newPath);
bool performRenamePlan(const std::vector<RenameStep>& plan);
