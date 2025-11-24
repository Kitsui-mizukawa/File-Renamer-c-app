#include "UI.h"
#include "FileScanner.h"
#include "Sorter.h"
#include "Pattern.h"
#include "Renamer.h"
#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <string>
#include <vector>
#include <filesystem>
#include <gdiplus.h>
#include <Shlwapi.h>
// Link dengan library yang diperlukan
#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Ole32.lib")

using namespace Gdiplus;

// ID kontrol
#define ID_BTN_SELECT    101
#define ID_BTN_RENAME    102
#define ID_EDIT_PATTERN  103
#define ID_LIST_PREVIEW  104
#define ID_COMBO_FIELD   201
#define ID_COMBO_ORDER   202
#define ID_EDIT_PADDING  301
#define IDB_BACKGROUND_IMAGE 1001

// Global kontrol
HWND hEditPattern, hListPreview, hComboField, hComboOrder, hEditPadding, hProgress, hBtnSelect, hBtnRename, hLabelPadding;
std::wstring g_folderPath;
std::vector<FileInfo> g_files;


// GDI+ token dan background image
ULONG_PTR gdiToken = 0;
Image* gBackgroundImage = nullptr;
// Set font untuk kontrol
void setFont(HWND hwnd) {
    HFONT hFont = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, TRUE);
}
// Ambil nilai padding dari input
int getPadding() {
    wchar_t buf[16]; GetWindowTextW(hEditPadding, buf, 16);
    int p = _wtoi(buf);
    return (p > 0 && p <= 10) ? p : 3;
}
// Update preview list
void updatePreview(HWND hwnd) {
    if (g_files.empty()) {
        SendMessageW(hListPreview, LB_RESETCONTENT, 0, 0);
        return;
    }
	//  Sorting
    int fieldIndex = (int)SendMessage(hComboField, CB_GETCURSEL, 0, 0);
    int orderIndex = (int)SendMessage(hComboOrder, CB_GETCURSEL, 0, 0);
	// Tentukan field dan order
    SortField field = (fieldIndex == 0) ? SortField::Name :
        (fieldIndex == 1) ? SortField::Size : SortField::Time;
    SortOrder order = (orderIndex == 0) ? SortOrder::Ascending : SortOrder::Descending;
	// Lakukan sorting
    sortFiles(g_files, field, order);
	// Ambil pattern
    wchar_t pbuf[256]; GetWindowTextW(hEditPattern, pbuf, 256);
    std::wstring pattern = pbuf;
	// Validasi pattern
    std::wstring err;
    if (!validatePattern(pattern, err)) {
        MessageBoxW(hwnd, err.c_str(), L"Pattern Error", MB_OK | MB_ICONERROR);
        return;
    }
	// Generate preview
    int padding = getPadding();
    SendMessageW(hListPreview, LB_RESETCONTENT, 0, 0);
	// Tambah entri preview
    for (int i = 0; i < (int)g_files.size(); ++i) {
        std::filesystem::path srcName = g_files[i].name;
        std::wstring ext = srcName.extension().wstring();
        std::wstring baseName = generateName(pattern, i, padding);
        std::wstring finalName = applyExtensionToName(baseName, ext);
        std::wstring preview = g_files[i].name + L" → " + finalName;
        SendMessageW(hListPreview, LB_ADDSTRING, 0, (LPARAM)preview.c_str());
    }
}
// Prosedur jendela utama
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        // Inisialisasi GDI+
        GdiplusStartupInput gdiplusStartupInput;
        if (GdiplusStartup(&gdiToken, &gdiplusStartupInput, NULL) != Ok) {
            MessageBoxW(hwnd, L"GDI+ gagal diinisialisasi.", L"Error", MB_OK | MB_ICONERROR);
            return 0;
        }

		// Muat gambar latar dari resource
        HMODULE hInst = GetModuleHandleW(NULL);
        HRSRC hRes = FindResourceW(hInst, MAKEINTRESOURCEW(IDB_BACKGROUND_IMAGE), RT_RCDATA);      
        HGLOBAL hData = LoadResource(hInst, hRes);
        void* pData = LockResource(hData);
        DWORD size = SizeofResource(hInst, hRes);       
        IStream* pStream = SHCreateMemStream(reinterpret_cast<const BYTE*>(pData), size);
        gBackgroundImage = Image::FromStream(pStream);
        pStream->Release();

        // Label dan edit pattern
        CreateWindowW(L"STATIC", L"Pattern:", WS_VISIBLE | WS_CHILD,
            20, 20, 60, 22, hwnd, NULL, NULL, NULL);
        hEditPattern = CreateWindowW(L"EDIT", L"file_{number}{ext}", WS_VISIBLE | WS_CHILD | WS_BORDER,
            90, 20, 220, 22, hwnd, (HMENU)ID_EDIT_PATTERN, NULL, NULL);

        // Tombol
        hBtnSelect = CreateWindowW(L"BUTTON", L"Pilih Folder", WS_VISIBLE | WS_CHILD,
            320, 20, 100, 25, hwnd, (HMENU)ID_BTN_SELECT, NULL, NULL);
        hBtnRename = CreateWindowW(L"BUTTON", L"Rename", WS_VISIBLE | WS_CHILD,
            430, 20, 80, 25, hwnd, (HMENU)ID_BTN_RENAME, NULL, NULL);

		// List preview
        hListPreview = CreateWindowW(L"LISTBOX", NULL,
            WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL,
            20, 60, 480, 280, hwnd, (HMENU)ID_LIST_PREVIEW, NULL, NULL);

        // Sorting
        hComboField = CreateWindowW(L"COMBOBOX", NULL,
            WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,
            20, 350, 160, 120, hwnd, (HMENU)ID_COMBO_FIELD, NULL, NULL);
        SendMessageW(hComboField, CB_ADDSTRING, 0, (LPARAM)L"Nama");
        SendMessageW(hComboField, CB_ADDSTRING, 0, (LPARAM)L"Ukuran");
        SendMessageW(hComboField, CB_ADDSTRING, 0, (LPARAM)L"Waktu");
        SendMessageW(hComboField, CB_SETCURSEL, 0, 0);
		// Order
        hComboOrder = CreateWindowW(L"COMBOBOX", NULL,
            WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,
            190, 350, 210, 120, hwnd, (HMENU)ID_COMBO_ORDER, NULL, NULL);
        SendMessageW(hComboOrder, CB_ADDSTRING, 0, (LPARAM)L"Naik (A-Z/Terkecil/Terlama)");
        SendMessageW(hComboOrder, CB_ADDSTRING, 0, (LPARAM)L"Turun (Z-A/Terbesar/Terbaru)");
        SendMessageW(hComboOrder, CB_SETCURSEL, 0, 0);

        // Padding sejajar
        hLabelPadding = CreateWindowW(L"STATIC", L"Padding:", WS_VISIBLE | WS_CHILD,
            410, 350, 60, 22, hwnd, NULL, NULL, NULL);
        hEditPadding = CreateWindowW(L"EDIT", L"3", WS_VISIBLE | WS_CHILD | WS_BORDER,
            480, 350, 40, 22, hwnd, (HMENU)ID_EDIT_PADDING, NULL, NULL);

        // Progress bar
        hProgress = CreateWindowExW(0, PROGRESS_CLASSW, NULL,
            WS_CHILD | WS_VISIBLE,
            20, 390, 490, 20,
            hwnd, NULL, GetModuleHandle(NULL), NULL);
        SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
        SendMessage(hProgress, PBM_SETPOS, 0, 0);

        // Font
        setFont(hEditPattern); setFont(hListPreview); setFont(hComboField);
        setFont(hComboOrder);  setFont(hEditPadding); setFont(hLabelPadding);
        return 0;
    }

    // Gambar latar
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        Graphics graphics(hdc);

        RECT rc;
        GetClientRect(hwnd, &rc);

        if (gBackgroundImage) {
            graphics.DrawImage(gBackgroundImage, 0, 0, rc.right, rc.bottom);
        }

        EndPaint(hwnd, &ps);
        return 0;
    }

	// Tata letak ulang kontrol saat jendela diubah ukuran
    case WM_SIZE: {
        int w = LOWORD(lParam), h = HIWORD(lParam);
        int margin = 20;
        int rowHeight = 25;

        // Baris atas
        MoveWindow(hEditPattern, margin + 70, 20, 220, rowHeight, TRUE);
        MoveWindow(hBtnSelect, margin + 300, 20, 100, rowHeight, TRUE);
        MoveWindow(hBtnRename, margin + 410, 20, 80, rowHeight, TRUE);

        // List preview
        MoveWindow(hListPreview, margin, 60, w - 2 * margin, h - 220, TRUE);

        // Baris bawah
        MoveWindow(hComboField, margin, h - 110, 160, rowHeight, TRUE);
        MoveWindow(hComboOrder, margin + 170, h - 110, 210, rowHeight, TRUE);
        MoveWindow(hLabelPadding, w - margin - 110, h - 110, 60, rowHeight, TRUE);
        MoveWindow(hEditPadding, w - margin - 50, h - 110, 40, rowHeight, TRUE);

        // Progress bar
        MoveWindow(hProgress, margin, h - 60, w - 2 * margin, rowHeight, TRUE);

        InvalidateRect(hwnd, NULL, FALSE); // redraw background
        return 0;
    }
	// Tangani perintah tombol
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case ID_BTN_SELECT: {
            wchar_t folderPath[MAX_PATH];
            BROWSEINFOW bi = { 0 };
            bi.lpszTitle = L"Pilih folder";
            LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
            if (pidl && SHGetPathFromIDListW(pidl, folderPath)) {
                g_folderPath = folderPath;
                g_files = scanFolder(g_folderPath);
                updatePreview(hwnd);
            }
            return 0;
        }
		// Tangani tombol rename
        case ID_BTN_RENAME: {
            if (g_folderPath.empty() || g_files.empty()) {
                MessageBoxW(hwnd, L"Pilih folder terlebih dahulu.", L"Info", MB_OK | MB_ICONINFORMATION);
                return 0;
			}
			// Ambil pattern
            wchar_t pbuf[256]; GetWindowTextW(hEditPattern, pbuf, 256);
            std::wstring pattern = pbuf;
            int padding = getPadding();
			// Validasi pattern
            std::vector<RenameStep> plan;
            bool conflict = false;
			// Cek konflik nama file
            for (int i = 0; i < (int)g_files.size(); ++i) {
                std::filesystem::path srcName = g_files[i].name;
                std::wstring ext = srcName.extension().wstring();
                std::wstring baseName = generateName(pattern, i, padding);
                std::wstring finalName = applyExtensionToName(baseName, ext);
                std::wstring newPath = g_folderPath + L"\\" + finalName;
				// Cek apakah file sudah ada
                DWORD attr = GetFileAttributesW(newPath.c_str());
                if (attr != INVALID_FILE_ATTRIBUTES) {
                    std::wstring msg = L"Konflik: file sudah ada → " + finalName;
                    MessageBoxW(hwnd, msg.c_str(), L"Conflict", MB_OK | MB_ICONERROR);
                    conflict = true;
                    break;
                }
                plan.push_back({ g_files[i].fullPath, newPath });
            }
			// Jika ada konflik, batalkan proses
            if (conflict) return 0;
			// Lakukan rename dengan progress bar
            int total = (int)plan.size();
            for (int i = 0; i < total; ++i) {
                renameFile(plan[i].oldPath, plan[i].newPath);
                int progress = (int)(((i + 1) * 100) / total);
                SendMessage(hProgress, PBM_SETPOS, progress, 0);
            }
			// Reset progress bar
            g_files = scanFolder(g_folderPath);
            updatePreview(hwnd);
            MessageBoxW(hwnd, L"Proses rename selesai.", L"Info", MB_OK | MB_ICONINFORMATION);
            return 0;
        }
        }
        break;
    }
	//  Tangani penghancuran window
    case WM_DESTROY:
        if (gBackgroundImage) { delete gBackgroundImage; gBackgroundImage = nullptr; }
        if (gdiToken) { GdiplusShutdown(gdiToken); gdiToken = 0; }
        PostQuitMessage(0);
        return 0;
    }
	// Default window procedure
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}
// Jalankan aplikasi
int RunApp(HINSTANCE hInstance, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"FileRenamerWindow";
	// Daftarkan kelas window
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    RegisterClassW(&wc);
	// Buat window utama
    HWND hwnd = CreateWindowExW(0, CLASS_NAME, L"File Renamer App",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        960, 540, NULL, NULL, hInstance, NULL);
	// Cek pembuatan window
    if (!hwnd) return 0;
	// Tampilkan window
    ShowWindow(hwnd, nCmdShow);
	// Loop pesan
    MSG msg = {};
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}
