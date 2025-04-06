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

#pragma comment(lib, "msimg32.lib")    // Для GradientFill
#pragma comment(lib, "comctl32.lib")     // Для Common Controls (ToolTip)

#define IDC_EDIT_PATH            102
#define IDC_BUTTON_COMPILE       103
#define IDC_BUTTON_BROWSE        104
#define IDC_BUTTON_DOWNLOAD      105
#define IDC_STATIC_DOWNLOADINFO  106
#define IDC_STATUSBAR            107

// Глобальный экземпляр приложения (используется для создания ToolTip)
HINSTANCE g_hInstance = nullptr;

// Объявляем имя класса как изменяемую строку
TCHAR g_szClassName[] = _T("MyWinAPIApp");

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HWND hStaticIcon, hEdit, hButtonBrowse, hButtonCompile, hButtonDownload;
    static HWND hStaticDownload, hStaticInfo, hStatusBar;
    static HFONT hFont; // Шрифт для элементов
    static HWND hToolTip; // Окно ToolTip

    switch (msg)
    {
    case WM_CREATE:
    {
        // Создаем шрифт "Segoe UI", 20-пунктовый
        hFont = CreateFont(
            20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("Segoe UI")
        );

        // Устанавливаем иконку для заголовка окна
        HICON hIcon = (HICON)LoadImage(NULL, _T("C:\\Program Files (x86)\\MyApp\\comp.ico"), IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
        if (hIcon)
            SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

        // Статический элемент для иконки в клиентской области (левый верхний угол)
        HICON hSmallIcon = (HICON)LoadImage(NULL, _T("C:\\Program Files (x86)\\MyApp\\comp.ico"), IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
        hStaticIcon = CreateWindow(_T("STATIC"), nullptr, WS_VISIBLE | WS_CHILD | SS_ICON,
            10, 10, 32, 32, hwnd, nullptr, g_hInstance, nullptr);
        if (hStaticIcon)
        {
            SendMessage(hStaticIcon, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hSmallIcon);
            SendMessage(hStaticIcon, WM_SETFONT, (WPARAM)hFont, TRUE);
        }

        // Метка "Укажи путь к Python скрипту:"
        HWND hLabel = CreateWindow(_T("STATIC"), _T("Укажи путь к Python скрипту:"),
            WS_VISIBLE | WS_CHILD, 50, 10, 400, 25, hwnd, nullptr, g_hInstance, nullptr);
        SendMessage(hLabel, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Поле ввода пути
        hEdit = CreateWindow(_T("EDIT"), _T(""),
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 50, 40, 500, 30, hwnd, (HMENU)IDC_EDIT_PATH, g_hInstance, nullptr);
        SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Кнопка "Выбрать"
        hButtonBrowse = CreateWindow(_T("BUTTON"), _T("Выбрать"),
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 560, 40, 120, 30, hwnd, (HMENU)IDC_BUTTON_BROWSE, g_hInstance, nullptr);
        SendMessage(hButtonBrowse, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Кнопка "Скомпилировать"
        hButtonCompile = CreateWindow(_T("BUTTON"), _T("Скомпилировать"),
            WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 50, 80, 200, 40, hwnd, (HMENU)IDC_BUTTON_COMPILE, g_hInstance, nullptr);
        SendMessage(hButtonCompile, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Кнопка "Скачать Python"
        hButtonDownload = CreateWindow(_T("BUTTON"), _T("Скачать Python"),
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 50, 130, 200, 40, hwnd, (HMENU)IDC_BUTTON_DOWNLOAD, g_hInstance, nullptr);
        SendMessage(hButtonDownload, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Статический текст под кнопкой "Скачать Python"
        hStaticDownload = CreateWindow(_T("STATIC"), _T("обязательно"),
            WS_VISIBLE | WS_CHILD | SS_CENTER, 50, 180, 200, 25, hwnd, (HMENU)IDC_STATIC_DOWNLOADINFO, g_hInstance, nullptr);
        SendMessage(hStaticDownload, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Правая панель с инструкциями
        hStaticInfo = CreateWindow(_T("STATIC"),
            _T("Как работает компилятор?\n\n1. Введи путь к Python-скрипту\n2. Нажми 'Выбрать', чтобы найти файл\n3. Нажми 'Скомпилировать'\n\nСкрипт будет преобразован в EXE."),
            WS_VISIBLE | WS_CHILD | SS_LEFT, 710, 10, 280, 250, hwnd, nullptr, g_hInstance, nullptr);
        SendMessage(hStaticInfo, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Статус-бар внизу окна
        hStatusBar = CreateWindow(_T("STATIC"), _T("Готов к работе"),
            WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 550, 980, 30, hwnd, (HMENU)IDC_STATUSBAR, g_hInstance, nullptr);
        SendMessage(hStatusBar, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Создаем окно ToolTip
        hToolTip = CreateWindowEx(0, TOOLTIPS_CLASS, NULL,
            WS_POPUP | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            hwnd, NULL, g_hInstance, NULL);

        // Добавляем ToolTip для кнопки "Выбрать"
        TOOLINFO ti = { sizeof(TOOLINFO) };
        ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
        ti.hwnd = hwnd;
        ti.hinst = g_hInstance;
        ti.uId = (UINT_PTR)hButtonCompile;
        ti.lpszText = const_cast<LPWSTR>(_T("Скомпилировать скрипт в EXE"));
        SendMessage(hToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);

        ti.uId = (UINT_PTR)hButtonDownload;
        ti.lpszText = const_cast<LPWSTR>(_T("Перейти на страницу скачивания Python"));
        SendMessage(hToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);

    }
    break;

    case WM_SIZE:
    {
        // Перемещаем статус-бар при изменении размеров окна
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        MoveWindow(hStatusBar, 10, height - 40, width - 20, 30, TRUE);
    }
    break;

    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDC_BUTTON_BROWSE:
        {
            OPENFILENAME ofn;
            TCHAR szFile[MAX_PATH] = { 0 };
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFilter = _T("Python Files\0*.py\0All Files\0*.*\0");
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
            ofn.lpstrDefExt = _T("py");

            if (GetOpenFileName(&ofn))
                SetWindowText(hEdit, szFile);
        }
        break;

        case IDC_BUTTON_COMPILE:
        {
            TCHAR pathBuffer[512] = { 0 };
            GetWindowText(hEdit, pathBuffer, 512);

            if (_tcslen(pathBuffer) == 0)
            {
                MessageBox(hwnd, _T("Выберите или укажите путь к скрипту!"), _T("Ошибка"), MB_ICONERROR);
                break;
            }

            // Проверка наличия PyInstaller
            std::string checkPyInstallerCommand = "pip show pyinstaller";
            int ret = system(checkPyInstallerCommand.c_str());
            if (ret != 0)
            {
                MessageBox(hwnd, _T("PyInstaller не найден. Устанавливаю..."), _T("Установка PyInstaller"), MB_ICONINFORMATION);
                std::string installCommand = "pip install pyinstaller";
                ret = system(installCommand.c_str());
                if (ret != 0)
                {
                    MessageBox(hwnd, _T("Ошибка при установке PyInstaller!"), _T("Ошибка"), MB_ICONERROR);
                    break;
                }
            }

            std::filesystem::path scriptPath(pathBuffer);
            std::string scriptDir = scriptPath.parent_path().string();

            // Формирование команды компиляции
            std::wstring command = L"python -m PyInstaller --onefile --distpath \"";
            command += std::wstring(scriptDir.begin(), scriptDir.end());
            command += L"\" \"";
            command += std::wstring(pathBuffer);
            command += L"\"";

            std::string cmd(command.begin(), command.end());
            ret = system(cmd.c_str());
            if (ret != 0)
                MessageBox(hwnd, _T("Ошибка при компиляции!"), _T("Ошибка"), MB_ICONERROR);
            else
                MessageBox(hwnd, _T("Компиляция завершена успешно!"), _T("Успех"), MB_ICONINFORMATION);
        }
        break;

        case IDC_BUTTON_DOWNLOAD:
        {
            ShellExecute(NULL, _T("open"), _T("https://www.python.org/downloads/"), NULL, NULL, SW_SHOWNORMAL);
        }
        break;
        }
    }
    break;

    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORBTN:
    {
        HDC hdcStatic = (HDC)wParam;
        SetTextColor(hdcStatic, RGB(255, 255, 255));
        SetBkMode(hdcStatic, TRANSPARENT);
        return (LRESULT)GetStockObject(NULL_BRUSH);
    }

    case WM_ERASEBKGND:
    {
        HDC hdc = (HDC)wParam;
        RECT rc;
        GetClientRect(hwnd, &rc);

        TRIVERTEX vertices[2];
        vertices[0].x = rc.left;
        vertices[0].y = rc.top;
        vertices[0].Red = 30 << 8;
        vertices[0].Green = 30 << 8;
        vertices[0].Blue = 60 << 8;
        vertices[0].Alpha = 0x0000;

        vertices[1].x = rc.right;
        vertices[1].y = rc.bottom;
        vertices[1].Red = 50 << 8;
        vertices[1].Green = 50 << 8;
        vertices[1].Blue = 50 << 8;
        vertices[1].Alpha = 0x0000;

        GRADIENT_RECT gRect = { 0, 1 };
        GradientFill(hdc, vertices, 2, &gRect, 1, GRADIENT_FILL_RECT_V);
        return 1;
    }

    case WM_DESTROY:
        DeleteObject(hFont);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    g_hInstance = hInstance;

    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = (HICON)LoadImage(NULL, _T("C:\\Program Files (x86)\\MyApp\\comp.ico"), IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszClassName = g_szClassName; // Теперь g_szClassName не является константным

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, _T("Ошибка регистрации окна!"), _T("Ошибка"), MB_ICONEXCLAMATION);
        return 0;
    }

    // Задаем расширенный стиль через CreateWindowEx (WS_EX_COMPOSITED)
    HWND hwnd = CreateWindowEx(WS_EX_COMPOSITED, g_szClassName, _T("Компилятор Python"),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1000, 350,
        NULL, NULL, hInstance, NULL);

    if (!hwnd)
    {
        MessageBox(NULL, _T("Ошибка создания окна!"), _T("Ошибка"), MB_ICONEXCLAMATION);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}
