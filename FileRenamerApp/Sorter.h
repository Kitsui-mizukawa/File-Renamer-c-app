#pragma once
#include "FileScanner.h"
#include <vector>
// Field untuk sorting
enum class SortField { Name, Size, Time };
enum class SortOrder { Ascending, Descending };
// Sortir file berdasarkan field dan order
void sortFiles(std::vector<FileInfo>& files, SortField field, SortOrder order);
