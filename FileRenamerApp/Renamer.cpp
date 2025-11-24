#include "Renamer.h"
#include <windows.h>

// Lakukan rename file
bool renameFile(const std::wstring& oldPath, const std::wstring& newPath) {
    return MoveFileExW(oldPath.c_str(), newPath.c_str(),
        MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING) != 0;
}
// Lakukan rename sesuai rencana
bool performRenamePlan(const std::vector<RenameStep>& plan) {
    bool ok = true;
	// Iterasi langkah rename
    for (const auto& step : plan) {
        if (!renameFile(step.oldPath, step.newPath)) ok = false;
    }
    return ok;
}
