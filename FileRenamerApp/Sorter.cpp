#include "Sorter.h"
#include <algorithm>
#include <windows.h>
// Sortir file berdasarkan field dan order
void sortFiles(std::vector<FileInfo>& files, SortField field, SortOrder order) {
    auto cmp = [&](const FileInfo& a, const FileInfo& b) {
		// Bandingkan berdasarkan field yang dipilih
        switch (field) {
        case SortField::Name:
            return (order == SortOrder::Ascending) ? (a.name < b.name) : (a.name > b.name);
        case SortField::Size:
            return (order == SortOrder::Ascending) ? (a.size < b.size) : (a.size > b.size);
        case SortField::Time:
            return (order == SortOrder::Ascending)
                ? (CompareFileTime(&a.creationTime, &b.creationTime) < 0)
                : (CompareFileTime(&a.creationTime, &b.creationTime) > 0);
        }
        return true;
        };
	// Lakukan sorting
    std::sort(files.begin(), files.end(), cmp);
}
