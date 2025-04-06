#include <windows.h>
#include <tchar.h>
#include <shellapi.h>
#include <commctrl.h>
#include <wingdi.h>
#include <filesystem>
#include <commdlg.h>
#include <cstdlib>
#include <iostream>
#include <string>

#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "comctl32.lib")

// Идентификаторы элементов
#define IDC_EDIT_PATH            101
#define IDC_BTN_BROWSE           102
#define IDC_BTN_COMPILE_EXE      103
#define IDC_BTN_COMPILE_APK      104
#define IDC_BTN_COMPILE_LINUX    105
#define IDC_STATUSBAR            106
#define IDC_LBL_TITLE            107

// Глобальные переменные
HINSTANCE g_hInst = nullptr;
const COLORREF PINK_TOP = RGB(255, 182, 193);
const COLORREF PINK_BOTTOM = RGB(255, 105, 180);

// Прототипы функций
void CompileEXE(HWND hwnd, const std::wstring& path);
void ShowError(HWND hwnd, const wchar_t* message);
void UpdateStatus(HWND hStatus, const wchar_t* text);
std::wstring GetPythonPath();

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hEdit, hBtnExe, hBtnApk, hBtnLinux, hStatus;
    static HFONT hFontMain, hFontTitle;

    switch (msg) {
    case WM_CREATE: {
        // Инициализация шрифтов
        hFontMain = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("Segoe UI"));

        hFontTitle = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("Segoe UI"));

        // Создание элементов GUI
        CreateWindow(_T("STATIC"), _T("🐍 Python Compiler"),
            WS_VISIBLE | WS_CHILD, 20, 15, 400, 40, hwnd,
            (HMENU)IDC_LBL_TITLE, g_hInst, NULL);

        hEdit = CreateWindow(_T("EDIT"), _T(""),
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            20, 70, 450, 30, hwnd, (HMENU)IDC_EDIT_PATH, g_hInst, NULL);

        CreateWindow(_T("BUTTON"), _T("📂 Выбрать файл"),
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            480, 70, 120, 30, hwnd, (HMENU)IDC_BTN_BROWSE, g_hInst, NULL);

        hBtnExe = CreateWindow(_T("BUTTON"), _T("💻 Собрать EXE"),
            WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            20, 120, 150, 40, hwnd, (HMENU)IDC_BTN_COMPILE_EXE, g_hInst, NULL);

        hBtnApk = CreateWindow(_T("BUTTON"), _T("📱 Собрать APK (PRO)"),
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            180, 120, 170, 40, hwnd, (HMENU)IDC_BTN_COMPILE_APK, g_hInst, NULL);

        hBtnLinux = CreateWindow(_T("BUTTON"), _T("🐧 Собрать Linux (PRO) "),
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            360, 120, 170, 40, hwnd, (HMENU)IDC_BTN_COMPILE_LINUX, g_hInst, NULL);

        hStatus = CreateWindow(_T("STATIC"), _T("✅ Готов к работе. сборка apk и под Linux-системы возможна только на linux."),
            WS_CHILD | WS_VISIBLE, 20, 180, 560, 30, hwnd,
            (HMENU)IDC_STATUSBAR, g_hInst, NULL);

        // Применение шрифтов
        for (HWND h : {hEdit, hBtnExe, hBtnApk, hBtnLinux, hStatus}) {
            SendMessage(h, WM_SETFONT, (WPARAM)hFontMain, TRUE);
        }
        SendDlgItemMessage(hwnd, IDC_LBL_TITLE, WM_SETFONT, (WPARAM)hFontTitle, TRUE);
        break;
    }

    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case IDC_BTN_BROWSE: {
            OPENFILENAME ofn = { 0 };
            wchar_t szFile[MAX_PATH] = { 0 };

            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFilter = _T("Python Files (*.py)\0*.py\0All Files (*.*)\0*.*\0");
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

            if (GetOpenFileName(&ofn)) {
                SetWindowText(hEdit, szFile);
            }
            break;
        }

        case IDC_BTN_COMPILE_EXE: {
            wchar_t path[MAX_PATH];
            GetWindowText(hEdit, path, MAX_PATH);

            if (wcslen(path) == 0) {
                ShowError(hwnd, _T("Выберите файл для компиляции!"));
                break;
            }

            UpdateStatus(hStatus, _T("🔄 Компиляция началась..."));
            CompileEXE(hwnd, path);
            break;
        }
        }
        break;
    }

    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORBTN: {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, RGB(255, 255, 255));
        SetBkMode(hdc, TRANSPARENT);
        return (LRESULT)GetStockObject(NULL_BRUSH);
    }

    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wParam;
        RECT rc;
        GetClientRect(hwnd, &rc);

        TRIVERTEX vertices[] = {
            {rc.left, rc.top,
             PINK_TOP, PINK_TOP, PINK_TOP, 0x0000},
            {rc.right, rc.bottom,
             GetRValue(PINK_BOTTOM) << 8,
             GetGValue(PINK_BOTTOM) << 8,
             GetBValue(PINK_BOTTOM) << 8, 0x0000}
        };

        GRADIENT_RECT gRect = { 0, 1 };
        GradientFill(hdc, vertices, 2, &gRect, 1, GRADIENT_FILL_RECT_V);
        return TRUE;
    }

    case WM_DESTROY: {
        PostQuitMessage(0);
        break;
    }

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// Функция компиляции в EXE
void CompileEXE(HWND hwnd, const std::wstring& path) {
    std::wstring cmd = L"pyinstaller --onefile --noconsole \"" + path + L"\"";

    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    if (CreateProcess(NULL, (LPWSTR)cmd.c_str(), NULL, NULL, FALSE,
        0, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);

        if (exitCode == 0) {
            UpdateStatus(GetDlgItem(hwnd, IDC_STATUSBAR),
                _T("✅ Компиляция успешно завершена!"));
        }
        else {
            ShowError(hwnd, _T("Ошибка компиляции! Проверьте зависимости."));
        }

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else {
        ShowError(hwnd, _T("Не удалось запустить PyInstaller!"));
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow) {
    g_hInst = hInstance;

    WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = _T("PythonCompilerClass");

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, _T("Ошибка регистрации окна!"), _T("Ошибка"), MB_ICONERROR);
        return 1;
    }

    HWND hwnd = CreateWindowEx(
        WS_EX_COMPOSITED,
        _T("PythonCompilerClass"),
        _T("Python Compiler Pro 1.18"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        640, 300,
        NULL, NULL, hInstance, NULL);

    if (!hwnd) {
        MessageBox(NULL, _T("Ошибка создания окна!"), _T("Ошибка"), MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

// Вспомогательные функции
void ShowError(HWND hwnd, const wchar_t* message) {
    MessageBox(hwnd, message, _T("Ошибка"), MB_ICONERROR);
}

void UpdateStatus(HWND hStatus, const wchar_t* text) {
    SetWindowText(hStatus, text);
}
