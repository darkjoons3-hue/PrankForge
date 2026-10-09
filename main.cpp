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
#include <cmath>
#include <algorithm>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(linker, "/ENTRY:wWinMainCRTStartup")

// ============================================================
// ЦВЕТА
// ============================================================
namespace Colors {
    const COLORREF BG        = RGB(30, 30, 30);
    const COLORREF BG_DARK   = RGB(20, 20, 20);
    const COLORREF BG_PANEL  = RGB(45, 45, 45);
    const COLORREF BG_CANVAS = RGB(25, 25, 25);
    const COLORREF BG_HOVER  = RGB(60, 60, 60);
    const COLORREF ACCENT    = RGB(255, 107, 53);
    const COLORREF ACCENT_H  = RGB(255, 130, 80);
    const COLORREF TEXT      = RGB(224, 224, 224);
    const COLORREF TEXT_DIM  = RGB(150, 150, 150);
    const COLORREF BORDER    = RGB(70, 70, 70);
    const COLORREF GRID      = RGB(40, 40, 40);
    const COLORREF LINK      = RGB(255, 150, 100);

    // Цвета категорий блоков
    const COLORREF CAT_WELCOME  = RGB(100, 200, 100);
    const COLORREF CAT_GDI      = RGB(80, 180, 255);
    const COLORREF CAT_AUDIO    = RGB(255, 180, 80);
    const COLORREF CAT_MSGBOX   = RGB(255, 100, 100);
    const COLORREF CAT_LINK     = RGB(180, 100, 255);
    const COLORREF CAT_WALLPAPER= RGB(255, 200, 100);
    const COLORREF CAT_REGISTRY = RGB(200, 100, 100);
    const COLORREF CAT_REBOOT   = RGB(150, 50, 50);
}

// ============================================================
// ГЛОБАЛЫ
// ============================================================
HINSTANCE g_hInst = nullptr;
int g_language = 0;

const wchar_t* CLASS_WELCOME = L"PrankForgeWelcome";
const wchar_t* CLASS_MAIN    = L"PrankForgeMain";
const wchar_t* CLASS_CREATE  = L"PrankForgeCreate";
const wchar_t* CLASS_EDITOR  = L"PrankForgeEditor";

// ============================================================
// СТРУКТУРЫ
// ============================================================
enum BlockType {
    BT_WELCOME = 0,
    BT_GDI,
    BT_BYTEBEAT,
    BT_MSGBOX,
    BT_LINK,
    BT_WALLPAPER,
    BT_REGISTRY,
    BT_REBOOT,
    BT_COUNT
};

struct BlockCategory {
    BlockType type;
    const wchar_t* ruName;
    const wchar_t* enName;
    const wchar_t* icon;
    COLORREF color;
};

BlockCategory g_categories[BT_COUNT] = {
    { BT_WELCOME,   L"Приветствие",   L"Welcome",    L"👋", Colors::CAT_WELCOME },
    { BT_GDI,       L"GDI-эффект",    L"GDI effect", L"🎨", Colors::CAT_GDI },
    { BT_BYTEBEAT,  L"Bytebeat",      L"Bytebeat",   L"🎵", Colors::CAT_AUDIO },
    { BT_MSGBOX,    L"MessageBox",    L"MessageBox", L"⚠",  Colors::CAT_MSGBOX },
    { BT_LINK,      L"Ссылка",        L"Link",       L"🔗", Colors::CAT_LINK },
    { BT_WALLPAPER, L"Обои",          L"Wallpaper",  L"🖼", Colors::CAT_WALLPAPER },
    { BT_REGISTRY,  L"Блокировка",    L"Registry",   L"🔒", Colors::CAT_REGISTRY },
    { BT_REBOOT,    L"Ребут",         L"Reboot",     L"⏻", Colors::CAT_REBOOT }
};

// ============================================================
// БЛОК
// ============================================================
struct Block {
    int id;
    BlockType type;
    std::wstring title;
    int x, y;          // позиция на канвасе (в координатах канваса)
    int w, h;
    bool selected;

    Block() : id(0), type(BT_WELCOME), x(0), y(0), w(220), h(70), selected(false) {}
};

// ============================================================
// ДАННЫЕ ПРОЕКТА (для теста — глобально)
// ============================================================
struct ProjectInfo {
    std::wstring name;
    std::wstring path;
    std::wstring date;
};
std::vector<ProjectInfo> g_projects;
HWND g_mainWnd = nullptr;

// ============================================================
// СОСТОЯНИЕ РЕДАКТОРА
// ============================================================
struct EditorState {
    std::vector<Block> blocks;
    int nextId = 1;
    int selectedId = -1;

    // Скролл и зум
    int scrollX = 0, scrollY = 0;
    float zoom = 1.0f;

    // Drag
    bool dragging = false;
    int dragBlockId = -1;
    int dragOffsetX = 0, dragOffsetY = 0;

    // Панель палитры
    int expandedCategory = -1;
    int hoveredCategory = -1;
    int hoveredBlockInPalette = -1;

    // Свойства
    int hoveredProperty = -1;

    std::wstring projectName;
    std::wstring projectPath;
};
EditorState g_editor;

// ============================================================
// УТИЛИТЫ
// ============================================================
void FillRectC(HDC dc, const RECT& rc, COLORREF c) {
    HBRUSH b = CreateSolidBrush(c);
    FillRect(dc, &rc, b);
    DeleteObject(b);
}

void DrawTextCentered(HDC dc, const wchar_t* text, RECT rc,
                     COLORREF color, int fontSize, bool bold) {
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, color);
    HFONT f = CreateFontW(fontSize, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL,
        0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    HFONT old = (HFONT)SelectObject(dc, f);
    DrawTextW(dc, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(dc, old);
    DeleteObject(f);
}

void DrawTextLeft(HDC dc, const wchar_t* text, RECT rc,
                 COLORREF color, int fontSize, bool bold) {
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, color);
    HFONT f = CreateFontW(fontSize, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL,
        0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    HFONT old = (HFONT)SelectObject(dc, f);
    DrawTextW(dc, text, -1, &rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    SelectObject(dc, old);
    DeleteObject(f);
}

void DrawCustomButton(HDC dc, const RECT& rc, const wchar_t* text,
                     bool hovered, bool accent) {
    COLORREF bg = accent
        ? (hovered ? Colors::ACCENT_H : Colors::ACCENT)
        : (hovered ? Colors::BG_HOVER : Colors::BG_PANEL);
    FillRectC(dc, rc, bg);
    HBRUSH border = CreateSolidBrush(Colors::BORDER);
    FrameRect(dc, &rc, border);
    DeleteObject(border);
    DrawTextCentered(dc, text, rc,
        accent ? RGB(255, 255, 255) : Colors::TEXT, 14, true);
}

// ============================================================
// КООРДИНАТЫ: канвас <-> экран
// ============================================================
POINT CanvasToScreen(int cx, int cy) {
    POINT p;
    p.x = (int)((cx - g_editor.scrollX) * g_editor.zoom);
    p.y = (int)((cy - g_editor.scrollY) * g_editor.zoom);
    return p;
}

POINT ScreenToCanvas(int sx, int sy) {
    POINT p;
    p.x = (int)(sx / g_editor.zoom) + g_editor.scrollX;
    p.y = (int)(sy / g_editor.zoom) + g_editor.scrollY;
    return p;
}

// ============================================================
// ПОИСК БЛОКА ПО КООРДИНАТАМ ЭКРАНА
// ============================================================
Block* FindBlockAtScreen(int sx, int sy) {
    for (auto it = g_editor.blocks.rbegin(); it != g_editor.blocks.rend(); ++it) {
        POINT tl = CanvasToScreen(it->x, it->y);
        int sw = (int)(it->w * g_editor.zoom);
        int sh = (int)(it->h * g_editor.zoom);
        if (sx >= tl.x && sx <= tl.x + sw && sy >= tl.y && sy <= tl.y + sh) {
            return &(*it);
        }
    }
    return nullptr;
}

// ============================================================
// ПОИСК ПОРТОВ (центры блоков)
// ============================================================
POINT BlockRightPort(const Block& b) {
    return { b.x + b.w, b.y + b.h / 2 };
}
POINT BlockLeftPort(const Block& b) {
    return { b.x, b.y + b.h / 2 };
}

// ============================================================
// РИСОВАНИЕ БЛОКА
// ============================================================
void DrawBlock(HDC dc, const Block& b) {
    POINT tl = CanvasToScreen(b.x, b.y);
    int sw = (int)(b.w * g_editor.zoom);
    int sh = (int)(b.h * g_editor.zoom);

    RECT rc = { tl.x, tl.y, tl.x + sw, tl.y + sh };

    // Фон
    COLORREF catColor = g_categories[b.type].color;
    COLORREF bgCol = b.selected ? Colors::BG_HOVER : Colors::BG_PANEL;
    FillRectC(dc, rc, bgCol);

    // Левая цветная полоса категории
    RECT catBar = { tl.x, tl.y, tl.x + 6, tl.y + sh };
    FillRectC(dc, catBar, catColor);

    // Рамка (выделенная — акцентная)
    HBRUSH border = CreateSolidBrush(b.selected ? Colors::ACCENT : Colors::BORDER);
    FrameRect(dc, &rc, border);
    DeleteObject(border);

    // Иконка
    RECT iconRc = { tl.x + 15, tl.y + 5, tl.x + 50, tl.y + sh - 5 };
    DrawTextCentered(dc, g_categories[b.type].icon, iconRc, Colors::TEXT, 22, false);

    // Название
    RECT nameRc = { tl.x + 55, tl.y + 5, tl.x + sw - 10, tl.y + 35 };
    DrawTextLeft(dc, b.title.c_str(), nameRc, Colors::TEXT, 14, true);

    // Тип
    RECT typeRc = { tl.x + 55, tl.y + 35, tl.x + sw - 10, tl.y + sh - 5 };
    const wchar_t* typeName = (g_language == 0)
        ? g_categories[b.type].ruName : g_categories[b.type].enName;
    DrawTextLeft(dc, typeName, typeRc, Colors::TEXT_DIM, 11, false);

    // Порты (маленькие кружки справа и слева)
    POINT inPort = CanvasToScreen(b.x, b.y + b.h / 2);
    POINT outPort = CanvasToScreen(b.x + b.w, b.y + b.h / 2);

    HBRUSH portBrush = CreateSolidBrush(catColor);
    int portR = (int)(5 * g_editor.zoom);
    if (portR < 3) portR = 3;

    RECT inRc = { inPort.x - portR, inPort.y - portR, inPort.x + portR, inPort.y + portR };
    HBRUSH oldB = (HBRUSH)SelectObject(dc, portBrush);
    HPEN oldP = (HPEN)SelectObject(dc, GetStockObject(NULL_PEN));
    Ellipse(dc, inRc.left, inRc.top, inRc.right, inRc.bottom);

    RECT outRc = { outPort.x - portR, outPort.y - portR, outPort.x + portR, outPort.y + portR };
    Ellipse(dc, outRc.left, outRc.top, outRc.right, outRc.bottom);

    SelectObject(dc, oldB);
    SelectObject(dc, oldP);
    DeleteObject(portBrush);
}

// ============================================================
// РИСОВАНИЕ СОЕДИНЕНИЙ (по близости)
// ============================================================
void DrawConnections(HDC dc) {
    int n = (int)g_editor.blocks.size();
    HPEN linkPen = CreatePen(PS_SOLID, 2, Colors::LINK);
    HPEN oldPen = (HPEN)SelectObject(dc, linkPen);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i == j) continue;

            Block& a = g_editor.blocks[i];
            Block& b = g_editor.blocks[j];

            // Соединяем если блок B находится правее и ниже A, и расстояние небольшое
            int dx = b.x - (a.x + a.w);
            int dy = std::abs((b.y + b.h / 2) - (a.y + a.h / 2));

            if (dx > -50 && dx < 250 && dy < 200) {
                // Проверяем, что нет блока между ними
                bool blocked = false;
                for (int k = 0; k < n; ++k) {
                    if (k == i || k == j) continue;
                    Block& c = g_editor.blocks[k];
                    int cx = c.x + c.w / 2;
                    int cy = c.y + c.h / 2;
                    if (cx > a.x + a.w && cx < b.x && std::abs(cy - (a.y + a.h / 2)) < 60) {
                        blocked = true; break;
                    }
                }
                if (blocked) continue;

                POINT p1 = CanvasToScreen(a.x + a.w, a.y + a.h / 2);
                POINT p2 = CanvasToScreen(b.x, b.y + b.h / 2);

                // Рисуем кривую Безье (кубическую)
                int ctrlOffset = 50;
                POINT c1 = { p1.x + ctrlOffset, p1.y };
                POINT c2 = { p2.x - ctrlOffset, p2.y };

                POINT pts[4] = { p1, c1, c2, p2 };
                PolyBezier(dc, pts, 4);
            }
        }
    }

    SelectObject(dc, oldPen);
    DeleteObject(linkPen);
}

// ============================================================
// РИСОВАНИЕ СЕТКИ
// ============================================================
void DrawGrid(HDC dc, const RECT& rc) {
    int gridSize = (int)(50 * g_editor.zoom);
    if (gridSize < 10) gridSize = 10;

    // Смещение сетки с учётом scroll
    int startX = -(int)(g_editor.scrollX * g_editor.zoom) % gridSize;
    int startY = -(int)(g_editor.scrollY * g_editor.zoom) % gridSize;
    if (startX > 0) startX -= gridSize;
    if (startY > 0) startY -= gridSize;

    HPEN pen = CreatePen(PS_SOLID, 1, Colors::GRID);
    HPEN oldPen = (HPEN)SelectObject(dc, pen);

    for (int x = startX; x < rc.right; x += gridSize) {
        MoveToEx(dc, x, 0, nullptr);
        LineTo(dc, x, rc.bottom);
    }
    for (int y = startY; y < rc.bottom; y += gridSize) {
        MoveToEx(dc, 0, y, nullptr);
        LineTo(dc, rc.right, y);
    }

    SelectObject(dc, oldPen);
    DeleteObject(pen);
}

// ============================================================
// EDITOR WINDOW
// ============================================================
const int SIDEBAR_W = 260;
const int PROPS_W = 320;
const int TOPBAR_H = 45;
const int BOTTOM_H = 60;

int GetCanvasLeft() { return SIDEBAR_W; }
int GetCanvasRight(HWND h) {
    RECT rc; GetClientRect(h, &rc);
    return rc.right - PROPS_W;
}
int GetCanvasTop() { return TOPBAR_H; }
int GetCanvasBottom(HWND h) {
    RECT rc; GetClientRect(h, &rc);
    return rc.bottom - BOTTOM_H;
}

// Рисование палитры категорий
int DrawPalette(HDC dc, int top, int left, int right) {
    int y = top + 10;
    const int itemH = 36;

    for (int i = 0; i < BT_COUNT; ++i) {
        RECT itemRc = { left + 5, y, right - 5, y + itemH };
        bool hovered = (i == g_editor.hoveredCategory);
        bool expanded = (i == g_editor.expandedCategory);

        COLORREF bg = expanded ? Colors::BG_HOVER
                     : (hovered ? Colors::BG_HOVER : Colors::BG_PANEL);
        FillRectC(dc, itemRc, bg);

        // Полоска категории
        RECT catBar = { itemRc.left, itemRc.top, itemRc.left + 4, itemRc.bottom };
        FillRectC(dc, catBar, g_categories[i].color);

        // Иконка
        RECT iconRc = { itemRc.left + 10, itemRc.top, itemRc.left + 40, itemRc.bottom };
        DrawTextCentered(dc, g_categories[i].icon, iconRc, Colors::TEXT, 16, false);

        // Название
        const wchar_t* name = (g_language == 0)
            ? g_categories[i].ruName : g_categories[i].enName;
        RECT nameRc = { itemRc.left + 45, itemRc.top, itemRc.right - 30, itemRc.bottom };
        DrawTextLeft(dc, name, nameRc, Colors::TEXT, 13, true);

        // Стрелка раскрытия
        RECT arrowRc = { itemRc.right - 30, itemRc.top, itemRc.right - 10, itemRc.bottom };
        DrawTextCentered(dc, expanded ? L"▼" : L"▶", arrowRc, Colors::TEXT_DIM, 10, false);

        y += itemH + 4;

        // Если раскрыта — показываем блоки
        if (expanded) {
            // Подкатегории — одна строка "Перетащи в центр"
            RECT hintRc = { left + 15, y, right - 10, y + 30 };
            FillRectC(dc, hintRc, Colors::BG);
            DrawTextLeft(dc, (g_language == 0)
                ? L"Клик — добавить в центр"
                : L"Click to add to center",
                hintRc, Colors::TEXT_DIM, 11, false);
            y += 36;
        }
    }
    return y;
}

// Рисование панели свойств
void DrawProperties(HDC dc, HWND h) {
    RECT rc; GetClientRect(h, &rc);
    int left = rc.right - PROPS_W;
    RECT propRc = { left, TOPBAR_H, rc.right, rc.bottom - BOTTOM_H };
    FillRectC(dc, propRc, Colors::BG_DARK);

    // Заголовок
    RECT titleRc = { left + 15, TOPBAR_H + 15, rc.right - 15, TOPBAR_H + 50 };
    DrawTextLeft(dc, (g_language == 0) ? L"Свойства" : L"Properties",
        titleRc, Colors::TEXT, 16, true);

    if (g_editor.selectedId < 0) {
        RECT emptyRc = { left + 15, TOPBAR_H + 70, rc.right - 15, TOPBAR_H + 200 };
        DrawTextCentered(dc, (g_language == 0)
            ? L"Выбери блок,\nчтобы увидеть его настройки"
            : L"Select a block\nto see its settings",
            emptyRc, Colors::TEXT_DIM, 13, false);
        return;
    }

    // Находим блок
    Block* b = nullptr;
    for (auto& bl : g_editor.blocks) {
        if (bl.id == g_editor.selectedId) { b = &bl; break; }
    }
    if (!b) return;

    int y = TOPBAR_H + 70;

    // Название блока (можно редактировать — пока статично)
    RECT labelName = { left + 15, y, rc.right - 15, y + 20 };
    DrawTextLeft(dc, (g_language == 0) ? L"Название блока:" : L"Block name:",
        labelName, Colors::TEXT_DIM, 11, false);
    y += 25;

    RECT valueName = { left + 15, y, rc.right - 15, y + 30 };
    FillRectC(dc, valueName, Colors::BG_PANEL);
    DrawTextLeft(dc, b->title.c_str(), valueName, Colors::TEXT, 13, false);
    y += 40;

    // Тип
    RECT labelType = { left + 15, y, rc.right - 15, y + 20 };
    DrawTextLeft(dc, (g_language == 0) ? L"Тип:" : L"Type:",
        labelType, Colors::TEXT_DIM, 11, false);
    y += 25;
    RECT valueType = { left + 15, y, rc.right - 15, y + 30 };
    FillRectC(dc, valueType, Colors::BG_PANEL);
    const wchar_t* typeName = (g_language == 0)
        ? g_categories[b->type].ruName : g_categories[b->type].enName;
    DrawTextLeft(dc, typeName, valueType, Colors::TEXT, 13, false);
    y += 40;

    // Позиция
    wchar_t posBuf[64];
    swprintf_s(posBuf, L"X: %d, Y: %d", b->x, b->y);
    RECT labelPos = { left + 15, y, rc.right - 15, y + 20 };
    DrawTextLeft(dc, (g_language == 0) ? L"Позиция:" : L"Position:",
        labelPos, Colors::TEXT_DIM, 11, false);
    y += 25;
    RECT valuePos = { left + 15, y, rc.right - 15, y + 30 };
    FillRectC(dc, valuePos, Colors::BG_PANEL);
    DrawTextLeft(dc, posBuf, valuePos, Colors::TEXT, 13, false);
    y += 40;

    // Заглушка для параметров
    RECT labelParams = { left + 15, y, rc.right - 15, y + 20 };
    DrawTextLeft(dc, (g_language == 0) ? L"Параметры:" : L"Parameters:",
        labelParams, Colors::TEXT_DIM, 11, false);
    y += 25;
    RECT hint = { left + 15, y, rc.right - 15, y + 80 };
    FillRectC(dc, hint, Colors::BG_PANEL);
    DrawTextCentered(dc, (g_language == 0)
        ? L"Настройки блока\nпоявятся на Этапе 3"
        : L"Block settings\non Stage 3",
        hint, Colors::TEXT_DIM, 12, false);
}

LRESULT CALLBACK EditorProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    static int hoverSave = 0, hoverPreview = 0, hoverBuild = 0;

    switch (m) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(h, &ps);
        RECT rc; GetClientRect(h, &rc);

        // Общий фон
        FillRectC(dc, rc, Colors::BG);

        // ===== ТОПБАР =====
        RECT topbar = { 0, 0, rc.right, TOPBAR_H };
        FillRectC(dc, topbar, Colors::BG_DARK);

        // Название проекта слева
        std::wstring title = L"⚒ PrankForge — " + g_editor.projectName;
        RECT titleRc = { 15, 0, rc.right - 300, TOPBAR_H };
        DrawTextLeft(dc, title.c_str(), titleRc, Colors::TEXT, 15, true);

        // Кнопка "Сохранить"
        RECT saveBtn = { rc.right - 280, 8, rc.right - 180, TOPBAR_H - 8 };
        const wchar_t* saveTxt = (g_language == 0) ? L"💾 Сохранить" : L"💾 Save";
        DrawCustomButton(dc, saveBtn, saveTxt, hoverSave, false);

        // ===== САЙДБАР =====
        RECT sidebar = { 0, TOPBAR_H, SIDEBAR_W, rc.bottom - BOTTOM_H };
        FillRectC(dc, sidebar, Colors::BG_DARK);

        // Заголовок сайдбара
        RECT sideTitle = { 15, TOPBAR_H + 10, SIDEBAR_W - 15, TOPBAR_H + 40 };
        DrawTextLeft(dc, (g_language == 0) ? L"Палитра блоков" : L"Block palette",
            sideTitle, Colors::TEXT, 14, true);

        // Палитра
        DrawPalette(dc, TOPBAR_H + 50, 0, SIDEBAR_W);

        // ===== КАНВАС =====
        RECT canvas = { SIDEBAR_W, TOPBAR_H, rc.right - PROPS_W, rc.bottom - BOTTOM_H };
        FillRectC(dc, canvas, Colors::BG_CANVAS);

        // Ограничим рисование канваса
        HRGN hRgn = CreateRectRgn(canvas.left, canvas.top, canvas.right, canvas.bottom);
        SelectClipRgn(dc, hRgn);

        // Сетка
        DrawGrid(dc, canvas);

        // Соединения
        DrawConnections(dc);

        // Блоки
        for (auto& b : g_editor.blocks) {
            DrawBlock(dc, b);
        }

        SelectClipRgn(dc, nullptr);
        DeleteObject(hRgn);

        // ===== ПАНЕЛЬ СВОЙСТВ =====
        DrawProperties(dc, h);

        // ===== НИЖНЯЯ ПАНЕЛЬ =====
        RECT bottom = { 0, rc.bottom - BOTTOM_H, rc.right, rc.bottom };
        FillRectC(dc, bottom, Colors::BG_DARK);

        // Инфо слева
        wchar_t infoBuf[128];
        swprintf_s(infoBuf, L"Блоков: %d    Zoom: %.0f%%    Scroll: %d, %d",
            (int)g_editor.blocks.size(),
            g_editor.zoom * 100.0f,
            g_editor.scrollX, g_editor.scrollY);
        RECT infoRc = { 15, 0, rc.right - 500, BOTTOM_H };
        DrawTextLeft(dc, infoBuf, infoRc, Colors::TEXT_DIM, 12, false);

        // Кнопка "Предпросмотр"
        RECT previewBtn = { rc.right - 480, 12, rc.right - 260, BOTTOM_H - 12 };
        const wchar_t* prevTxt = (g_language == 0) ? L"▶ Предпросмотр" : L"▶ Preview";
        DrawCustomButton(dc, previewBtn, prevTxt, hoverPreview, false);

        // Кнопка "Собрать EXE"
        RECT buildBtn = { rc.right - 250, 12, rc.right - 15, BOTTOM_H - 12 };
        const wchar_t* buildTxt = (g_language == 0) ? L"🔨 Собрать EXE" : L"🔨 Build EXE";
        DrawCustomButton(dc, buildBtn, buildTxt, hoverBuild, true);

        EndPaint(h, &ps);
        return 0;
    }

    case WM_MOUSEMOVE: {
        int x = LOWORD(l), y = HIWORD(l);
        RECT rc; GetClientRect(h, &rc);

        // Обновление hover для кнопок нижней панели
        RECT previewBtn = { rc.right - 480, 12, rc.right - 260, BOTTOM_H - 12 };
        RECT buildBtn = { rc.right - 250, 12, rc.right - 15, BOTTOM_H - 12 };
        RECT saveBtn = { rc.right - 280, 8, rc.right - 180, TOPBAR_H - 8 };

        auto inside = [](const RECT& r, int px, int py) {
            return px >= r.left && px <= r.right && py >= r.top && py <= r.bottom;
        };

        int np = inside(previewBtn, x, y);
        int nb = inside(buildBtn, x, y);
        int ns = inside(saveBtn, x, y);

        int changed = 0;
        if (np != hoverPreview) { hoverPreview = np; changed = 1; }
        if (nb != hoverBuild) { hoverBuild = nb; changed = 1; }
        if (ns != hoverSave) { hoverSave = ns; changed = 1; }

        // Hover для палитры
        if (x < SIDEBAR_W && y > TOPBAR_H) {
            int palTop = TOPBAR_H + 50;
            int yy = palTop + 10;
            int newHover = -1;
            for (int i = 0; i < BT_COUNT; ++i) {
                RECT itemRc = { 5, yy, SIDEBAR_W - 5, yy + 36 };
                if (inside(itemRc, x, y)) { newHover = i; break; }
                yy += 40;
                if (i == g_editor.expandedCategory) yy += 36;
            }
            if (newHover != g_editor.hoveredCategory) {
                g_editor.hoveredCategory = newHover;
                changed = 1;
            }
        } else if (g_editor.hoveredCategory != -1) {
            g_editor.hoveredCategory = -1;
            changed = 1;
        }

        // Drag блока
        if (g_editor.dragging && g_editor.dragBlockId >= 0) {
            POINT p = ScreenToCanvas(x, y);
            for (auto& b : g_editor.blocks) {
                if (b.id == g_editor.dragBlockId) {
                    b.x = p.x - g_editor.dragOffsetX;
                    b.y = p.y - g_editor.dragOffsetY;
                    InvalidateRect(h, nullptr, FALSE);
                    return 0;
                }
            }
        }

        if (changed) InvalidateRect(h, nullptr, FALSE);

        // Курсор
        if (np || nb || ns) SetCursor(LoadCursor(nullptr, IDC_HAND));
        else if (x > SIDEBAR_W && x < rc.right - PROPS_W && y > TOPBAR_H && y < rc.bottom - BOTTOM_H) {
            if (FindBlockAtScreen(x, y)) SetCursor(LoadCursor(nullptr, IDC_SIZEALL));
            else SetCursor(LoadCursor(nullptr, IDC_ARROW));
        }
        else SetCursor(LoadCursor(nullptr, IDC_ARROW));
        return 0;
    }

    case WM_LBUTTONDOWN: {
        int x = LOWORD(l), y = HIWORD(l);
        RECT rc; GetClientRect(h, &rc);

        // Проверка кнопок
        RECT previewBtn = { rc.right - 480, 12, rc.right - 260, BOTTOM_H - 12 };
        RECT buildBtn = { rc.right - 250, 12, rc.right - 15, BOTTOM_H - 12 };
        RECT saveBtn = { rc.right - 280, 8, rc.right - 180, TOPBAR_H - 8 };

        auto inside = [](const RECT& r, int px, int py) {
            return px >= r.left && px <= r.right && py >= r.top && py <= r.bottom;
        };

        if (inside(saveBtn, x, y)) {
            MessageBoxW(h, L"Сохранение проекта — на Этапе 3.",
                L"PrankForge", MB_OK | MB_ICONINFORMATION);
            return 0;
        }
        if (inside(previewBtn, x, y)) {
            MessageBoxW(h, L"Предпросмотр — на Этапе 5.",
                L"PrankForge", MB_OK | MB_ICONINFORMATION);
            return 0;
        }
        if (inside(buildBtn, x, y)) {
            MessageBoxW(h, L"Сборка .exe — на Этапе 5.",
                L"PrankForge", MB_OK | MB_ICONINFORMATION);
            return 0;
        }

        // Клик по палитре
        if (x < SIDEBAR_W && y > TOPBAR_H + 50) {
            int palTop = TOPBAR_H + 50;
            int yy = palTop + 10;
            for (int i = 0; i < BT_COUNT; ++i) {
                RECT itemRc = { 5, yy, SIDEBAR_W - 5, yy + 36 };
                if (inside(itemRc, x, y)) {
                    // Toggle expanded
                    if (g_editor.expandedCategory == i) {
                        g_editor.expandedCategory = -1;
                    } else {
                        g_editor.expandedCategory = i;

                        // Добавляем блок в центр канваса
                        Block b;
                        b.id = g_editor.nextId++;
                        b.type = (BlockType)i;
                        const wchar_t* name = (g_language == 0)
                            ? g_categories[i].ruName : g_categories[i].enName;
                        b.title = name;

                        // Центр канваса
                        int cx = (GetCanvasLeft() + GetCanvasRight(h)) / 2;
                        int cy = (GetCanvasTop() + GetCanvasBottom(h)) / 2;
                        POINT canvasPt = ScreenToCanvas(cx, cy);
                        b.x = canvasPt.x - b.w / 2;
                        b.y = canvasPt.y - b.h / 2;
                        // Небольшое смещение, чтобы не пересекались
                        b.x += (int)(g_editor.blocks.size() % 5) * 30;
                        b.y += (int)(g_editor.blocks.size() % 5) * 30;

                        g_editor.blocks.push_back(b);
                        g_editor.selectedId = b.id;
                    }
                    InvalidateRect(h, nullptr, TRUE);
                    return 0;
                }
                yy += 40;
                if (i == g_editor.expandedCategory) yy += 36;
            }
        }

        // Клик по канвасу
        if (x > SIDEBAR_W && x < rc.right - PROPS_W && y > TOPBAR_H && y < rc.bottom - BOTTOM_H) {
            Block* b = FindBlockAtScreen(x, y);
            if (b) {
                // Начинаем drag
                g_editor.dragging = true;
                g_editor.dragBlockId = b->id;
                POINT canvasPt = ScreenToCanvas(x, y);
                g_editor.dragOffsetX = canvasPt.x - b->x;
                g_editor.dragOffsetY = canvasPt.y - b->y;

                // Выделяем
                for (auto& bl : g_editor.blocks) bl.selected = false;
                b->selected = true;
                g_editor.selectedId = b->id;
            } else {
                // Снимаем выделение
                for (auto& bl : g_editor.blocks) bl.selected = false;
                g_editor.selectedId = -1;
            }
            InvalidateRect(h, nullptr, TRUE);
            return 0;
        }
        return 0;
    }

    case WM_LBUTTONUP: {
        g_editor.dragging = false;
        g_editor.dragBlockId = -1;
        return 0;
    }

    case WM_MOUSEWHEEL: {
        int delta = GET_WHEEL_DELTA_WPARAM(w);
        bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

        if (ctrl) {
            // Zoom
            float oldZoom = g_editor.zoom;
            if (delta > 0) g_editor.zoom *= 1.1f;
            else g_editor.zoom /= 1.1f;
            if (g_editor.zoom < 0.25f) g_editor.zoom = 0.25f;
            if (g_editor.zoom > 3.0f) g_editor.zoom = 3.0f;
            (void)oldZoom;
        } else {
            // Scroll
            g_editor.scrollY -= delta / 3;
            if (g_editor.scrollY < 0) g_editor.scrollY = 0;
        }
        InvalidateRect(h, nullptr, TRUE);
        return 0;
    }

    case WM_KEYDOWN: {
        // Delete — удалить выбранный блок
        if (w == VK_DELETE && g_editor.selectedId >= 0) {
            for (auto it = g_editor.blocks.begin(); it != g_editor.blocks.end(); ++it) {
                if (it->id == g_editor.selectedId) {
                    g_editor.blocks.erase(it);
                    g_editor.selectedId = -1;
                    InvalidateRect(h, nullptr, TRUE);
                    break;
                }
            }
            return 0;
        }
        return 0;
    }

    case WM_DESTROY:
        return 0;
    }
    return DefWindowProcW(h, m, w, l);
}

// ============================================================
// ОТКРЫТИЕ РЕДАКТОРА
// ============================================================
void OpenEditor(HWND parent, int projectIndex) {
    if (projectIndex < 0 || projectIndex >= (int)g_projects.size()) return;

    g_editor.blocks.clear();
    g_editor.nextId = 1;
    g_editor.selectedId = -1;
    g_editor.scrollX = 0;
    g_editor.scrollY = 0;
    g_editor.zoom = 1.0f;
    g_editor.projectName = g_projects[projectIndex].name;
    g_editor.projectPath = g_projects[projectIndex].path;

    RECT rc = { 0, 0, 1400, 800 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;
    int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

    HWND editor = CreateWindowExW(0,
        CLASS_EDITOR,
        (L"PrankForge — " + g_editor.projectName).c_str(),
        WS_OVERLAPPEDWINDOW,
        x, y, w, h,
        parent, nullptr, g_hInst, nullptr);

    ShowWindow(editor, SW_SHOW);
    UpdateWindow(editor);
}

// ============================================================
// WELCOME / MAIN / CREATE — из Этапа 1 (сокращённо)
// ============================================================
LRESULT CALLBACK WelcomeProc(HWND h, UINT m, WPARAM w, LPARAM l);

LRESULT CALLBACK CreateProc(HWND h, UINT m, WPARAM w, LPARAM l);

// ============================================================
// MAIN WINDOW (упрощённая версия с открытием редактора)
// ============================================================
LRESULT CALLBACK MainProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    static int hoverCreate = 0, hoverSettings = 0;
    static int hoveredProject = -1;
    static int clickedProjectTime = 0;
    static int clickedProjectIdx = -1;

    switch (m) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(h, &ps);
        RECT rc; GetClientRect(h, &rc);
        FillRectC(dc, rc, Colors::BG);

        RECT sidebar = { 0, 0, 250, rc.bottom };
        FillRectC(dc, sidebar, Colors::BG_DARK);
        RECT logoRc = { 0, 20, 250, 70 };
        DrawTextCentered(dc, L"⚒ PrankForge", logoRc, Colors::ACCENT, 22, true);

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

        RECT settingsBtn = { 10, rc.bottom - 60, 240, rc.bottom - 15 };
        const wchar_t* setTxt = (g_language == 0) ? L"⚙ Настройки" : L"⚙ Settings";
        DrawCustomButton(dc, settingsBtn, setTxt, hoverSettings, false);

        RECT topbar = { 250, 0, rc.right, 70 };
        FillRectC(dc, topbar, Colors::BG_DARK);
        RECT titleRc = { 270, 15, 500, 55 };
        DrawTextLeft(dc, (g_language == 0) ? L"Мои проекты" : L"My projects",
            titleRc, Colors::TEXT, 22, true);

        RECT createBtn = { rc.right - 280, 15, rc.right - 20, 55 };
        const wchar_t* createTxt = (g_language == 0) ? L"+ Создать проект" : L"+ Create project";
        DrawCustomButton(dc, createBtn, createTxt, hoverCreate, true);

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

                RECT nameRc = { card.left + 20, card.top + 10, card.right - 20, card.top + 45 };
                DrawTextLeft(dc, g_projects[i].name.c_str(), nameRc, Colors::TEXT, 18, true);

                RECT infoRc = { card.left + 20, card.top + 45, card.right - 20, card.top + 80 };
                std::wstring info = g_projects[i].path + L"   ·   " + g_projects[i].date
                    + L"   ·   " + ((g_language == 0) ? L"двойной клик → открыть" : L"double-click → open");
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
            RECT dlg = { 0, 0, 600, 320 };
            AdjustWindowRect(&dlg, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME, FALSE);
            int dlgW = dlg.right - dlg.left;
            int dlgH = dlg.bottom - dlg.top;
            int px = (GetSystemMetrics(SM_CXSCREEN) - dlgW) / 2;
            int py = (GetSystemMetrics(SM_CYSCREEN) - dlgH) / 2;

            CreateWindowExW(WS_EX_DLGMODALFRAME,
                CLASS_CREATE, L"PrankForge — Новый проект",
                WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
                px, py, 600, 320,
                h, nullptr, g_hInst, nullptr);
            return 0;
        }
        if (inside(settingsBtn, x, y)) {
            MessageBoxW(h, L"Настройки — позже.",
                L"PrankForge", MB_OK | MB_ICONINFORMATION);
            return 0;
        }
        if (x > 250) {
            int yy = 100;
            for (size_t i = 0; i < g_projects.size(); ++i) {
                RECT card = { 270, yy, rc.right - 20, yy + 90 };
                if (inside(card, x, y)) {
                    clickedProjectTime = GetTickCount();
                    clickedProjectIdx = (int)i;
                    return 0;
                }
                yy += 100;
            }
        }
        return 0;
    }

    case WM_LBUTTONDBLCLK: {
        int x = LOWORD(l), y = HIWORD(l);
        RECT rc; GetClientRect(h, &rc);
        if (x > 250) {
            int yy = 100;
            for (size_t i = 0; i < g_projects.size(); ++i) {
                RECT card = { 270, yy, rc.right - 20, yy + 90 };
                if (x >= card.left && x <= card.right && y >= card.top && y <= card.bottom) {
                    OpenEditor(h, (int)i);
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

    g_mainWnd = CreateWindowExW(0, CLASS_MAIN, L"PrankForge",
        WS_OVERLAPPEDWINDOW, x, y, w, h,
        nullptr, nullptr, g_hInst, nullptr);

    ShowWindow(g_mainWnd, SW_SHOW);
    UpdateWindow(g_mainWnd);
}

// ============================================================
// WELCOME WINDOW (из Этапа 1)
// ============================================================
LRESULT CALLBACK WelcomeProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    static int hoverStart = 0, hoverRu = 0, hoverEn = 0;
    static int hoverDoc = 0, hoverAbout = 0, hoverExit = 0;

    switch (m) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(h, &ps);
        RECT rc; GetClientRect(h, &rc);

        FillRectC(dc, rc, Colors::BG);

        RECT topBar = { 0, 0, rc.right, 40 };
        FillRectC(dc, topBar, Colors::BG_DARK);

        RECT btnClose = { rc.right - 40, 0, rc.right, 40 };
        if (hoverExit) FillRectC(dc, btnClose, RGB(200, 50, 50));
        DrawTextCentered(dc, L"✕", btnClose, Colors::TEXT, 16, false);

        RECT logoRc = { 0, 100, rc.right, 180 };
        DrawTextCentered(dc, L"⚒ PrankForge", logoRc, Colors::ACCENT, 56, true);

        RECT slogRc = { 0, 180, rc.right, 220 };
        DrawTextCentered(dc, L"Build your chaos.", slogRc, Colors::TEXT_DIM, 18, false);

        RECT wellRc = { 0, 260, rc.right, 300 };
        const wchar_t* welcome = (g_language == 0)
            ? L"Добро пожаловать в конструктор пранков"
            : L"Welcome to the prank builder";
        DrawTextCentered(dc, welcome, wellRc, Colors::TEXT, 22, false);

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
        DrawTextW(dc, desc, -1, &descRc, DT_CENTER | DT_TOP | DT_WORDBREAK);
        SelectObject(dc, old);
        DeleteObject(f);

        RECT startBtn = { rc.right / 2 - 100, 430, rc.right / 2 + 100, 480 };
        const wchar_t* startTxt = (g_language == 0) ? L"Начать" : L"Start";
        DrawCustomButton(dc, startBtn, startTxt, hoverStart, true);

        RECT ruBtn = { 30, rc.bottom - 60, 110, rc.bottom - 30 };
        RECT enBtn = { 120, rc.bottom - 60, 200, rc.bottom - 30 };
        DrawCustomButton(dc, ruBtn, L"Русский", hoverRu, g_language == 0);
        DrawCustomButton(dc, enBtn, L"English", hoverEn, g_language == 1);

        RECT docBtn = { 240, rc.bottom - 60, 360, rc.bottom - 30 };
        RECT aboutBtn = { 370, rc.bottom - 60, 490, rc.bottom - 30 };
        DrawCustomButton(dc, docBtn,
            (g_language == 0) ? L"Документация" : L"Documentation", hoverDoc, false);
        DrawCustomButton(dc, aboutBtn,
            (g_language == 0) ? L"О программе" : L"About", hoverAbout, false);

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
            DestroyWindow(h);
            OpenMainWindow();
            return 0;
        }
        if (inside(ruBtn, x, y)) { g_language = 0; InvalidateRect(h, nullptr, TRUE); return 0; }
        if (inside(enBtn, x, y)) { g_language = 1; InvalidateRect(h, nullptr, TRUE); return 0; }
        if (inside(docBtn, x, y)) {
            MessageBoxW(h, L"Документация скоро.", L"PrankForge", MB_OK);
            return 0;
        }
        if (inside(aboutBtn, x, y)) {
            MessageBoxW(h, L"PrankForge v1.0.0", L"О программе", MB_OK);
            return 0;
        }
        if (inside(closeBtn, x, y)) { PostQuitMessage(0); return 0; }
        return 0;
    }
    case WM_DESTROY:
        if (!g_mainWnd) PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(h, m, w, l);
}

// ============================================================
// CREATE PROJECT
// ============================================================
HWND g_createEditName = nullptr;
HWND g_createEditPath = nullptr;

LRESULT CALLBACK CreateProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    static int hoverBrowse = 0, hoverOk = 0, hoverCancel = 0;

    switch (m) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(h, &ps);
        RECT rc; GetClientRect(h, &rc);
        FillRectC(dc, rc, Colors::BG);

        RECT titleRc = { 20, 15, rc.right - 20, 50 };
        DrawTextLeft(dc, (g_language == 0) ? L"Создание нового проекта" : L"Create new project",
            titleRc, Colors::TEXT, 20, true);

        RECT label1 = { 20, 70, rc.right - 20, 95 };
        DrawTextLeft(dc, (g_language == 0) ? L"Название:" : L"Name:",
            label1, Colors::TEXT_DIM, 14, false);

        RECT label2 = { 20, 140, rc.right - 20, 165 };
        DrawTextLeft(dc, (g_language == 0) ? L"Расположение:" : L"Location:",
            label2, Colors::TEXT_DIM, 14, false);

        RECT browseBtn = { rc.right - 120, 165, rc.right - 20, 200 };
        DrawCustomButton(dc, browseBtn, L"...", hoverBrowse, false);

        RECT cancelBtn = { rc.right - 240, rc.bottom - 60, rc.right - 140, rc.bottom - 20 };
        RECT okBtn = { rc.right - 130, rc.bottom - 60, rc.right - 20, rc.bottom - 20 };
        DrawCustomButton(dc, cancelBtn,
            (g_language == 0) ? L"Отмена" : L"Cancel", hoverCancel, false);
        DrawCustomButton(dc, okBtn,
            (g_language == 0) ? L"Создать" : L"Create", hoverOk, true);

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
                MessageBoxW(h, L"Введи название проекта.", L"Ошибка", MB_OK);
                return 0;
            }
            wchar_t projDir[MAX_PATH];
            swprintf_s(projDir, L"%s\\%s", path, name);
            CreateDirectoryW(projDir, nullptr);
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
            ProjectInfo pi;
            pi.name = name;
            pi.path = projDir;
            SYSTEMTIME st; GetLocalTime(&st);
            wchar_t dateBuf[64];
            swprintf_s(dateBuf, L"%04d-%02d-%02d %02d:%02d",
                st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
            pi.date = dateBuf;
            g_projects.push_back(pi);
            if (g_mainWnd) InvalidateRect(g_mainWnd, nullptr, TRUE);
            DestroyWindow(h);
            return 0;
        }
        if (inside(cancelBtn, x, y)) { DestroyWindow(h); return 0; }
        return 0;
    }
    case WM_CREATE: {
        RECT rc; GetClientRect(h, &rc);
        g_createEditName = CreateWindowExW(0, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            20, 100, 560, 32, h, nullptr, g_hInst, nullptr);
        g_createEditPath = CreateWindowExW(0, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            20, 170, 440, 32, h, nullptr, g_hInst, nullptr);
        HFONT editFont = CreateFontW(15, 0, 0, 0, FW_NORMAL, 0, 0, 0,
            DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
        SendMessageW(g_createEditName, WM_SETFONT, (WPARAM)editFont, TRUE);
        SendMessageW(g_createEditPath, WM_SETFONT, (WPARAM)editFont, TRUE);
        return 0;
    }
    case WM_CLOSE:
        DestroyWindow(h);
        return 0;
    }
    return DefWindowProcW(h, m, w, l);
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
    wc.style = CS_DBLCLKS;
    RegisterClassW(&wc);

    wc.lpfnWndProc = MainProc;
    wc.lpszClassName = CLASS_MAIN;
    wc.style = CS_DBLCLKS;
    RegisterClassW(&wc);

    wc.lpfnWndProc = CreateProc;
    wc.lpszClassName = CLASS_CREATE;
    wc.style = 0;
    RegisterClassW(&wc);

    wc.lpfnWndProc = EditorProc;
    wc.lpszClassName = CLASS_EDITOR;
    wc.style = CS_DBLCLKS;
    RegisterClassW(&wc);
}

// ============================================================
// ТОЧКА ВХОДА
// ============================================================
int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int) {
    g_hInst = hInst;

    HANDLE hMutex = CreateMutexW(nullptr, TRUE, L"PrankForgeMutex_v1");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBoxW(nullptr, L"PrankForge уже запущен.", L"PrankForge", MB_OK);
        return 0;
    }

    SetProcessDPIAware();
    RegisterClasses();

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
