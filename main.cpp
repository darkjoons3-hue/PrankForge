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
#include <map>

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
    const COLORREF SLIDER_BG = RGB(60, 60, 60);
    const COLORREF SLIDER_FG = RGB(255, 107, 53);

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
// БИБЛИОТЕКА GDI-ЭФФЕКТОВ
// ============================================================
struct GdiEffect {
    const wchar_t* ruName;
    const wchar_t* enName;
    int complexity; // 1=легко, 2=средне, 3=тяжело
};
GdiEffect g_gdiEffects[] = {
    { L"Screen Bend",      L"Screen Bend",      2 },
    { L"Ribbon",           L"Ribbon",           1 },
    { L"Screen Tear",      L"Screen Tear",      1 },
    { L"Zoom Blur",        L"Zoom Blur",        2 },
    { L"Kaleidoscope",     L"Kaleidoscope",     3 },
    { L"Screen Rotate",    L"Screen Rotate",    2 },
    { L"Water Ripple",     L"Water Ripple",     3 },
    { L"Squeeze",          L"Squeeze",          2 },
    { L"Tunnel",           L"Tunnel",           3 },
    { L"Invert",           L"Invert",           1 },
    { L"Grayscale",        L"Grayscale",        1 },
    { L"Red Filter",       L"Red Filter",       1 },
    { L"Magenta Filter",   L"Magenta Filter",   1 },
    { L"Rainbow",          L"Rainbow",          1 },
    { L"Threshold",        L"Threshold",        2 },
    { L"RGB Split",        L"RGB Split",        1 },
    { L"Glitch Blocks",    L"Glitch Blocks",    1 },
    { L"Stretch Blocks",   L"Stretch Blocks",   1 },
    { L"Scanlines",        L"Scanlines",        1 },
    { L"TV Noise",         L"TV Noise",         1 },
    { L"Matrix Rain",      L"Matrix Rain",      2 },
    { L"Pixelation",       L"Pixelation",       1 },
    { L"Shake",            L"Shake",            1 },
    { L"Drip",             L"Drip",             3 },
    { L"Holes",            L"Holes",            1 },
    { L"Cursor Trail",     L"Cursor Trail",     2 }
};
constexpr int kGdiEffectCount = 26;

// ============================================================
// БИБЛИОТЕКА BYTEBEAT
// ============================================================
struct ByteTrack {
    const wchar_t* name;
    const char* formula;
};
ByteTrack g_byteTracks[] = {
    { L"Quarknova",          "(t*(t>>8|t>>13)&63)+(t*(t>>11&t>>15)&127)" },
    { L"Monoxide",           "(t>>10&42)*t+(t|t%255|t%257)+(t&t>>8)+((t%((t>>8|t>>16)+1))^t)" },
    { L"Synth Progression",  "synth_progression" },
    { L"Ambient Low",        "(t>>12)|(t>>10)" },
    { L"Bass Drop",          "(t>>14)|(t*(t>>13))" },
    { L"Glitch Noise",       "(t^(t>>3))*(t>>8)" },
    { L"Nyan Bytebeat",      "t*(t>>8&t>>11)" },
    { L"Horror Drone",       "(t>>4)^(t>>8)^(t>>12)" },
    { L"Alarm Siren",        "((t>>6)|(t>>9))&0x7F" },
    { L"Digital Rain",       "((t>>10)^(t>>13))*(t>>8)" }
};
constexpr int kByteTrackCount = 10;

// ============================================================
// ГЛОБАЛЫ
// ============================================================
HINSTANCE g_hInst = nullptr;
int g_language = 0;

const wchar_t* CLASS_WELCOME  = L"PrankForgeWelcome";
const wchar_t* CLASS_MAIN     = L"PrankForgeMain";
const wchar_t* CLASS_CREATE   = L"PrankForgeCreate";
const wchar_t* CLASS_EDITOR   = L"PrankForgeEditor";
const wchar_t* CLASS_EFFECTSEL= L"PrankForgeEffectSel";

// ============================================================
// СТРУКТУРЫ
// ============================================================
enum BlockType {
    BT_WELCOME = 0, BT_GDI, BT_BYTEBEAT, BT_MSGBOX,
    BT_LINK, BT_WALLPAPER, BT_REGISTRY, BT_REBOOT, BT_COUNT
};

struct BlockCategory {
    BlockType type;
    const wchar_t* ruName;
    const wchar_t* enName;
    const wchar_t* icon;
    COLORREF color;
};
BlockCategory g_categories[BT_COUNT] = {
    { BT_WELCOME,   L"Приветствие", L"Welcome",    L"👋", Colors::CAT_WELCOME },
    { BT_GDI,       L"GDI-эффект",  L"GDI effect", L"🎨", Colors::CAT_GDI },
    { BT_BYTEBEAT,  L"Bytebeat",    L"Bytebeat",   L"🎵", Colors::CAT_AUDIO },
    { BT_MSGBOX,    L"MessageBox",  L"MessageBox", L"⚠",  Colors::CAT_MSGBOX },
    { BT_LINK,      L"Ссылка",      L"Link",       L"🔗", Colors::CAT_LINK },
    { BT_WALLPAPER, L"Обои",        L"Wallpaper",  L"🖼", Colors::CAT_WALLPAPER },
    { BT_REGISTRY,  L"Блокировка",  L"Registry",   L"🔒", Colors::CAT_REGISTRY },
    { BT_REBOOT,    L"Ребут",       L"Reboot",     L"⏻", Colors::CAT_REBOOT }
};

// Параметры блока
struct Block {
    int id;
    BlockType type;
    std::wstring title;
    int x, y, w, h;
    bool selected;

    // GDI
    int gdiEffect = 0;
    int gdiDuration = 10;   // сек
    int gdiIntensity = 60;  // 0-100

    // Bytebeat
    int bbTrack = 0;
    int bbVolume = 70;
    std::wstring bbFormula;

    // MessageBox
    std::wstring msgText = L"Сообщение";
    int msgIcon = 2;      // 0=info 1=warn 2=err 3=quest
    int msgCount = 1;
    int msgTrajectory = 0; // 0=circle-cursor 1=static 2=vertical 3=horizontal 4=random
    int msgDelay = 500;    // мс между окнами

    Block() : id(0), type(BT_WELCOME), x(0), y(0), w(220), h(70), selected(false) {}
};

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
    int scrollX = 0, scrollY = 0;
    float zoom = 1.0f;
    bool dragging = false;
    int dragBlockId = -1;
    int dragOffsetX = 0, dragOffsetY = 0;
    int expandedCategory = -1;
    int hoveredCategory = -1;

    // Слайдеры
    int activeSlider = -1; // 0=intensity, 1=duration, 2=volume, 3=msgcount, 4=msgdelay
    RECT sliderRects[8] = {};
    int sliderCount = 0;

    // Hover над кнопками
    int hoveredBtn = -1;
    RECT buttonRects[8] = {};
    int buttonCount = 0;

    std::wstring projectName;
    std::wstring projectPath;
    bool modified = false;
};
EditorState g_editor;
HWND g_editorWnd = nullptr;

// Состояние модального окна выбора эффекта
struct EffectSelState {
    HWND parent = nullptr;
    int scrollY = 0;
    int hovered = -1;
    int selected = -1;
    int forBlockId = -1; // для какого блока выбираем
};
EffectSelState g_effectSel;

// ============================================================
// УТИЛИТЫ
// ============================================================
void FillRectC(HDC dc, const RECT& rc, COLORREF c) {
    HBRUSH b = CreateSolidBrush(c);
    FillRect(dc, &rc, b);
    DeleteObject(b);
}

void DrawTextCentered(HDC dc, const wchar_t* text, RECT rc, COLORREF color, int fontSize, bool bold) {
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, color);
    HFONT f = CreateFontW(fontSize, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL,
        0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    HFONT old = (HFONT)SelectObject(dc, f);
    DrawTextW(dc, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(dc, old);
    DeleteObject(f);
}

void DrawTextLeft(HDC dc, const wchar_t* text, RECT rc, COLORREF color, int fontSize, bool bold) {
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, color);
    HFONT f = CreateFontW(fontSize, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL,
        0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    HFONT old = (HFONT)SelectObject(dc, f);
    DrawTextW(dc, text, -1, &rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    SelectObject(dc, old);
    DeleteObject(f);
}

void DrawCustomButton(HDC dc, const RECT& rc, const wchar_t* text, bool hovered, bool accent) {
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

// Рисование слайдера
void DrawSlider(HDC dc, const RECT& rc, int value, int maxVal, bool hovered) {
    // Фон слайдера
    RECT bgRc = { rc.left, rc.top + rc.bottom / 2 - 4, rc.right, rc.top + rc.bottom / 2 + 4 };
    // Проще: центральная полоса
    int midY = (rc.top + rc.bottom) / 2;
    RECT trackRc = { rc.left, midY - 4, rc.right, midY + 4 };
    FillRectC(dc, trackRc, Colors::SLIDER_BG);

    // Заполненная часть
    int fillW = (rc.right - rc.left) * value / (maxVal > 0 ? maxVal : 1);
    RECT fillRc = { rc.left, midY - 4, rc.left + fillW, midY + 4 };
    FillRectC(dc, fillRc, Colors::SLIDER_FG);

    // Ползунок
    int thumbX = rc.left + fillW;
    int thumbR = hovered ? 12 : 10;
    HBRUSH thumbBrush = CreateSolidBrush(hovered ? Colors::ACCENT_H : Colors::ACCENT);
    HBRUSH old = (HBRUSH)SelectObject(dc, thumbBrush);
    HPEN oldPen = (HPEN)SelectObject(dc, GetStockObject(NULL_PEN));
    Ellipse(dc, thumbX - thumbR, midY - thumbR, thumbX + thumbR, midY + thumbR);
    SelectObject(dc, old);
    SelectObject(dc, oldPen);
    DeleteObject(thumbBrush);
}

// ============================================================
// СОХРАНЕНИЕ / ЗАГРУЗКА JSON
// ============================================================
std::string WideToUtf8(const std::wstring& w) {
    if (w.empty()) return "";
    int sz = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string out(sz, 0);
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &out[0], sz, nullptr, nullptr);
    return out;
}

std::wstring Utf8ToWide(const std::string& s) {
    if (s.empty()) return L"";
    int sz = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    std::wstring out(sz, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &out[0], sz);
    return out;
}

std::string EscapeJson(const std::string& s) {
    std::string out;
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:   out += c;
        }
    }
    return out;
}

bool SaveProject() {
    if (g_editor.projectPath.empty() || g_editor.projectName.empty()) return false;

    wchar_t filePath[MAX_PATH];
    swprintf_s(filePath, L"%s\\project.pforge", g_editor.projectPath.c_str());

    std::string j;
    j += "{\n";
    j += "  \"name\": \"" + EscapeJson(WideToUtf8(g_editor.projectName)) + "\",\n";
    j += "  \"blocks\": [\n";

    for (size_t i = 0; i < g_editor.blocks.size(); ++i) {
        Block& b = g_editor.blocks[i];
        j += "    {\n";
        j += "      \"id\": " + std::to_string(b.id) + ",\n";
        j += "      \"type\": " + std::to_string((int)b.type) + ",\n";
        j += "      \"x\": " + std::to_string(b.x) + ",\n";
        j += "      \"y\": " + std::to_string(b.y) + ",\n";
        j += "      \"gdiEffect\": " + std::to_string(b.gdiEffect) + ",\n";
        j += "      \"gdiDuration\": " + std::to_string(b.gdiDuration) + ",\n";
        j += "      \"gdiIntensity\": " + std::to_string(b.gdiIntensity) + ",\n";
        j += "      \"bbTrack\": " + std::to_string(b.bbTrack) + ",\n";
        j += "      \"bbVolume\": " + std::to_string(b.bbVolume) + ",\n";
        j += "      \"msgText\": \"" + EscapeJson(WideToUtf8(b.msgText)) + "\",\n";
        j += "      \"msgIcon\": " + std::to_string(b.msgIcon) + ",\n";
        j += "      \"msgCount\": " + std::to_string(b.msgCount) + ",\n";
        j += "      \"msgTrajectory\": " + std::to_string(b.msgTrajectory) + ",\n";
        j += "      \"msgDelay\": " + std::to_string(b.msgDelay) + "\n";
        j += "    }";
        if (i + 1 < g_editor.blocks.size()) j += ",";
        j += "\n";
    }
    j += "  ]\n";
    j += "}\n";

    HANDLE f = CreateFileW(filePath, GENERIC_WRITE, 0, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (f == INVALID_HANDLE_VALUE) return false;

    DWORD wr;
    WriteFile(f, j.c_str(), (DWORD)j.size(), &wr, nullptr);
    CloseHandle(f);

    g_editor.modified = false;
    return true;
}

// ============================================================
// КООРДИНАТЫ
// ============================================================
POINT CanvasToScreen(int cx, int cy) {
    return { (int)((cx - g_editor.scrollX) * g_editor.zoom),
             (int)((cy - g_editor.scrollY) * g_editor.zoom) };
}
POINT ScreenToCanvas(int sx, int sy) {
    return { (int)(sx / g_editor.zoom) + g_editor.scrollX,
             (int)(sy / g_editor.zoom) + g_editor.scrollY };
}

Block* FindBlockAtScreen(int sx, int sy) {
    for (auto it = g_editor.blocks.rbegin(); it != g_editor.blocks.rend(); ++it) {
        POINT tl = CanvasToScreen(it->x, it->y);
        int sw = (int)(it->w * g_editor.zoom);
        int sh = (int)(it->h * g_editor.zoom);
        if (sx >= tl.x && sx <= tl.x + sw && sy >= tl.y && sy <= tl.y + sh)
            return &(*it);
    }
    return nullptr;
}

Block* FindBlockById(int id) {
    for (auto& b : g_editor.blocks) if (b.id == id) return &b;
    return nullptr;
}

// ============================================================
// РИСОВАНИЕ БЛОКА
// ============================================================
void DrawBlock(HDC dc, const Block& b) {
    POINT tl = CanvasToScreen(b.x, b.y);
    int sw = (int)(b.w * g_editor.zoom);
    int sh = (int)(b.h * g_editor.zoom);
    RECT rc = { tl.x, tl.y, tl.x + sw, tl.y + sh };

    COLORREF catColor = g_categories[b.type].color;
    FillRectC(dc, rc, b.selected ? Colors::BG_HOVER : Colors::BG_PANEL);

    RECT catBar = { tl.x, tl.y, tl.x + 6, tl.y + sh };
    FillRectC(dc, catBar, catColor);

    HBRUSH border = CreateSolidBrush(b.selected ? Colors::ACCENT : Colors::BORDER);
    FrameRect(dc, &rc, border);
    DeleteObject(border);

    RECT iconRc = { tl.x + 15, tl.y + 5, tl.x + 50, tl.y + sh - 5 };
    DrawTextCentered(dc, g_categories[b.type].icon, iconRc, Colors::TEXT, 22, false);

    RECT nameRc = { tl.x + 55, tl.y + 5, tl.x + sw - 10, tl.y + 35 };
    DrawTextLeft(dc, b.title.c_str(), nameRc, Colors::TEXT, 14, true);

    RECT typeRc = { tl.x + 55, tl.y + 35, tl.x + sw - 10, tl.y + sh - 5 };
    const wchar_t* typeName = (g_language == 0) ? g_categories[b.type].ruName : g_categories[b.type].enName;

    // Строка-описание под тип
    std::wstring subtitle = typeName;
    if (b.type == BT_GDI) {
        subtitle = g_gdiEffects[b.gdiEffect].ruName;
        wchar_t buf[64];
        swprintf_s(buf, L"%s · %d сек · %d%%", subtitle.c_str(),
            b.gdiDuration, b.gdiIntensity);
        subtitle = buf;
    } else if (b.type == BT_BYTEBEAT) {
        wchar_t buf[64];
        swprintf_s(buf, L"%s · %d%%", g_byteTracks[b.bbTrack].name, b.bbVolume);
        subtitle = buf;
    } else if (b.type == BT_MSGBOX) {
        wchar_t buf[64];
        swprintf_s(buf, L"%s · x%d", b.msgText.c_str(), b.msgCount);
        subtitle = buf;
    }
    DrawTextLeft(dc, subtitle.c_str(), typeRc, Colors::TEXT_DIM, 11, false);

    // Порты
    POINT inPort = CanvasToScreen(b.x, b.y + b.h / 2);
    POINT outPort = CanvasToScreen(b.x + b.w, b.y + b.h / 2);
    HBRUSH portBrush = CreateSolidBrush(catColor);
    int portR = (int)(5 * g_editor.zoom); if (portR < 3) portR = 3;
    HBRUSH oldB = (HBRUSH)SelectObject(dc, portBrush);
    HPEN oldP = (HPEN)SelectObject(dc, GetStockObject(NULL_PEN));
    Ellipse(dc, inPort.x - portR, inPort.y - portR, inPort.x + portR, inPort.y + portR);
    Ellipse(dc, outPort.x - portR, outPort.y - portR, outPort.x + portR, outPort.y + portR);
    SelectObject(dc, oldB);
    SelectObject(dc, oldP);
    DeleteObject(portBrush);
}

void DrawConnections(HDC dc) {
    int n = (int)g_editor.blocks.size();
    HPEN linkPen = CreatePen(PS_SOLID, 2, Colors::LINK);
    HPEN oldPen = (HPEN)SelectObject(dc, linkPen);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            Block& a = g_editor.blocks[i];
            Block& b = g_editor.blocks[j];
            int dx = b.x - (a.x + a.w);
            int dy = std::abs((b.y + b.h / 2) - (a.y + a.h / 2));
            if (dx > -50 && dx < 250 && dy < 200) {
                bool blocked = false;
                for (int k = 0; k < n; ++k) {
                    if (k == i || k == j) continue;
                    Block& c = g_editor.blocks[k];
                    int cx = c.x + c.w / 2;
                    int cy = c.y + c.h / 2;
                    if (cx > a.x + a.w && cx < b.x &&
                        std::abs(cy - (a.y + a.h / 2)) < 60) { blocked = true; break; }
                }
                if (blocked) continue;
                POINT p1 = CanvasToScreen(a.x + a.w, a.y + a.h / 2);
                POINT p2 = CanvasToScreen(b.x, b.y + b.h / 2);
                POINT c1 = { p1.x + 50, p1.y };
                POINT c2 = { p2.x - 50, p2.y };
                POINT pts[4] = { p1, c1, c2, p2 };
                PolyBezier(dc, pts, 4);
            }
        }
    }
    SelectObject(dc, oldPen);
    DeleteObject(linkPen);
}

void DrawGrid(HDC dc, const RECT& rc) {
    int gridSize = (int)(50 * g_editor.zoom); if (gridSize < 10) gridSize = 10;
    int startX = -(int)(g_editor.scrollX * g_editor.zoom) % gridSize;
    int startY = -(int)(g_editor.scrollY * g_editor.zoom) % gridSize;
    if (startX > 0) startX -= gridSize;
    if (startY > 0) startY -= gridSize;
    HPEN pen = CreatePen(PS_SOLID, 1, Colors::GRID);
    HPEN oldPen = (HPEN)SelectObject(dc, pen);
    for (int x = startX; x < rc.right; x += gridSize) {
        MoveToEx(dc, x, 0, nullptr); LineTo(dc, x, rc.bottom);
    }
    for (int y = startY; y < rc.bottom; y += gridSize) {
        MoveToEx(dc, 0, y, nullptr); LineTo(dc, rc.right, y);
    }
    SelectObject(dc, oldPen);
    DeleteObject(pen);
}

// ============================================================
// ПАНЕЛИ
// ============================================================
const int SIDEBAR_W = 260;
const int PROPS_W = 320;
const int TOPBAR_H = 45;
const int BOTTOM_H = 60;

int GetCanvasLeft() { return SIDEBAR_W; }
int GetCanvasRight(HWND h) { RECT rc; GetClientRect(h, &rc); return rc.right - PROPS_W; }
int GetCanvasTop() { return TOPBAR_H; }
int GetCanvasBottom(HWND h) { RECT rc; GetClientRect(h, &rc); return rc.bottom - BOTTOM_H; }

int DrawPalette(HDC dc, int top, int left, int right) {
    int y = top + 10;
    const int itemH = 36;
    for (int i = 0; i < BT_COUNT; ++i) {
        RECT itemRc = { left + 5, y, right - 5, y + itemH };
        bool hovered = (i == g_editor.hoveredCategory);
        bool expanded = (i == g_editor.expandedCategory);
        COLORREF bg = expanded ? Colors::BG_HOVER : (hovered ? Colors::BG_HOVER : Colors::BG_PANEL);
        FillRectC(dc, itemRc, bg);
        RECT catBar = { itemRc.left, itemRc.top, itemRc.left + 4, itemRc.bottom };
        FillRectC(dc, catBar, g_categories[i].color);
        RECT iconRc = { itemRc.left + 10, itemRc.top, itemRc.left + 40, itemRc.bottom };
        DrawTextCentered(dc, g_categories[i].icon, iconRc, Colors::TEXT, 16, false);
        const wchar_t* name = (g_language == 0) ? g_categories[i].ruName : g_categories[i].enName;
        RECT nameRc = { itemRc.left + 45, itemRc.top, itemRc.right - 30, itemRc.bottom };
        DrawTextLeft(dc, name, nameRc, Colors::TEXT, 13, true);
        RECT arrowRc = { itemRc.right - 30, itemRc.top, itemRc.right - 10, itemRc.bottom };
        DrawTextCentered(dc, expanded ? L"▼" : L"▶", arrowRc, Colors::TEXT_DIM, 10, false);
        y += itemH + 4;
        if (expanded) {
            RECT hintRc = { left + 15, y, right - 10, y + 30 };
            FillRectC(dc, hintRc, Colors::BG);
            DrawTextLeft(dc, (g_language == 0) ? L"Клик — добавить в центр" : L"Click to add",
                hintRc, Colors::TEXT_DIM, 11, false);
            y += 36;
        }
    }
    return y;
}

// ============================================================
// ПАНЕЛЬ СВОЙСТВ (главное отличие Этапа 3)
// ============================================================
void DrawProperties(HDC dc, HWND h) {
    RECT rc; GetClientRect(h, &rc);
    int left = rc.right - PROPS_W;
    RECT propRc = { left, TOPBAR_H, rc.right, rc.bottom - BOTTOM_H };
    FillRectC(dc, propRc, Colors::BG_DARK);

    g_editor.sliderCount = 0;
    g_editor.buttonCount = 0;

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

    Block* b = FindBlockById(g_editor.selectedId);
    if (!b) return;

    int y = TOPBAR_H + 70;
    int innerLeft = left + 15;
    int innerRight = rc.right - 15;

    // ===== Название блока =====
    RECT labelName = { innerLeft, y, innerRight, y + 20 };
    DrawTextLeft(dc, (g_language == 0) ? L"Название:" : L"Name:",
        labelName, Colors::TEXT_DIM, 11, false);
    y += 22;
    RECT valueName = { innerLeft, y, innerRight, y + 28 };
    FillRectC(dc, valueName, Colors::BG_PANEL);
    DrawTextLeft(dc, b->title.c_str(), valueName, Colors::TEXT, 13, false);
    y += 38;

    // ===== Тип =====
    RECT labelType = { innerLeft, y, innerRight, y + 20 };
    DrawTextLeft(dc, (g_language == 0) ? L"Тип:" : L"Type:",
        labelType, Colors::TEXT_DIM, 11, false);
    y += 22;
    RECT valueType = { innerLeft, y, innerRight, y + 28 };
    FillRectC(dc, valueType, Colors::BG_PANEL);
    const wchar_t* typeName = (g_language == 0) ? g_categories[b->type].ruName : g_categories[b->type].enName;
    DrawTextLeft(dc, typeName, valueType, Colors::TEXT, 13, false);
    y += 38;

    // ===== Параметры =====

    if (b->type == BT_GDI) {
        // --- GDI эффект ---
        RECT labelEff = { innerLeft, y, innerRight, y + 20 };
        DrawTextLeft(dc, (g_language == 0) ? L"Эффект:" : L"Effect:",
            labelEff, Colors::TEXT_DIM, 11, false);
        y += 22;

        RECT effBtn = { innerLeft, y, innerRight, y + 32 };
        g_editor.buttonRects[g_editor.buttonCount] = effBtn;
        g_editor.buttonCount++;
        bool hovered = false; // пока не наведено
        DrawCustomButton(dc, effBtn, g_gdiEffects[b->gdiEffect].ruName, hovered, false);
        y += 40;

        // Complexity
        RECT labelComplex = { innerLeft, y, innerRight, y + 18 };
        std::wstring complex = (g_language == 0) ? L"Сложность: " : L"Complexity: ";
        for (int i = 0; i < g_gdiEffects[b->gdiEffect].complexity; ++i) complex += L"★";
        DrawTextLeft(dc, complex.c_str(), labelComplex, Colors::TEXT_DIM, 11, false);
        y += 26;

        // Intensity
        RECT labelInt = { innerLeft, y, innerRight - 60, y + 20 };
        DrawTextLeft(dc, (g_language == 0) ? L"Интенсивность:" : L"Intensity:",
            labelInt, Colors::TEXT_DIM, 11, false);
        wchar_t intVal[16]; swprintf_s(intVal, L"%d%%", b->gdiIntensity);
        RECT valInt = { innerRight - 55, y, innerRight, y + 20 };
        DrawTextLeft(dc, intVal, valInt, Colors::TEXT, 12, true);
        y += 22;

        RECT sliderInt = { innerLeft, y, innerRight, y + 26 };
        g_editor.sliderRects[g_editor.sliderCount] = sliderInt;
        g_editor.sliderCount++;
        DrawSlider(dc, sliderInt, b->gdiIntensity, 100, false);
        y += 36;

        // Duration
        RECT labelDur = { innerLeft, y, innerRight - 60, y + 20 };
        DrawTextLeft(dc, (g_language == 0) ? L"Длительность:" : L"Duration:",
            labelDur, Colors::TEXT_DIM, 11, false);
        wchar_t durVal[16]; swprintf_s(durVal, L"%d сек", b->gdiDuration);
        RECT valDur = { innerRight - 80, y, innerRight, y + 20 };
        DrawTextLeft(dc, durVal, valDur, Colors::TEXT, 12, true);
        y += 22;

        RECT sliderDur = { innerLeft, y, innerRight, y + 26 };
        g_editor.sliderRects[g_editor.sliderCount] = sliderDur;
        g_editor.sliderCount++;
        DrawSlider(dc, sliderDur, b->gdiDuration, 60, false);
        y += 36;

    } else if (b->type == BT_BYTEBEAT) {
        RECT labelTrack = { innerLeft, y, innerRight, y + 20 };
        DrawTextLeft(dc, (g_language == 0) ? L"Трек:" : L"Track:",
            labelTrack, Colors::TEXT_DIM, 11, false);
        y += 22;

        RECT trackBtn = { innerLeft, y, innerRight, y + 32 };
        g_editor.buttonRects[g_editor.buttonCount] = trackBtn;
        g_editor.buttonCount++;
        DrawCustomButton(dc, trackBtn, g_byteTracks[b->bbTrack].name, false, false);
        y += 40;

        RECT labelVol = { innerLeft, y, innerRight - 60, y + 20 };
        DrawTextLeft(dc, (g_language == 0) ? L"Громкость:" : L"Volume:",
            labelVol, Colors::TEXT_DIM, 11, false);
        wchar_t volVal[16]; swprintf_s(volVal, L"%d%%", b->bbVolume);
        RECT valVol = { innerRight - 55, y, innerRight, y + 20 };
        DrawTextLeft(dc, volVal, valVol, Colors::TEXT, 12, true);
        y += 22;

        RECT sliderVol = { innerLeft, y, innerRight, y + 26 };
        g_editor.sliderRects[g_editor.sliderCount] = sliderVol;
        g_editor.sliderCount++;
        DrawSlider(dc, sliderVol, b->bbVolume, 100, false);
        y += 36;

    } else if (b->type == BT_MSGBOX) {
        // Текст
        RECT labelText = { innerLeft, y, innerRight, y + 20 };
        DrawTextLeft(dc, (g_language == 0) ? L"Текст:" : L"Text:",
            labelText, Colors::TEXT_DIM, 11, false);
        y += 22;
        RECT valueText = { innerLeft, y, innerRight, y + 40 };
        FillRectC(dc, valueText, Colors::BG_PANEL);
        DrawTextLeft(dc, b->msgText.c_str(), valueText, Colors::TEXT, 12, false);
        y += 48;

        // Иконка
        RECT labelIcon = { innerLeft, y, innerRight, y + 20 };
        DrawTextLeft(dc, (g_language == 0) ? L"Иконка:" : L"Icon:",
            labelIcon, Colors::TEXT_DIM, 11, false);
        y += 22;
        const wchar_t* iconNames[] = { L"Info", L"Warning", L"Error", L"Question" };
        RECT iconBtn = { innerLeft, y, innerRight, y + 28 };
        g_editor.buttonRects[g_editor.buttonCount] = iconBtn;
        g_editor.buttonCount++;
        DrawCustomButton(dc, iconBtn, iconNames[b->msgIcon], false, false);
        y += 36;

        // Количество окон
        RECT labelCount = { innerLeft, y, innerRight - 60, y + 20 };
        DrawTextLeft(dc, (g_language == 0) ? L"Количество:" : L"Count:",
            labelCount, Colors::TEXT_DIM, 11, false);
        wchar_t cntVal[16]; swprintf_s(cntVal, L"x%d", b->msgCount);
        RECT valCnt = { innerRight - 55, y, innerRight, y + 20 };
        DrawTextLeft(dc, cntVal, valCnt, Colors::TEXT, 12, true);
        y += 22;

        RECT sliderCnt = { innerLeft, y, innerRight, y + 26 };
        g_editor.sliderRects[g_editor.sliderCount] = sliderCnt;
        g_editor.sliderCount++;
        DrawSlider(dc, sliderCnt, b->msgCount, 30, false);
        y += 36;

        // Траектория
        RECT labelTraj = { innerLeft, y, innerRight, y + 20 };
        DrawTextLeft(dc, (g_language == 0) ? L"Траектория:" : L"Trajectory:",
            labelTraj, Colors::TEXT_DIM, 11, false);
        y += 22;

        const wchar_t* trajNames[] = {
            L"Круг вокруг курсора",
            L"Без курсора (центр)",
            L"Вертикально",
            L"Горизонтально",
            L"Случайно"
        };
        RECT trajBtn = { innerLeft, y, innerRight, y + 28 };
        g_editor.buttonRects[g_editor.buttonCount] = trajBtn;
        g_editor.buttonCount++;
        DrawCustomButton(dc, trajBtn, trajNames[b->msgTrajectory], false, false);
        y += 36;

        // Задержка
        RECT labelDelay = { innerLeft, y, innerRight - 60, y + 20 };
        DrawTextLeft(dc, (g_language == 0) ? L"Задержка (мс):" : L"Delay (ms):",
            labelDelay, Colors::TEXT_DIM, 11, false);
        wchar_t delayVal[16]; swprintf_s(delayVal, L"%d", b->msgDelay);
        RECT valDelay = { innerRight - 60, y, innerRight, y + 20 };
        DrawTextLeft(dc, delayVal, valDelay, Colors::TEXT, 12, true);
        y += 22;

        RECT sliderDelay = { innerLeft, y, innerRight, y + 26 };
        g_editor.sliderRects[g_editor.sliderCount] = sliderDelay;
        g_editor.sliderCount++;
        DrawSlider(dc, sliderDelay, b->msgDelay, 3000, false);
        y += 36;
    }

    // Позиция
    wchar_t posBuf[64];
    swprintf_s(posBuf, L"X: %d, Y: %d", b->x, b->y);
    RECT labelPos = { innerLeft, y, innerRight, y + 18 };
    DrawTextLeft(dc, posBuf, labelPos, Colors::TEXT_DIM, 10, false);
}

// ============================================================
// МОДАЛЬНОЕ ОКНО ВЫБОРА GDI-ЭФФЕКТА
// ============================================================
LRESULT CALLBACK EffectSelProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    static int hoverOk = 0, hoverCancel = 0;

    switch (m) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(h, &ps);
        RECT rc; GetClientRect(h, &rc);

        FillRectC(dc, rc, Colors::BG);

        // Заголовок
        RECT titleRc = { 20, 15, rc.right - 20, 50 };
        DrawTextLeft(dc, (g_language == 0) ? L"Выбор GDI эффекта" : L"Select GDI effect",
            titleRc, Colors::TEXT, 18, true);

        // Список
        RECT listRc = { 20, 60, rc.right - 20, rc.bottom - 70 };
        FillRectC(dc, listRc, Colors::BG_PANEL);

        HRGN hRgn = CreateRectRgn(listRc.left, listRc.top, listRc.right, listRc.bottom);
        SelectClipRgn(dc, hRgn);

        int itemH = 44;
        int startY = listRc.top - g_effectSel.scrollY;

        for (int i = 0; i < kGdiEffectCount; ++i) {
            RECT itemRc = { listRc.left, startY + i * itemH, listRc.right, startY + (i + 1) * itemH - 4 };
            if (itemRc.bottom < listRc.top || itemRc.top > listRc.bottom) continue;

            bool hovered = (i == g_effectSel.hovered);
            bool selected = (i == g_effectSel.selected);
            COLORREF bg = selected ? Colors::ACCENT : (hovered ? Colors::BG_HOVER : Colors::BG_PANEL);
            FillRectC(dc, itemRc, bg);

            // Название
            RECT nameRc = { itemRc.left + 15, itemRc.top, itemRc.right - 100, itemRc.bottom };
            DrawTextLeft(dc, g_gdiEffects[i].ruName, nameRc, Colors::TEXT, 14, true);

            // Сложность
            std::wstring stars;
            for (int j = 0; j < g_gdiEffects[i].complexity; ++j) stars += L"★";
            RECT starRc = { itemRc.right - 90, itemRc.top, itemRc.right - 10, itemRc.bottom };
            DrawTextLeft(dc, stars.c_str(), starRc, RGB(255, 200, 80), 14, false);
        }

        SelectClipRgn(dc, nullptr);
        DeleteObject(hRgn);

        // Кнопки
        RECT cancelBtn = { rc.right - 240, rc.bottom - 55, rc.right - 140, rc.bottom - 15 };
        RECT okBtn = { rc.right - 130, rc.bottom - 55, rc.right - 20, rc.bottom - 15 };
        DrawCustomButton(dc, cancelBtn,
            (g_language == 0) ? L"Отмена" : L"Cancel", hoverCancel, false);
        DrawCustomButton(dc, okBtn, L"OK", hoverOk, true);

        EndPaint(h, &ps);
        return 0;
    }

    case WM_MOUSEMOVE: {
        int x = LOWORD(l), y = HIWORD(l);
        RECT rc; GetClientRect(h, &rc);
        RECT listRc = { 20, 60, rc.right - 20, rc.bottom - 70 };
        RECT cancelBtn = { rc.right - 240, rc.bottom - 55, rc.right - 140, rc.bottom - 15 };
        RECT okBtn = { rc.right - 130, rc.bottom - 55, rc.right - 20, rc.bottom - 15 };

        auto inside = [](const RECT& r, int px, int py) {
            return px >= r.left && px <= r.right && py >= r.top && py <= r.bottom;
        };

        int nh = -1;
        if (inside(listRc, x, y)) {
            int itemH = 44;
            int relY = y - listRc.top + g_effectSel.scrollY;
            int idx = relY / itemH;
            if (idx >= 0 && idx < kGdiEffectCount) nh = idx;
        }

        int nOk = inside(okBtn, x, y);
        int nCancel = inside(cancelBtn, x, y);

        if (nh != g_effectSel.hovered || nOk != hoverOk || nCancel != hoverCancel) {
            g_effectSel.hovered = nh;
            hoverOk = nOk;
            hoverCancel = nCancel;
            InvalidateRect(h, nullptr, FALSE);
        }
        if (nh >= 0 || nOk || nCancel) SetCursor(LoadCursor(nullptr, IDC_HAND));
        else SetCursor(LoadCursor(nullptr, IDC_ARROW));
        return 0;
    }

    case WM_LBUTTONDOWN: {
        int x = LOWORD(l), y = HIWORD(l);
        RECT rc; GetClientRect(h, &rc);
        RECT listRc = { 20, 60, rc.right - 20, rc.bottom - 70 };
        RECT cancelBtn = { rc.right - 240, rc.bottom - 55, rc.right - 140, rc.bottom - 15 };
        RECT okBtn = { rc.right - 130, rc.bottom - 55, rc.right - 20, rc.bottom - 15 };

        auto inside = [](const RECT& r, int px, int py) {
            return px >= r.left && px <= r.right && py >= r.top && py <= r.bottom;
        };

        if (inside(listRc, x, y)) {
            int itemH = 44;
            int relY = y - listRc.top + g_effectSel.scrollY;
            int idx = relY / itemH;
            if (idx >= 0 && idx < kGdiEffectCount) {
                g_effectSel.selected = idx;
                InvalidateRect(h, nullptr, FALSE);
            }
            return 0;
        }
        if (inside(okBtn, x, y)) {
            if (g_effectSel.selected >= 0) {
                Block* b = FindBlockById(g_effectSel.forBlockId);
                if (b) {
                    b->gdiEffect = g_effectSel.selected;
                    g_editor.modified = true;
                }
            }
            DestroyWindow(h);
            return 0;
        }
        if (inside(cancelBtn, x, y)) { DestroyWindow(h); return 0; }
        return 0;
    }

    case WM_MOUSEWHEEL: {
        int delta = GET_WHEEL_DELTA_WPARAM(w);
        g_effectSel.scrollY -= delta / 2;
        if (g_effectSel.scrollY < 0) g_effectSel.scrollY = 0;
        int maxScroll = kGdiEffectCount * 44 - (int)(GetClientRect(h, nullptr), 400);
        if (g_effectSel.scrollY > maxScroll) g_effectSel.scrollY = maxScroll;
        InvalidateRect(h, nullptr, FALSE);
        return 0;
    }

    case WM_CLOSE:
        DestroyWindow(h);
        return 0;
    }
    return DefWindowProcW(h, m, w, l);
}

void OpenEffectSelector(HWND parent, int blockId, int currentEffect) {
    g_effectSel.parent = parent;
    g_effectSel.scrollY = 0;
    g_effectSel.hovered = -1;
    g_effectSel.selected = currentEffect;
    g_effectSel.forBlockId = blockId;

    int w = 500, h = 600;
    int px = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    int py = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

    HWND dlg = CreateWindowExW(WS_EX_DLGMODALFRAME,
        CLASS_EFFECTSEL, L"GDI эффекты",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        px, py, w, h, parent, nullptr, g_hInst, nullptr);

    ShowWindow(dlg, SW_SHOW);
    UpdateWindow(dlg);
}

// ============================================================
// EDITOR WINDOW
// ============================================================
LRESULT CALLBACK EditorProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    static int hoverSave = 0, hoverPreview = 0, hoverBuild = 0;

    switch (m) {
    case WM_CREATE:
        g_editorWnd = h;
        SetTimer(h, 1, 30000, nullptr); // автосейв каждые 30 сек
        return 0;

    case WM_TIMER:
        if (w == 1) {
            if (g_editor.modified) SaveProject();
        }
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(h, &ps);
        RECT rc; GetClientRect(h, &rc);

        FillRectC(dc, rc, Colors::BG);

        // Топбар
        RECT topbar = { 0, 0, rc.right, TOPBAR_H };
        FillRectC(dc, topbar, Colors::BG_DARK);

        std::wstring title = L"⚒ PrankForge — " + g_editor.projectName;
        if (g_editor.modified) title += L" ●";
        RECT titleRc = { 15, 0, rc.right - 300, TOPBAR_H };
        DrawTextLeft(dc, title.c_str(), titleRc, Colors::TEXT, 15, true);

        RECT saveBtn = { rc.right - 280, 8, rc.right - 180, TOPBAR_H - 8 };
        DrawCustomButton(dc, saveBtn,
            (g_language == 0) ? L"💾 Сохранить" : L"💾 Save", hoverSave, false);

        // Сайдбар
        RECT sidebar = { 0, TOPBAR_H, SIDEBAR_W, rc.bottom - BOTTOM_H };
        FillRectC(dc, sidebar, Colors::BG_DARK);

        RECT sideTitle = { 15, TOPBAR_H + 10, SIDEBAR_W - 15, TOPBAR_H + 40 };
        DrawTextLeft(dc, (g_language == 0) ? L"Палитра блоков" : L"Block palette",
            sideTitle, Colors::TEXT, 14, true);

        DrawPalette(dc, TOPBAR_H + 50, 0, SIDEBAR_W);

        // Канвас
        RECT canvas = { SIDEBAR_W, TOPBAR_H, rc.right - PROPS_W, rc.bottom - BOTTOM_H };
        FillRectC(dc, canvas, Colors::BG_CANVAS);

        HRGN hRgn = CreateRectRgn(canvas.left, canvas.top, canvas.right, canvas.bottom);
        SelectClipRgn(dc, hRgn);

        DrawGrid(dc, canvas);
        DrawConnections(dc);
        for (auto& b : g_editor.blocks) DrawBlock(dc, b);

        SelectClipRgn(dc, nullptr);
        DeleteObject(hRgn);

        // Свойства
        DrawProperties(dc, h);

        // Нижняя панель
        RECT bottom = { 0, rc.bottom - BOTTOM_H, rc.right, rc.bottom };
        FillRectC(dc, bottom, Colors::BG_DARK);

        wchar_t infoBuf[128];
        swprintf_s(infoBuf, L"Блоков: %d    Zoom: %.0f%%",
            (int)g_editor.blocks.size(), g_editor.zoom * 100.0f);
        RECT infoRc = { 15, 0, rc.right - 500, BOTTOM_H };
        DrawTextLeft(dc, infoBuf, infoRc, Colors::TEXT_DIM, 12, false);

        RECT previewBtn = { rc.right - 480, 12, rc.right - 260, BOTTOM_H - 12 };
        RECT buildBtn = { rc.right - 250, 12, rc.right - 15, BOTTOM_H - 12 };
        DrawCustomButton(dc, previewBtn,
            (g_language == 0) ? L"▶ Предпросмотр" : L"▶ Preview", hoverPreview, false);
        DrawCustomButton(dc, buildBtn,
            (g_language == 0) ? L"🔨 Собрать EXE" : L"🔨 Build EXE", hoverBuild, true);

        EndPaint(h, &ps);
        return 0;
    }

    case WM_MOUSEMOVE: {
        int x = LOWORD(l), y = HIWORD(l);
        RECT rc; GetClientRect(h, &rc);

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

        // Hover палитры
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

        // Активный слайдер — обновляем значение
        if (g_editor.activeSlider >= 0 && g_editor.selectedId >= 0) {
            Block* b = FindBlockById(g_editor.selectedId);
            if (b && g_editor.activeSlider < g_editor.sliderCount) {
                RECT sr = g_editor.sliderRects[g_editor.activeSlider];
                int val = (x - sr.left) * 100 / (sr.right - sr.left);
                if (val < 0) val = 0;
                if (val > 100) val = 100;

                // Определяем, к какому параметру относится слайдер
                if (b->type == BT_GDI) {
                    if (g_editor.activeSlider == 0) b->gdiIntensity = val;
                    else if (g_editor.activeSlider == 1) b->gdiDuration = val * 60 / 100;
                } else if (b->type == BT_BYTEBEAT) {
                    if (g_editor.activeSlider == 0) b->bbVolume = val;
                } else if (b->type == BT_MSGBOX) {
                    if (g_editor.activeSlider == 0) b->msgCount = 1 + val * 29 / 100;
                    else if (g_editor.activeSlider == 1) b->msgDelay = val * 30;
                }
                g_editor.modified = true;
                changed = 1;
            }
        }

        // Drag блока
        if (g_editor.dragging && g_editor.dragBlockId >= 0) {
            POINT p = ScreenToCanvas(x, y);
            Block* b = FindBlockById(g_editor.dragBlockId);
            if (b) {
                b->x = p.x - g_editor.dragOffsetX;
                b->y = p.y - g_editor.dragOffsetY;
                g_editor.modified = true;
                InvalidateRect(h, nullptr, FALSE);
                return 0;
            }
        }

        if (changed) InvalidateRect(h, nullptr, FALSE);

        if (np || nb || ns) SetCursor(LoadCursor(nullptr, IDC_HAND));
        else if (g_editor.activeSlider >= 0) SetCursor(LoadCursor(nullptr, IDC_HAND));
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

        RECT previewBtn = { rc.right - 480, 12, rc.right - 260, BOTTOM_H - 12 };
        RECT buildBtn = { rc.right - 250, 12, rc.right - 15, BOTTOM_H - 12 };
        RECT saveBtn = { rc.right - 280, 8, rc.right - 180, TOPBAR_H - 8 };

        auto inside = [](const RECT& r, int px, int py) {
            return px >= r.left && px <= r.right && py >= r.top && py <= r.bottom;
        };

        if (inside(saveBtn, x, y)) {
            if (SaveProject())
                MessageBoxW(h, (g_language == 0) ? L"Сохранено." : L"Saved.",
                    L"PrankForge", MB_OK);
            else
                MessageBoxW(h, L"Ошибка сохранения.", L"PrankForge", MB_OK | MB_ICONERROR);
            InvalidateRect(h, nullptr, TRUE);
            return 0;
        }
        if (inside(previewBtn, x, y)) {
            MessageBoxW(h, L"Предпросмотр — Этап 5.", L"PrankForge", MB_OK);
            return 0;
        }
        if (inside(buildBtn, x, y)) {
            MessageBoxW(h, L"Сборка — Этап 5.", L"PrankForge", MB_OK);
            return 0;
        }

        // Клик по палитре
        if (x < SIDEBAR_W && y > TOPBAR_H + 50) {
            int palTop = TOPBAR_H + 50;
            int yy = palTop + 10;
            for (int i = 0; i < BT_COUNT; ++i) {
                RECT itemRc = { 5, yy, SIDEBAR_W - 5, yy + 36 };
                if (inside(itemRc, x, y)) {
                    g_editor.expandedCategory = (g_editor.expandedCategory == i) ? -1 : i;
                    if (g_editor.expandedCategory == i) {
                        Block b;
                        b.id = g_editor.nextId++;
                        b.type = (BlockType)i;
                        const wchar_t* name = (g_language == 0)
                            ? g_categories[i].ruName : g_categories[i].enName;
                        b.title = name;
                        int cx = (GetCanvasLeft() + GetCanvasRight(h)) / 2;
                        int cy = (GetCanvasTop() + GetCanvasBottom(h)) / 2;
                        POINT canvasPt = ScreenToCanvas(cx, cy);
                        b.x = canvasPt.x - b.w / 2 + (int)(g_editor.blocks.size() % 5) * 30;
                        b.y = canvasPt.y - b.h / 2 + (int)(g_editor.blocks.size() % 5) * 30;
                        g_editor.blocks.push_back(b);
                        g_editor.selectedId = b.id;
                        g_editor.modified = true;
                    }
                    InvalidateRect(h, nullptr, TRUE);
                    return 0;
                }
                yy += 40;
                if (i == g_editor.expandedCategory) yy += 36;
            }
        }

        // Клик по свойствам
        if (x > rc.right - PROPS_W && y > TOPBAR_H) {
            Block* b = FindBlockById(g_editor.selectedId);
            if (b) {
                // Кнопки
                for (int i = 0; i < g_editor.buttonCount; ++i) {
                    if (inside(g_editor.buttonRects[i], x, y)) {
                        if (b->type == BT_GDI && i == 0) {
                            OpenEffectSelector(h, b->id, b->gdiEffect);
                            return 0;
                        }
                        if (b->type == BT_BYTEBEAT && i == 0) {
                            b->bbTrack = (b->bbTrack + 1) % kByteTrackCount;
                            g_editor.modified = true;
                            InvalidateRect(h, nullptr, TRUE);
                            return 0;
                        }
                        if (b->type == BT_MSGBOX) {
                            if (i == 0) { b->msgIcon = (b->msgIcon + 1) % 4; }
                            else if (i == 1) { b->msgTrajectory = (b->msgTrajectory + 1) % 5; }
                            g_editor.modified = true;
                            InvalidateRect(h, nullptr, TRUE);
                            return 0;
                        }
                    }
                }
                // Слайдеры
                for (int i = 0; i < g_editor.sliderCount; ++i) {
                    if (inside(g_editor.sliderRects[i], x, y)) {
                        g_editor.activeSlider = i;
                        // Сразу обновить
                        RECT sr = g_editor.sliderRects[i];
                        int val = (x - sr.left) * 100 / (sr.right - sr.left);
                        if (val < 0) val = 0; if (val > 100) val = 100;
                        if (b->type == BT_GDI) {
                            if (i == 0) b->gdiIntensity = val;
                            else if (i == 1) b->gdiDuration = val * 60 / 100;
                        } else if (b->type == BT_BYTEBEAT && i == 0) {
                            b->bbVolume = val;
                        } else if (b->type == BT_MSGBOX) {
                            if (i == 0) b->msgCount = 1 + val * 29 / 100;
                            else if (i == 1) b->msgDelay = val * 30;
                        }
                        g_editor.modified = true;
                        InvalidateRect(h, nullptr, TRUE);
                        return 0;
                    }
                }
            }
            return 0;
        }

        // Клик по канвасу
        if (x > SIDEBAR_W && x < rc.right - PROPS_W && y > TOPBAR_H && y < rc.bottom - BOTTOM_H) {
            Block* b = FindBlockAtScreen(x, y);
            if (b) {
                g_editor.dragging = true;
                g_editor.dragBlockId = b->id;
                POINT canvasPt = ScreenToCanvas(x, y);
                g_editor.dragOffsetX = canvasPt.x - b->x;
                g_editor.dragOffsetY = canvasPt.y - b->y;
                for (auto& bl : g_editor.blocks) bl.selected = false;
                b->selected = true;
                g_editor.selectedId = b->id;
            } else {
                for (auto& bl : g_editor.blocks) bl.selected = false;
                g_editor.selectedId = -1;
            }
            InvalidateRect(h, nullptr, TRUE);
            return 0;
        }
        return 0;
    }

    case WM_LBUTTONUP:
        g_editor.dragging = false;
        g_editor.dragBlockId = -1;
        g_editor.activeSlider = -1;
        return 0;

    case WM_MOUSEWHEEL: {
        int delta = GET_WHEEL_DELTA_WPARAM(w);
        bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
        if (ctrl) {
            if (delta > 0) g_editor.zoom *= 1.1f; else g_editor.zoom /= 1.1f;
            if (g_editor.zoom < 0.25f) g_editor.zoom = 0.25f;
            if (g_editor.zoom > 3.0f) g_editor.zoom = 3.0f;
        } else {
            g_editor.scrollY -= delta / 3;
            if (g_editor.scrollY < 0) g_editor.scrollY = 0;
        }
        InvalidateRect(h, nullptr, TRUE);
        return 0;
    }

    case WM_KEYDOWN: {
        bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
        if (ctrl && w == 'S') {
            if (SaveProject())
                MessageBoxW(h, (g_language == 0) ? L"Сохранено." : L"Saved.",
                    L"PrankForge", MB_OK);
            InvalidateRect(h, nullptr, TRUE);
            return 0;
        }
        if (w == VK_DELETE && g_editor.selectedId >= 0) {
            for (auto it = g_editor.blocks.begin(); it != g_editor.blocks.end(); ++it) {
                if (it->id == g_editor.selectedId) {
                    g_editor.blocks.erase(it);
                    g_editor.selectedId = -1;
                    g_editor.modified = true;
                    InvalidateRect(h, nullptr, TRUE);
                    break;
                }
            }
            return 0;
        }
        return 0;
    }

    case WM_DESTROY:
        if (g_editor.modified) SaveProject();
        g_editorWnd = nullptr;
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
    g_editor.modified = false;

    RECT rc = { 0, 0, 1400, 800 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;
    int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

    HWND editor = CreateWindowExW(0, CLASS_EDITOR,
        (L"PrankForge — " + g_editor.projectName).c_str(),
        WS_OVERLAPPEDWINDOW, x, y, w, h,
        parent, nullptr, g_hInst, nullptr);

    ShowWindow(editor, SW_SHOW);
    UpdateWindow(editor);
}

// ============================================================
// WELCOME / MAIN / CREATE (без изменений, но короче)
// ============================================================
LRESULT CALLBACK WelcomeProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    static int hoverStart = 0, hoverRu = 0, hoverEn = 0;
    static int hoverDoc = 0, hoverAbout = 0, hoverExit = 0;
    switch (m) {
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC dc = BeginPaint(h, &ps);
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
        DrawTextCentered(dc, (g_language == 0)
            ? L"Добро пожаловать в конструктор пранков"
            : L"Welcome to the prank builder",
            wellRc, Colors::TEXT, 22, false);
        RECT descRc = { 60, 310, rc.right - 60, 400 };
        const wchar_t* desc = (g_language == 0)
            ? L"Собирайте пранки из блоков, как в конструкторе."
            : L"Build pranks from blocks like a constructor.";
        SetBkMode(dc, TRANSPARENT); SetTextColor(dc, Colors::TEXT_DIM);
        HFONT f = CreateFontW(15, 0, 0, 0, FW_NORMAL, 0, 0, 0,
            DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
        HFONT old = (HFONT)SelectObject(dc, f);
        DrawTextW(dc, desc, -1, &descRc, DT_CENTER | DT_TOP | DT_WORDBREAK);
        SelectObject(dc, old); DeleteObject(f);
        RECT startBtn = { rc.right / 2 - 100, 430, rc.right / 2 + 100, 480 };
        DrawCustomButton(dc, startBtn,
            (g_language == 0) ? L"Начать" : L"Start", hoverStart, true);
        RECT ruBtn = { 30, rc.bottom - 60, 110, rc.bottom - 30 };
        RECT enBtn = { 120, rc.bottom - 60, 200, rc.bottom - 30 };
        DrawCustomButton(dc, ruBtn, L"Русский", hoverRu, g_language == 0);
        DrawCustomButton(dc, enBtn, L"English", hoverEn, g_language == 1);
        RECT docBtn = { 240, rc.bottom - 60, 360, rc.bottom - 30 };
        RECT aboutBtn = { 370, rc.bottom - 60, 490, rc.bottom - 30 };
        DrawCustomButton(dc, docBtn, (g_language == 0) ? L"Документация" : L"Docs", hoverDoc, false);
        DrawCustomButton(dc, aboutBtn, (g_language == 0) ? L"О программе" : L"About", hoverAbout, false);
        RECT verRc = { rc.right - 120, rc.bottom - 40, rc.right - 10, rc.bottom - 10 };
        DrawTextCentered(dc, L"v1.0.0", verRc, Colors::TEXT_DIM, 12, false);
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
        SelectObject(dc, oldPen); DeleteObject(pen);
        EndPaint(h, &ps);
        return 0;
    }
    case WM_MOUSEMOVE: {
        int x = LOWORD(l), y = HIWORD(l); RECT rc; GetClientRect(h, &rc);
        RECT startBtn = { rc.right / 2 - 100, 430, rc.right / 2 + 100, 480 };
        RECT ruBtn = { 30, rc.bottom - 60, 110, rc.bottom - 30 };
        RECT enBtn = { 120, rc.bottom - 60, 200, rc.bottom - 30 };
        RECT docBtn = { 240, rc.bottom - 60, 360, rc.bottom - 30 };
        RECT aboutBtn = { 370, rc.bottom - 60, 490, rc.bottom - 30 };
        RECT closeBtn = { rc.right - 40, 0, rc.right, 40 };
        auto inside = [](const RECT& r, int px, int py) {
            return px >= r.left && px <= r.right && py >= r.top && py <= r.bottom; };
        int ns=inside(startBtn,x,y),nr=inside(ruBtn,x,y),ne=inside(enBtn,x,y);
        int nd=inside(docBtn,x,y),na=inside(aboutBtn,x,y),nc=inside(closeBtn,x,y);
        if (ns!=hoverStart||nr!=hoverRu||ne!=hoverEn||nd!=hoverDoc||na!=hoverAbout||nc!=hoverExit) {
            hoverStart=ns;hoverRu=nr;hoverEn=ne;hoverDoc=nd;hoverAbout=na;hoverExit=nc;
            InvalidateRect(h,nullptr,FALSE);
        }
        if (ns||nr||ne||nd||na||nc) SetCursor(LoadCursor(nullptr,IDC_HAND));
        else SetCursor(LoadCursor(nullptr,IDC_ARROW));
        return 0;
    }
    case WM_LBUTTONDOWN: {
        int x = LOWORD(l), y = HIWORD(l); RECT rc; GetClientRect(h, &rc);
        RECT startBtn = { rc.right / 2 - 100, 430, rc.right / 2 + 100, 480 };
        RECT ruBtn = { 30, rc.bottom - 60, 110, rc.bottom - 30 };
        RECT enBtn = { 120, rc.bottom - 60, 200, rc.bottom - 30 };
        RECT closeBtn = { rc.right - 40, 0, rc.right, 40 };
        auto inside = [](const RECT& r, int px, int py) {
            return px >= r.left && px <= r.right && py >= r.top && py <= r.bottom; };
        if (inside(startBtn,x,y)) { DestroyWindow(h); extern void OpenMainWindow(); OpenMainWindow(); return 0; }
        if (inside(ruBtn,x,y)) { g_language = 0; InvalidateRect(h,nullptr,TRUE); return 0; }
        if (inside(enBtn,x,y)) { g_language = 1; InvalidateRect(h,nullptr,TRUE); return 0; }
        if (inside(closeBtn,x,y)) { PostQuitMessage(0); return 0; }
        return 0;
    }
    case WM_DESTROY: if (!g_mainWnd) PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(h, m, w, l);
}

HWND g_createEditName = nullptr;
HWND g_createEditPath = nullptr;

LRESULT CALLBACK CreateProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    static int hoverBrowse = 0, hoverOk = 0, hoverCancel = 0;
    switch (m) {
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC dc = BeginPaint(h, &ps);
        RECT rc; GetClientRect(h, &rc);
        FillRectC(dc, rc, Colors::BG);
        RECT titleRc = { 20, 15, rc.right - 20, 50 };
        DrawTextLeft(dc, (g_language == 0) ? L"Создание нового проекта" : L"Create new project",
            titleRc, Colors::TEXT, 20, true);
        RECT label1 = { 20, 70, rc.right - 20, 95 };
        DrawTextLeft(dc, (g_language == 0) ? L"Название:" : L"Name:", label1, Colors::TEXT_DIM, 14, false);
        RECT label2 = { 20, 140, rc.right - 20, 165 };
        DrawTextLeft(dc, (g_language == 0) ? L"Расположение:" : L"Location:", label2, Colors::TEXT_DIM, 14, false);
        RECT browseBtn = { rc.right - 120, 165, rc.right - 20, 200 };
        DrawCustomButton(dc, browseBtn, L"...", hoverBrowse, false);
        RECT cancelBtn = { rc.right - 240, rc.bottom - 60, rc.right - 140, rc.bottom - 20 };
        RECT okBtn = { rc.right - 130, rc.bottom - 60, rc.right - 20, rc.bottom - 20 };
        DrawCustomButton(dc, cancelBtn, (g_language == 0) ? L"Отмена" : L"Cancel", hoverCancel, false);
        DrawCustomButton(dc, okBtn, (g_language == 0) ? L"Создать" : L"Create", hoverOk, true);
        EndPaint(h, &ps);
        return 0;
    }
    case WM_CTLCOLOREDIT: {
        HDC dc = (HDC)w;
        SetTextColor(dc, Colors::TEXT); SetBkColor(dc, Colors::BG_PANEL);
        static HBRUSH hbr = CreateSolidBrush(Colors::BG_PANEL);
        return (LRESULT)hbr;
    }
    case WM_CREATE: {
        RECT rc; GetClientRect(h, &rc);
        g_createEditName = CreateWindowExW(0, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 20, 100, 560, 32, h, nullptr, g_hInst, nullptr);
        g_createEditPath = CreateWindowExW(0, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 20, 170, 440, 32, h, nullptr, g_hInst, nullptr);
        HFONT editFont = CreateFontW(15, 0, 0, 0, FW_NORMAL, 0, 0, 0,
            DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
        SendMessageW(g_createEditName, WM_SETFONT, (WPARAM)editFont, TRUE);
        SendMessageW(g_createEditPath, WM_SETFONT, (WPARAM)editFont, TRUE);
        return 0;
    }
    case WM_MOUSEMOVE: {
        int x = LOWORD(l), y = HIWORD(l); RECT rc; GetClientRect(h, &rc);
        RECT browseBtn = { rc.right - 120, 165, rc.right - 20, 200 };
        RECT cancelBtn = { rc.right - 240, rc.bottom - 60, rc.right - 140, rc.bottom - 20 };
        RECT okBtn = { rc.right - 130, rc.bottom - 60, rc.right - 20, rc.bottom - 20 };
        auto inside = [](const RECT& r, int px, int py) {
            return px >= r.left && px <= r.right && py >= r.top && py <= r.bottom; };
        int nb=inside(browseBtn,x,y),nc=inside(cancelBtn,x,y),no=inside(okBtn,x,y);
        if (nb!=hoverBrowse||nc!=hoverCancel||no!=hoverOk) {
            hoverBrowse=nb;hoverCancel=nc;hoverOk=no; InvalidateRect(h,nullptr,FALSE);
        }
        if (nb||nc||no) SetCursor(LoadCursor(nullptr,IDC_HAND));
        else SetCursor(LoadCursor(nullptr,IDC_ARROW));
        return 0;
    }
    case WM_LBUTTONDOWN: {
        int x = LOWORD(l), y = HIWORD(l); RECT rc; GetClientRect(h, &rc);
        RECT browseBtn = { rc.right - 120, 165, rc.right - 20, 200 };
        RECT cancelBtn = { rc.right - 240, rc.bottom - 60, rc.right - 140, rc.bottom - 20 };
        RECT okBtn = { rc.right - 130, rc.bottom - 60, rc.right - 20, rc.bottom - 20 };
        auto inside = [](const RECT& r, int px, int py) {
            return px >= r.left && px <= r.right && py >= r.top && py <= r.bottom; };
        if (inside(browseBtn,x,y)) {
            BROWSEINFOW bi = {}; bi.hwndOwner = h;
            bi.lpszTitle = L"Выбери папку"; bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
            LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
            if (pidl) { wchar_t p[MAX_PATH]; SHGetPathFromIDListW(pidl, p);
                SetWindowTextW(g_createEditPath, p); CoTaskMemFree(pidl); }
            return 0;
        }
        if (inside(okBtn,x,y)) {
            wchar_t name[256] = {}; wchar_t path[MAX_PATH] = {};
            GetWindowTextW(g_createEditName, name, 256);
            GetWindowTextW(g_createEditPath, path, MAX_PATH);
            if (!wcslen(name)) { MessageBoxW(h, L"Введи название.", L"!", MB_OK); return 0; }
            wchar_t projDir[MAX_PATH]; swprintf_s(projDir, L"%s\\%s", path, name);
            CreateDirectoryW(projDir, nullptr);
            wchar_t projFile[MAX_PATH]; swprintf_s(projFile, L"%s\\project.pforge", projDir);
            HANDLE f = CreateFileW(projFile, GENERIC_WRITE, 0, nullptr,
                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (f != INVALID_HANDLE_VALUE) {
                const char* j = "{\n  \"name\": \"\",\n  \"blocks\": []\n}\n";
                DWORD wr; WriteFile(f, j, (DWORD)strlen(j), &wr, nullptr); CloseHandle(f);
            }
            ProjectInfo pi;
            pi.name = name; pi.path = projDir;
            SYSTEMTIME st; GetLocalTime(&st);
            wchar_t d[64]; swprintf_s(d, L"%04d-%02d-%02d %02d:%02d",
                st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
            pi.date = d;
            g_projects.push_back(pi);
            if (g_mainWnd) InvalidateRect(g_mainWnd, nullptr, TRUE);
            DestroyWindow(h);
            return 0;
        }
        if (inside(cancelBtn,x,y)) { DestroyWindow(h); return 0; }
        return 0;
    }
    case WM_CLOSE: DestroyWindow(h); return 0;
    }
    return DefWindowProcW(h, m, w, l);
}

LRESULT CALLBACK MainProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    static int hoverCreate = 0, hoverSettings = 0;
    static int hoveredProject = -1;
    switch (m) {
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC dc = BeginPaint(h, &ps);
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
        DrawCustomButton(dc, settingsBtn,
            (g_language == 0) ? L"⚙ Настройки" : L"⚙ Settings", hoverSettings, false);
        RECT topbar = { 250, 0, rc.right, 70 };
        FillRectC(dc, topbar, Colors::BG_DARK);
        RECT titleRc = { 270, 15, 500, 55 };
        DrawTextLeft(dc, (g_language == 0) ? L"Мои проекты" : L"My projects",
            titleRc, Colors::TEXT, 22, true);
        RECT createBtn = { rc.right - 280, 15, rc.right - 20, 55 };
        DrawCustomButton(dc, createBtn,
            (g_language == 0) ? L"+ Создать проект" : L"+ Create project", hoverCreate, true);

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
                FrameRect(dc, &card, border); DeleteObject(border);
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
        int x = LOWORD(l), y = HIWORD(l); RECT rc; GetClientRect(h, &rc);
        RECT createBtn = { rc.right - 280, 15, rc.right - 20, 55 };
        RECT settingsBtn = { 10, rc.bottom - 60, 240, rc.bottom - 15 };
        auto inside = [](const RECT& r, int px, int py) {
            return px >= r.left && px <= r.right && py >= r.top && py <= r.bottom; };
        int nc = inside(createBtn,x,y), ns = inside(settingsBtn,x,y);
        int hp = -1;
        if (x > 250) {
            int yy = 100;
            for (size_t i = 0; i < g_projects.size(); ++i) {
                RECT card = { 270, yy, rc.right - 20, yy + 90 };
                if (inside(card,x,y)) { hp = (int)i; break; }
                yy += 100;
            }
        }
        if (nc!=hoverCreate||ns!=hoverSettings||hp!=hoveredProject) {
            hoverCreate=nc;hoverSettings=ns;hoveredProject=hp;
            InvalidateRect(h,nullptr,FALSE);
        }
        if (nc||ns||hp>=0) SetCursor(LoadCursor(nullptr,IDC_HAND));
        else SetCursor(LoadCursor(nullptr,IDC_ARROW));
        return 0;
    }
    case WM_LBUTTONDOWN: {
        int x = LOWORD(l), y = HIWORD(l); RECT rc; GetClientRect(h, &rc);
        RECT createBtn = { rc.right - 280, 15, rc.right - 20, 55 };
        auto inside = [](const RECT& r, int px, int py) {
            return px >= r.left && px <= r.right && py >= r.top && py <= r.bottom; };
        if (inside(createBtn,x,y)) {
            RECT dlg = { 0, 0, 600, 320 };
            AdjustWindowRect(&dlg, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME, FALSE);
            int dw = dlg.right - dlg.left, dh = dlg.bottom - dlg.top;
            int px = (GetSystemMetrics(SM_CXSCREEN)-dw)/2;
            int py = (GetSystemMetrics(SM_CYSCREEN)-dh)/2;
            CreateWindowExW(WS_EX_DLGMODALFRAME, CLASS_CREATE, L"PrankForge — Новый проект",
                WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, px, py, 600, 320, h, nullptr, g_hInst, nullptr);
            return 0;
        }
        return 0;
    }
    case WM_LBUTTONDBLCLK: {
        int x = LOWORD(l), y = HIWORD(l); RECT rc; GetClientRect(h, &rc);
        if (x > 250) {
            int yy = 100;
            for (size_t i = 0; i < g_projects.size(); ++i) {
                RECT card = { 270, yy, rc.right - 20, yy + 90 };
                if (x >= card.left && x <= card.right && y >= card.top && y <= card.bottom) {
                    OpenEditor(h, (int)i); return 0;
                }
                yy += 100;
            }
        }
        return 0;
    }
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(h, m, w, l);
}

void OpenMainWindow() {
    RECT rc = { 0, 0, 1280, 720 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    int w = rc.right - rc.left, h = rc.bottom - rc.top;
    int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;
    g_mainWnd = CreateWindowExW(0, CLASS_MAIN, L"PrankForge",
        WS_OVERLAPPEDWINDOW, x, y, w, h, nullptr, nullptr, g_hInst, nullptr);
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

    wc.lpfnWndProc = WelcomeProc; wc.lpszClassName = CLASS_WELCOME; wc.style = CS_DBLCLKS; RegisterClassW(&wc);
    wc.lpfnWndProc = MainProc;    wc.lpszClassName = CLASS_MAIN;    wc.style = CS_DBLCLKS; RegisterClassW(&wc);
    wc.lpfnWndProc = CreateProc;  wc.lpszClassName = CLASS_CREATE;  wc.style = 0;          RegisterClassW(&wc);
    wc.lpfnWndProc = EditorProc;  wc.lpszClassName = CLASS_EDITOR;  wc.style = CS_DBLCLKS; RegisterClassW(&wc);
    wc.lpfnWndProc = EffectSelProc; wc.lpszClassName = CLASS_EFFECTSEL; wc.style = 0;     RegisterClassW(&wc);
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
    int w = rc.right - rc.left, h = rc.bottom - rc.top;
    int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

    HWND welcome = CreateWindowExW(0, CLASS_WELCOME, L"PrankForge — Добро пожаловать",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, x, y, 700, 540,
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
