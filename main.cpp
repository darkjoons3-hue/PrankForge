#pragma execution_character_set("utf-8")
#define _WIN32_WINNT 0x0601
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <string>
#include <vector>
#include <cstdio>
#include <cwchar>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(linker, "/ENTRY:wWinMainCRTStartup")

// ============================================================
// ЦВЕТА (тёмная тема)
// ============================================================
namespace Colors {
    const COLORREF BG        = RGB(30, 30, 30);
    const COLORREF BG_DARK   = RGB(20, 20, 20);
    const COLORREF BG_PANEL  = RGB(45, 45, 45);
    const COLORREF BG_HOVER  = RGB(60, 60, 60);
    const COLORREF ACCENT    = RGB(255, 107, 53);
    const COLORREF ACCENT_H  = RGB(255, 130, 80);
    const COLORREF TEXT      = RGB(224, 224, 224);
    const COLORREF TEXT_DIM  = RGB(150, 150, 150);
    const COLORREF BORDER    = RGB(70, 70, 70);
}

// ============================================================
// ГЛОБАЛЫ
// ============================================================
HINSTANCE g_hInst = nullptr;

const wchar_t* CLASS_WELCOME = L"PrankForgeWelcome";
const wchar_t* CLASS_MAIN    = L"PrankForgeMain";
const wchar_t* CLASS_CREATE  = L"PrankForgeCreate";

// Язык (0 = RU, 1 = EN)
int g_language = 0;

// Данные проекта (для теста)
struct ProjectInfo {
    std::wstring name;
    std::wstring path;
    std::wstring date;
};
std::vector<ProjectInfo> g_projects;

// Welcome
HWND g_welcomeBtnStart = nullptr;
HWND g_welcomeBtnRu = nullptr;
HWND g_welcomeBtnEn = nullptr;
HWND g_welcomeBtnDoc = nullptr;
HWND g_welcomeBtnAbout = nullptr;
HWND g_welcomeBtnExit = nullptr;

// Main
HWND g_mainBtnCreate = nullptr;
HWND g_mainBtnSettings = nullptr;
HWND g_mainBtnRefresh = nullptr;

// Create
HWND g_createEditName = nullptr;
HWND g_createEditPath = nullptr;
HWND g_createBtnBrowse = nullptr;
HWND g_createBtnOk = nullptr;
HWND g_createBtnCancel = nullptr;

// Текущее окно main
HWND g_mainWnd = nullptr;

// ============================================================
// УТИЛИТЫ
// ============================================================
void FillRectC(HDC dc, const RECT& rc, COLORREF c) {
    HBRUSH b = CreateSolidBrush(c);
    FillRect(dc, &rc, b);
    DeleteObject(b);
}

void DrawTextCentered(HDC dc, const wchar_t* text, const RECT& rc,
                     COLORREF color, int fontSize, bool bold) {
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, color);
    HFONT f = CreateFontW(fontSize, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL,
        0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    HFONT old = (HFONT)SelectObject(dc, f);
    DrawTextW(dc, text, -1, (LPRECT)&rc,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(dc, old);
    DeleteObject(f);
}

void DrawTextLeft(HDC dc, const wchar_t* text, const RECT& rc,
                 COLORREF color, int fontSize, bool bold) {
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, color);
    HFONT f = CreateFontW(fontSize, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL,
        0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    HFONT old = (HFONT)SelectObject(dc, f);
    DrawTextW(dc, text, -1, (LPRECT)&rc,
        DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    SelectObject(dc, old);
    DeleteObject(f);
}

// Простая кастомная кнопка — рисуется в WM_PAINT родителя
void DrawCustomButton(HDC dc, const RECT& rc, const wchar_t* text,
                     bool hovered, bool accent) {
    COLORREF bg = accent
        ? (hovered ? Colors::ACCENT_H : Colors::ACCENT)
        : (hovered ? Colors::BG_HOVER : Colors::BG_PANEL);
    HBRUSH b = CreateSolidBrush(bg);
    FillRect(dc, &rc, b);
    DeleteObject(b);

    // Рамка
    HBRUSH border = CreateSolidBrush(Colors::BORDER);
    FrameRect(dc, &rc, border);
    DeleteObject(border);

    // Текст
    DrawTextCentered(dc, text, rc,
        accent ? RGB(255, 255, 255) : Colors::TEXT, 15, true);
}

// ============================================================
// WELCOME WINDOW
// ============================================================
LRESULT CALLBACK WelcomeProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    static int hoverStart = 0, hoverRu = 0, hoverEn = 0;
    static int hoverDoc = 0, hoverAbout = 0, hoverExit = 0;

    switch (m) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(h, &ps);
        RECT rc; GetClientRect(h, &rc);

        // Фон
        FillRectC(dc, rc, Colors::BG);

        // Верхняя полоса
        RECT topBar = { 0, 0, rc.right, 40 };
        FillRectC(dc, topBar, Colors::BG_DARK);

        // Кнопка закрытия (справа сверху)
        RECT btnClose = { rc.right - 40, 0, rc.right, 40 };
        if (hoverExit) FillRectC(dc, btnClose, RGB(200, 50, 50));
        DrawTextCentered(dc, L"✕", btnClose, Colors::TEXT, 16, false);

        // Логотип
        RECT logoRc = { 0, 100, rc.right, 180 };
        DrawTextCentered(dc, L"⚒ PrankForge", logoRc,
            Colors::ACCENT, 56, true);

        // Слоган
        RECT slogRc = { 0, 180, rc.right, 220 };
        DrawTextCentered(dc, L"Build your chaos.",
            slogRc, Colors::TEXT_DIM, 18, false);

        // Приветствие
        RECT wellRc = { 0, 260, rc.right, 300 };
        const wchar_t* welcome = (g_language == 0)
            ? L"Добро пожаловать в конструктор пранков"
            : L"Welcome to the prank builder";
        DrawTextCentered(dc, welcome, wellRc, Colors::TEXT, 22, false);

        // Описание
        RECT descRc = { 60, 310, rc.right - 60, 400 };
        const wchar_t* desc = (g_language == 0)
            ? L"Собирайте пранки из блоков, как в конструкторе.\n"
              L"Добавляйте эффекты, звуки, окна — и получайте готовый .exe."
            : L"Build pranks from blocks like a constructor.\n"
              L"Add effects, sounds, windows — and get a ready .exe.";
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, Colors::TEXT_DIM);
        HFONT f = CreateFontW(15, 0, 0, 0, FW_NORMAL, 0, 0, 0,
            DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
        HFONT old = (HFONT)SelectObject(dc, f);
        DrawTextW(dc, desc, -1, &descRc,
            DT_CENTER | DT_TOP | DT_WORDBREAK);
        SelectObject(dc, old);
        DeleteObject(f);

        // Кнопка "Начать"
        RECT startBtn = { rc.right / 2 - 100, 430, rc.right / 2 + 100, 480 };
        const wchar_t* startTxt = (g_language == 0) ? L"Начать" : L"Start";
        DrawCustomButton(dc, startBtn, startTxt, hoverStart, true);

        // Переключатель языка
        RECT ruBtn = { 30, rc.bottom - 60, 110, rc.bottom - 30 };
        RECT enBtn = { 120, rc.bottom - 60, 200, rc.bottom - 30 };
        DrawCustomButton(dc, ruBtn, L"Русский",
            hoverRu, g_language == 0);
        DrawCustomButton(dc, enBtn, L"English",
            hoverEn, g_language == 1);

        // Нижние кнопки
        RECT docBtn = { 240, rc.bottom - 60, 360, rc.bottom - 30 };
        RECT aboutBtn = { 370, rc.bottom - 60, 490, rc.bottom - 30 };
        const wchar_t* docTxt = (g_language == 0) ? L"Документация" : L"Documentation";
        const wchar_t* aboutTxt = (g_language == 0) ? L"О программе" : L"About";
        DrawCustomButton(dc, docBtn, docTxt, hoverDoc, false);
        DrawCustomButton(dc, aboutBtn, aboutTxt, hoverAbout, false);

        // Версия
        RECT verRc = { rc.right - 120, rc.bottom - 40, rc.right - 10, rc.bottom - 10 };
        DrawTextCentered(dc, L"v1.0.0", verRc, Colors::TEXT_DIM, 12, false);

        // Волнистые линии
        HPEN pen = CreatePen(PS_SOLID, 2, Colors::ACCENT);
        HPEN oldPen = (HPEN)SelectObject(dc, pen);
        for (int i = 0; i < 5; ++i) {
            int y = 250 + i * 8;
            MoveToEx(dc, 80, y, nullptr);
            for (int x = 80; x < rc.right - 80; x += 10) {
                int wy = y + (int)(5 * sin((x + i * 30) * 0.05));
                LineTo(dc, x, wy);
            }
        }
        SelectObject(dc, oldPen);
        DeleteObject(pen);

        EndPaint(h, &ps);
        return 0;
    }

    case WM_MOUSEMOVE: {
        int x = LOWORD(l), y = HIWORD(l);
        RECT rc; GetClientRect(h, &rc);

        RECT startBtn = { rc.right / 2 - 100, 430, rc.right / 2 + 100, 480 };
        RECT ruBtn = { 30, rc.bottom - 60, 110, rc.bottom - 30 };
        RECT enBtn = { 120, rc.bottom - 60, 200, rc.bottom - 30 };
        RECT docBtn = { 240, rc.bottom - 60, 360, rc.bottom - 30 };
        RECT aboutBtn = { 370, rc.bottom - 60, 490, rc.bottom - 30 };
        RECT closeBtn = { rc.right - 40, 0, rc.right, 40 };

        auto inside = [](const RECT& r, int px, int py) {
            return px >= r.left && px <= r.right && py >= r.top && py <= r.bottom;
        };

        int ns = inside(startBtn, x, y);
        int nr = inside(ruBtn, x, y);
        int ne = inside(enBtn, x, y);
        int nd = inside(docBtn, x, y);
        int na = inside(aboutBtn, x, y);
        int nc = inside(closeBtn, x, y);

        if (ns != hoverStart || nr != hoverRu || ne != hoverEn ||
            nd != hoverDoc || na != hoverAbout || nc != hoverExit) {
            hoverStart = ns; hoverRu = nr; hoverEn = ne;
            hoverDoc = nd; hoverAbout = na; hoverExit = nc;
            InvalidateRect(h, nullptr, FALSE);
        }

        // Курсор "рука" на кнопках
        if (ns || nr || ne || nd || na || nc) SetCursor(LoadCursor(nullptr, IDC_HAND));
        else SetCursor(LoadCursor(nullptr, IDC_ARROW));
        return 0;
    }

    case WM_LBUTTONDOWN: {
        int x = LOWORD(l), y = HIWORD(l);
        RECT rc; GetClientRect(h, &rc);

        RECT startBtn = { rc.right / 2 - 100, 430, rc.right / 2 + 100, 480 };
        RECT ruBtn = { 30, rc.bottom - 60, 110, rc.bottom - 30 };
        RECT enBtn = { 120, rc.bottom - 60, 200, rc.bottom - 30 };
        RECT docBtn = { 240, rc.bottom - 60, 360, rc.bottom - 30 };
        RECT aboutBtn = { 370, rc.bottom - 60, 490, rc.bottom - 30 };
        RECT closeBtn = { rc.right - 40, 0, rc.right, 40 };

        auto inside = [](const RECT& r, int px, int py) {
            return px >= r.left && px <= r.right && py >= r.top && py <= r.bottom;
        };

        if (inside(startBtn, x, y)) {
            // Открываем главное окно
            DestroyWindow(h);
            // Создастся в WM_DESTROY — через PostQuitMessage нельзя,
            // поэтому просто показываем main через CreateWindow
            extern void OpenMainWindow();
            OpenMainWindow();
            return 0;
        }
        if (inside(ruBtn, x, y)) { g_language = 0; InvalidateRect(h, nullptr, TRUE); return 0; }
        if (inside(enBtn, x, y)) { g_language = 1; InvalidateRect(h, nullptr, TRUE); return 0; }
        if (inside(docBtn, x, y)) {
            MessageBoxW(h, L"Документация скоро появится.\n\nПока — читай README на GitHub.",
                L"PrankForge", MB_OK | MB_ICONINFORMATION);
            return 0;
        }
        if (inside(aboutBtn, x, y)) {
            MessageBoxW(h,
                L"PrankForge v1.0.0\n\n"
                L"Конструктор пранков.\n"
                L"Собирай из блоков, получай .exe.",
                L"О программе", MB_OK | MB_ICONINFORMATION);
            return 0;
        }
        if (inside(closeBtn, x, y)) {
            PostQuitMessage(0);
            return 0;
        }
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(h, m, w, l);
}

// ============================================================
// CREATE PROJECT DIALOG
// ============================================================
LRESULT CALLBACK CreateProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    static int hoverBrowse = 0, hoverOk = 0, hoverCancel = 0;

    switch (m) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(h, &ps);
        RECT rc; GetClientRect(h, &rc);

        FillRectC(dc, rc, Colors::BG);

        // Заголовок
        RECT titleRc = { 20, 15, rc.right - 20, 50 };
        DrawTextLeft(dc, (g_language == 0) ? L"Создание нового проекта" : L"Create new project",
            titleRc, Colors::TEXT, 20, true);

        // Поле "Название"
        RECT label1 = { 20, 70, rc.right - 20, 95 };
        DrawTextLeft(dc, (g_language == 0) ? L"Название:" : L"Name:",
            label1, Colors::TEXT_DIM, 14, false);

        // Поле "Путь"
        RECT label2 = { 20, 140, rc.right - 20, 165 };
        DrawTextLeft(dc, (g_language == 0) ? L"Расположение:" : L"Location:",
            label2, Colors::TEXT_DIM, 14, false);

        // Кнопка обзора
        RECT browseBtn = { rc.right - 120, 165, rc.right - 20, 200 };
        DrawCustomButton(dc, browseBtn, L"...", hoverBrowse, false);

        // Кнопки снизу
        RECT cancelBtn = { rc.right - 240, rc.bottom - 60, rc.right - 140, rc.bottom - 20 };
        RECT okBtn = { rc.right - 130, rc.bottom - 60, rc.right - 20, rc.bottom - 20 };
        const wchar_t* okTxt = (g_language == 0) ? L"Создать" : L"Create";
        const wchar_t* cancelTxt = (g_language == 0) ? L"Отмена" : L"Cancel";
        DrawCustomButton(dc, cancelBtn, cancelTxt, hoverCancel, false);
        DrawCustomButton(dc, okBtn, okTxt, hoverOk, true);

        EndPaint(h, &ps);
        return 0;
    }

    case WM_CTLCOLOREDIT: {
        HDC dc = (HDC)w;
        SetTextColor(dc, Colors::TEXT);
        SetBkColor(dc, Colors::BG_PANEL);
        static HBRUSH hbr = CreateSolidBrush(Colors::BG_PANEL);
        return (LRESULT)hbr;
    }

    case WM_MOUSEMOVE: {
        int x = LOWORD(l), y = HIWORD(l);
        RECT rc; GetClientRect(h, &rc);
        RECT browseBtn = { rc.right - 120, 165, rc.right - 20, 200 };
        RECT cancelBtn = { rc.right - 240, rc.bottom - 60, rc.right - 140, rc.bottom - 20 };
        RECT okBtn = { rc.right - 130, rc.bottom - 60, rc.right - 20, rc.bottom - 20 };

        auto inside = [](const RECT& r, int px, int py) {
            return px >= r.left && px <= r.right && py >= r.top && py <= r.bottom;
        };

        int nb = inside(browseBtn, x, y);
        int nc = inside(cancelBtn, x, y);
        int no = inside(okBtn, x, y);
        if (nb != hoverBrowse || nc != hoverCancel || no != hoverOk) {
            hoverBrowse = nb; hoverCancel = nc; hoverOk = no;
            InvalidateRect(h, nullptr, FALSE);
        }
        if (nb || nc || no) SetCursor(LoadCursor(nullptr, IDC_HAND));
        else SetCursor(LoadCursor(nullptr, IDC_ARROW));
        return 0;
    }

    case WM_LBUTTONDOWN: {
        int x = LOWORD(l), y = HIWORD(l);
        RECT rc; GetClientRect(h, &rc);
        RECT browseBtn = { rc.right - 120, 165, rc.right - 20, 200 };
        RECT cancelBtn = { rc.right - 240, rc.bottom - 60, rc.right - 140, rc.bottom - 20 };
        RECT okBtn = { rc.right - 130, rc.bottom - 60, rc.right - 20, rc.bottom - 20 };

        auto inside = [](const RECT& r, int px, int py) {
            return px >= r.left && px <= r.right && py >= r.top && py <= r.bottom;
        };

        if (inside(browseBtn, x, y)) {
            BROWSEINFOW bi = {};
            bi.hwndOwner = h;
            bi.lpszTitle = L"Выбери папку для проекта";
            bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
            LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
            if (pidl) {
                wchar_t path[MAX_PATH];
                SHGetPathFromIDListW(pidl, path);
                SetWindowTextW(g_createEditPath, path);
                CoTaskMemFree(pidl);
            }
            return 0;
        }
        if (inside(okBtn, x, y)) {
            wchar_t name[256] = {};
            wchar_t path[MAX_PATH] = {};
            GetWindowTextW(g_createEditName, name, 256);
            GetWindowTextW(g_createEditPath, path, MAX_PATH);

            if (wcslen(name) == 0) {
                MessageBoxW(h, L"Введи название проекта.",
                    L"Ошибка", MB_OK | MB_ICONWARNING);
                return 0;
            }

            // Создаём структуру папки проекта
            wchar_t projDir[MAX_PATH];
            swprintf_s(projDir, L"%s\\%s", path, name);

            if (!CreateDirectoryW(projDir, nullptr)) {
                if (GetLastError() != ERROR_ALREADY_EXISTS) {
                    MessageBoxW(h, L"Не удалось создать папку.",
                        L"Ошибка", MB_OK | MB_ICONERROR);
                    return 0;
                }
            }

            // Создаём пустой файл .pforge
            wchar_t projFile[MAX_PATH];
            swprintf_s(projFile, L"%s\\project.pforge", projDir);
            HANDLE f = CreateFileW(projFile, GENERIC_WRITE, 0, nullptr,
                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (f != INVALID_HANDLE_VALUE) {
                const char* emptyJson = "{\n  \"name\": \"\",\n  \"blocks\": []\n}\n";
                DWORD wr;
                WriteFile(f, emptyJson, (DWORD)strlen(emptyJson), &wr, nullptr);
                CloseHandle(f);
            }

            // Добавляем в список
            ProjectInfo pi;
            pi.name = name;
            pi.path = projDir;
            SYSTEMTIME st; GetLocalTime(&st);
            wchar_t dateBuf[64];
            swprintf_s(dateBuf, L"%04d-%02d-%02d %02d:%02d",
                st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
            pi.date = dateBuf;
            g_projects.push_back(pi);

            // Обновляем main
            if (g_mainWnd) InvalidateRect(g_mainWnd, nullptr, TRUE);
            DestroyWindow(h);
            return 0;
        }
        if (inside(cancelBtn, x, y)) {
            DestroyWindow(h);
            return 0;
        }
        return 0;
    }

    case WM_CLOSE:
        DestroyWindow(h);
        return 0;
    }
    return DefWindowProcW(h, m, w, l);
}

// ============================================================
// MAIN WINDOW
// ============================================================
LRESULT CALLBACK MainProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    static int hoverCreate = 0, hoverSettings = 0, hoverRefresh = 0;
    static int hoveredProject = -1;

    switch (m) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(h, &ps);
        RECT rc; GetClientRect(h, &rc);

        FillRectC(dc, rc, Colors::BG);

        // Сайдбар слева
        RECT sidebar = { 0, 0, 250, rc.bottom };
        FillRectC(dc, sidebar, Colors::BG_DARK);

        // Логотип в сайдбаре
        RECT logoRc = { 0, 20, 250, 70 };
        DrawTextCentered(dc, L"⚒ PrankForge", logoRc, Colors::ACCENT, 22, true);

        // Пункты сайдбара
        const wchar_t* items[] = {
            (g_language == 0) ? L"📁 Проекты" : L"📁 Projects",
            (g_language == 0) ? L"📋 Шаблоны" : L"📋 Templates",
            (g_language == 0) ? L"🎨 Эффекты" : L"🎨 Effects",
            (g_language == 0) ? L"📜 История" : L"📜 History"
        };
        for (int i = 0; i < 4; ++i) {
            RECT itemRc = { 10, 100 + i * 45, 240, 140 + i * 45 };
            if (i == 0) FillRectC(dc, itemRc, Colors::BG_PANEL);
            DrawTextLeft(dc, items[i], itemRc, Colors::TEXT, 15, false);
        }

        // Кнопка настроек внизу
        RECT settingsBtn = { 10, rc.bottom - 60, 240, rc.bottom - 15 };
        const wchar_t* setTxt = (g_language == 0) ? L"⚙ Настройки" : L"⚙ Settings";
        DrawCustomButton(dc, settingsBtn, setTxt, hoverSettings, false);

        // Верхняя панель
        RECT topbar = { 250, 0, rc.right, 70 };
        FillRectC(dc, topbar, Colors::BG_DARK);

        // Заголовок
        RECT titleRc = { 270, 15, 500, 55 };
        DrawTextLeft(dc, (g_language == 0) ? L"Мои проекты" : L"My projects",
            titleRc, Colors::TEXT, 22, true);

        // Кнопка "Создать новый проект"
        RECT createBtn = { rc.right - 280, 15, rc.right - 20, 55 };
        const wchar_t* createTxt = (g_language == 0) ? L"+ Создать проект" : L"+ Create project";
        DrawCustomButton(dc, createBtn, createTxt, hoverCreate, true);

        // Список проектов
        int y = 100;
        if (g_projects.empty()) {
            RECT emptyRc = { 250, rc.bottom / 2 - 40, rc.right, rc.bottom / 2 + 40 };
            DrawTextCentered(dc, (g_language == 0)
                ? L"Пока нет проектов.\nНажми «Создать проект»."
                : L"No projects yet.\nClick «Create project».",
                emptyRc, Colors::TEXT_DIM, 18, false);
        } else {
            for (size_t i = 0; i < g_projects.size(); ++i) {
                RECT card = { 270, y, rc.right - 20, y + 90 };
                bool hovered = ((int)i == hoveredProject);
                FillRectC(dc, card, hovered ? Colors::BG_HOVER : Colors::BG_PANEL);

                HBRUSH border = CreateSolidBrush(Colors::BORDER);
                FrameRect(dc, &card, border);
                DeleteObject(border);

                // Название
                RECT nameRc = { card.left + 20, card.top + 10, card.right - 20, card.top + 45 };
                DrawTextLeft(dc, g_projects[i].name.c_str(), nameRc,
                    Colors::TEXT, 18, true);

                // Путь и дата
                RECT infoRc = { card.left + 20, card.top + 45, card.right - 20, card.top + 80 };
                std::wstring info = g_projects[i].path + L"   ·   " + g_projects[i].date;
                DrawTextLeft(dc, info.c_str(), infoRc, Colors::TEXT_DIM, 13, false);

                y += 100;
            }
        }

        EndPaint(h, &ps);
        return 0;
    }

    case WM_MOUSEMOVE: {
        int x = LOWORD(l), y = HIWORD(l);
        RECT rc; GetClientRect(h, &rc);

        RECT createBtn = { rc.right - 280, 15, rc.right - 20, 55 };
        RECT settingsBtn = { 10, rc.bottom - 60, 240, rc.bottom - 15 };

        auto inside = [](const RECT& r, int px, int py) {
            return px >= r.left && px <= r.right && py >= r.top && py <= r.bottom;
        };

        int nc = inside(createBtn, x, y);
        int ns = inside(settingsBtn, x, y);

        // Проверка hover над проектами
        int hp = -1;
        if (x > 250) {
            int yy = 100;
            for (size_t i = 0; i < g_projects.size(); ++i) {
                RECT card = { 270, yy, rc.right - 20, yy + 90 };
                if (inside(card, x, y)) { hp = (int)i; break; }
                yy += 100;
            }
        }

        if (nc != hoverCreate || ns != hoverSettings || hp != hoveredProject) {
            hoverCreate = nc; hoverSettings = ns; hoveredProject = hp;
            InvalidateRect(h, nullptr, FALSE);
        }
        if (nc || ns || hp >= 0) SetCursor(LoadCursor(nullptr, IDC_HAND));
        else SetCursor(LoadCursor(nullptr, IDC_ARROW));
        return 0;
    }

    case WM_LBUTTONDOWN: {
        int x = LOWORD(l), y = HIWORD(l);
        RECT rc; GetClientRect(h, &rc);

        RECT createBtn = { rc.right - 280, 15, rc.right - 20, 55 };
        RECT settingsBtn = { 10, rc.bottom - 60, 240, rc.bottom - 15 };

        auto inside = [](const RECT& r, int px, int py) {
            return px >= r.left && px <= r.right && py >= r.top && py <= r.bottom;
        };

        if (inside(createBtn, x, y)) {
            // Открываем диалог создания
            RECT dlg = { 0, 0, 600, 320 };
            AdjustWindowRect(&dlg, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME, FALSE);
            int dlgW = dlg.right - dlg.left;
            int dlgH = dlg.bottom - dlg.top;
            int px = (GetSystemMetrics(SM_CXSCREEN) - dlgW) / 2;
            int py = (GetSystemMetrics(SM_CYSCREEN) - dlgH) / 2;

            HWND dlgWnd = CreateWindowExW(WS_EX_DLGMODALFRAME,
                CLASS_CREATE, L"PrankForge — Новый проект",
                WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
                px, py, 600, 320,
                h, nullptr, g_hInst, nullptr);

            // Создаём поля ввода внутри диалога
            g_createEditName = CreateWindowExW(0, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                20, 100, 560, 32, dlgWnd, nullptr, g_hInst, nullptr);

            g_createEditPath = CreateWindowExW(0, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                20, 170, 440, 32, dlgWnd, nullptr, g_hInst, nullptr);

            // Красим edit-поля в тёмный цвет
            HFONT editFont = CreateFontW(15, 0, 0, 0, FW_NORMAL, 0, 0, 0,
                DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
            SendMessageW(g_createEditName, WM_SETFONT, (WPARAM)editFont, TRUE);
            SendMessageW(g_createEditPath, WM_SETFONT, (WPARAM)editFont, TRUE);

            ShowWindow(dlgWnd, SW_SHOW);
            UpdateWindow(dlgWnd);
            return 0;
        }

        if (inside(settingsBtn, x, y)) {
            MessageBoxW(h, L"Настройки появятся на следующем этапе разработки.",
                L"PrankForge", MB_OK | MB_ICONINFORMATION);
            return 0;
        }

        // Клик по проекту — открыть (заглушка)
        if (x > 250) {
            int yy = 100;
            for (size_t i = 0; i < g_projects.size(); ++i) {
                RECT card = { 270, yy, rc.right - 20, yy + 90 };
                if (inside(card, x, y)) {
                    std::wstring msg = L"Открыть проект «" + g_projects[i].name +
                        L"»?\n\n(Редактор появится на этапе 2)";
                    MessageBoxW(h, msg.c_str(), L"PrankForge",
                        MB_OK | MB_ICONINFORMATION);
                    return 0;
                }
                yy += 100;
            }
        }
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(h, m, w, l);
}

// ============================================================
// ОТКРЫТИЕ ГЛАВНОГО ОКНА
// ============================================================
void OpenMainWindow() {
    RECT rc = { 0, 0, 1280, 720 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;
    int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

    g_mainWnd = CreateWindowExW(0,
        CLASS_MAIN, L"PrankForge",
        WS_OVERLAPPEDWINDOW,
        x, y, w, h,
        nullptr, nullptr, g_hInst, nullptr);

    ShowWindow(g_mainWnd, SW_SHOW);
    UpdateWindow(g_mainWnd);
}

// ============================================================
// РЕГИСТРАЦИЯ КЛАССОВ
// ============================================================
void RegisterClasses() {
    WNDCLASSW wc = {};
    wc.hInstance = g_hInst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;

    wc.lpfnWndProc = WelcomeProc;
    wc.lpszClassName = CLASS_WELCOME;
    RegisterClassW(&wc);

    wc.lpfnWndProc = MainProc;
    wc.lpszClassName = CLASS_MAIN;
    RegisterClassW(&wc);

    wc.lpfnWndProc = CreateProc;
    wc.lpszClassName = CLASS_CREATE;
    RegisterClassW(&wc);
}

// ============================================================
// ТОЧКА ВХОДА
// ============================================================
int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int) {
    g_hInst = hInst;

    // Проверка единственного экземпляра
    HANDLE hMutex = CreateMutexW(nullptr, TRUE, L"PrankForgeMutex_v1");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBoxW(nullptr, L"PrankForge уже запущен.",
            L"PrankForge", MB_OK | MB_ICONINFORMATION);
        return 0;
    }

    SetProcessDPIAware();
    RegisterClasses();

    // Открываем Welcome
    RECT rc = { 0, 0, 700, 540 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, FALSE);
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;
    int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

    HWND welcome = CreateWindowExW(0,
        CLASS_WELCOME, L"PrankForge — Добро пожаловать",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        x, y, 700, 540,
        nullptr, nullptr, hInst, nullptr);

    ShowWindow(welcome, SW_SHOW);
    UpdateWindow(welcome);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (hMutex) { ReleaseMutex(hMutex); CloseHandle(hMutex); }
    return 0;
}
