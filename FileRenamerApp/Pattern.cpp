#include "Pattern.h"
#include <cwctype>
// hitung kemunculan substring
static size_t countOccurrences(const std::wstring& s, const std::wstring& p) {
    size_t c = 0, pos = s.find(p);
    while (pos != std::wstring::npos) { ++c; pos = s.find(p, pos + p.size()); }
    return c;
}
// validasi pattern
bool validatePattern(const std::wstring& pattern, std::wstring& errorMessage) {
	// harus ada {number} sekali
    if (countOccurrences(pattern, L"{number}") != 1) {
        errorMessage = L"Pattern harus mengandung tepat satu {number}.";
        return false;
    }
	// boleh ada {ext} nol atau sekali
    if (pattern.size() > 200) {
        errorMessage = L"Pattern terlalu panjang (maksimal 200 karakter).";
        return false;
    }

    if (countOccurrences(pattern, L"{ext}") > 1) {
        errorMessage = L"Pattern hanya boleh mengandung satu {ext}.";
        return false;
    }

	// cek karakter terlarang di Windows
    const wchar_t* invalid = L"<>:\"/\\|?*";
    for (wchar_t ch : pattern) {
        if (wcschr(invalid, ch)) {
            errorMessage = L"Pattern mengandung karakter terlarang.";
            return false;
        }
    }
    
	// cek reserved names di Windows
    std::wstring base = pattern;
    size_t posNum = base.find(L"{number}");
    if (posNum != std::wstring::npos) base.erase(posNum, 8);
    size_t posExt = base.find(L"{ext}");
    while (posExt != std::wstring::npos) {
        base.erase(posExt, 5);
        posExt = base.find(L"{ext}");
    }
	// trim whitespace
	const wchar_t* reserved[] = {// daftar nama reserved
        L"CON",L"PRN",L"AUX",L"NUL",
        L"COM1",L"COM2",L"COM3",L"COM4",L"COM5",L"COM6",L"COM7",L"COM8",L"COM9",
        L"LPT1",L"LPT2",L"LPT3",L"LPT4",L"LPT5",L"LPT6",L"LPT7",L"LPT8",L"LPT9"
    };
	//  bandingkan tanpa memperhatikan besar kecil huruf
    for (const auto& r : reserved) {
        if (_wcsicmp(base.c_str(), r) == 0) {
            errorMessage = L"Pattern menggunakan nama reserved di Windows.";
            return false;
        }
    }
    return true;
}
// generate nama file
std::wstring generateName(const std::wstring& pattern, int index, int padding) {
    wchar_t numBuf[32];
    swprintf(numBuf, 32, L"%0*d", padding, index + 1);
    std::wstring result = pattern;
    size_t pos = result.find(L"{number}");
    if (pos != std::wstring::npos) result.replace(pos, 8, numBuf);
    return result; 
}
// terapkan ekstensi
std::wstring applyExtensionToName(const std::wstring& baseName, const std::wstring& originalExt) {// cek placeholder {ext}
    std::wstring name = baseName;
	size_t ph = name.find(L"{ext}");// ganti dengan ekstensi asli
    if (ph != std::wstring::npos) {
		name.replace(ph, 5, originalExt);// kembalikan nama dengan ekstensi
        return name;
    }
	// Cek apakah sudah ada ekstensi
    size_t dot = name.find_last_of(L'.');
    if (dot != std::wstring::npos && dot > 0) return name;
    return name + originalExt;
}
