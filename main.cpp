#pragma execution_character_set("utf-8")
#define _WIN32_WINNT 0x0601
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <commdlg.h>
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
#pragma comment(lib, "comdlg32.lib")
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
    const COLORREF WARN      = RGB(255, 180, 60);
    const COLORREF CHECK_ON  = RGB(255, 107, 53);
    const COLORREF CHECK_OFF = RGB(80, 80, 80);

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
// БИБЛИОТЕКИ
// ============================================================
struct GdiEffect { const wchar_t* ruName; int complexity; };
GdiEffect g_gdiEffects[] = {
    { L"Screen Bend", 2 }, { L"Ribbon", 1 }, { L"Screen Tear", 1 },
    { L"Zoom Blur", 2 }, { L"Kaleidoscope", 3 }, { L"Screen Rotate", 2 },
    { L"Water Ripple", 3 }, { L"Squeeze", 2 }, { L"Tunnel", 3 },
    { L"Invert", 1 }, { L"Grayscale", 1 }, { L"Red Filter", 1 },
    { L"Magenta Filter", 1 }, { L"Rainbow", 1 }, { L"Threshold", 2 },
    { L"RGB Split", 1 }, { L"Glitch Blocks", 1 }, { L"Stretch Blocks", 1 },
    { L"Scanlines", 1 }, { L"TV Noise", 1 }, { L"Matrix Rain", 2 },
    { L"Pixelation", 1 }, { L"Shake", 1 }, { L"Drip", 3 },
    { L"Holes", 1 }, { L"Cursor Trail", 2 }
};
constexpr int kGdiEffectCount = 26;

struct ByteTrack { const wchar_t* name; };
ByteTrack g_byteTracks[] = {
    { L"Quarknova" }, { L"Monoxide" }, { L"Synth Progression" },
    { L"Ambient Low" }, { L"Bass Drop" }, { L"Glitch Noise" },
    { L"Nyan Bytebeat" }, { L"Horror Drone" }, { L"Alarm Siren" },
    { L"Digital Rain" }
};
constexpr int kByteTrackCount = 10;

struct WpColor { const wchar_t* name; COLORREF color; };
WpColor g_wpColors[] = {
    { L"Чёрный",   RGB(0,0,0) },
    { L"Белый",    RGB(255,255,255) },
    { L"Красный",  RGB(200,30,30) },
    { L"Синий",    RGB(30,80,200) },
    { L"Зелёный",  RGB(30,160,50) },
    { L"Оранжевый",RGB(255,120,30) },
    { L"Жёлтый",   RGB(240,210,40) },
    { L"Фиолет.",  RGB(150,50,200) },
    { L"Розовый",  RGB(230,110,180) },
    { L"Бирюзов.", RGB(60,190,200) },
    { L"Серый",    RGB(100,100,100) },
    { L"Тёмный",   RGB(25,25,35) }
};
constexpr int kWpColorCount = 12;

// ============================================================
// ГЛОБАЛЫ
// ============================================================
HINSTANCE g_hInst = nullptr;
int g_language = 0;

const wchar_t* CLASS_WELCOME   = L"PrankForgeWelcome";
const wchar_t* CLASS_MAIN      = L"PrankForgeMain";
const wchar_t* CLASS_CREATE    = L"PrankForgeCreate";
const wchar_t* CLASS_EDITOR    = L"PrankForgeEditor";
const wchar_t* CLASS_EFFECTSEL = L"PrankForgeEffectSel";

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

struct Block {
    int id;
    BlockType type;
    std::wstring title;
    int x, y, w, h;
    bool selected;

    // Welcome
    std::wstring welcomeText = L"Это пранк. Запускать только на виртуалке.";

    // GDI
    int gdiEffect = 0;
    int gdiDuration = 10;
    int gdiIntensity = 60;

    // Bytebeat
    int bbTrack = 0;
    int bbVolume = 70;

    // MessageBox
    std::wstring msgText = L"Сообщение";
    int msgIcon = 2;
    int msgCount = 1;
    int msgTrajectory = 0;
    int msgDelay = 500;

    // Link
    std::wstring linkURL = L"https://google.com";
    int linkCount = 1;
    int linkDelay = 2000;

    // Wallpaper
    int wpType = 0;      // 0=color, 1=png
    int wpColor = 0;     // индекс в палитре
    std::wstring wpPath; // путь к PNG

    // Registry
    bool regTaskMgr = false;
    bool regRegedit = false;
    bool regControlPanel = false;
    bool regSettings = false;
    bool regInternet = false;
    bool regWarn = true;

    // Reboot
    int rebootDelay = 5;

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
enum EditField { EF_NONE = 0, EF_WELCOME, EF_MSGTEXT, EF_LINKURL, EF_WPPATH };

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
    int activeSlider = -1;
    RECT sliderRects[8] = {};
    int sliderCount = 0;

    // Кнопки
    RECT buttonRects[8] = {};
    int buttonCount = 0;

    // Чекбоксы
    RECT checkboxRects[8] = {};
    int checkboxCount = 0;

    // Цветовые свотчи
    RECT swatchRects[16] = {};
    int swatchCount = 0;

    // EDIT control
    HWND propEdit = nullptr;
    int editBlockId = -1;
    EditField editField = EF_NONE;
    RECT editRect = {};

    std::wstring projectName;
    std::wstring projectPath;
    bool modified = false;

    // Hover индексы
    int hoveredBtn = -1;
    int hoveredCheckbox = -1;
    int hoveredSwatch = -1;
};
EditorState g_editor;
HWND g_editorWnd = nullptr;

struct EffectSelState {
    HWND parent = nullptr;
    int scrollY = 0;
    int hovered = -1;
    int selected = -1;
    int forBlockId = -1;
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

void DrawTextCentered(HDC dc, const wchar_t* t, RECT rc, COLORREF c, int fs, bool b) {
    SetBkMode(dc, TRANSPARENT); SetTextColor(dc, c);
    HFONT f = CreateFontW(fs, 0, 0, 0, b ? FW_BOLD : FW_NORMAL, 0, 0, 0,
        DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    HFONT old = (HFONT)SelectObject(dc, f);
    DrawTextW(dc, t, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(dc, old); DeleteObject(f);
}

void DrawTextLeft(HDC dc, const wchar_t* t, RECT rc, COLORREF c, int fs, bool b) {
    SetBkMode(dc, TRANSPARENT); SetTextColor(dc, c);
    HFONT f = CreateFontW(fs, 0, 0, 0, b ? FW_BOLD : FW_NORMAL, 0, 0, 0,
        DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    HFONT old = (HFONT)SelectObject(dc, f);
    DrawTextW(dc, t, -1, &rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    SelectObject(dc, old); DeleteObject(f);
}

void DrawCustomButton(HDC dc, const RECT& rc, const wchar_t* t, bool hov, bool accent) {
    COLORREF bg = accent ? (hov ? Colors::ACCENT_H : Colors::ACCENT)
                        : (hov ? Colors::BG_HOVER : Colors::BG_PANEL);
    FillRectC(dc, rc, bg);
    HBRUSH border = CreateSolidBrush(Colors::BORDER);
    FrameRect(dc, &rc, border); DeleteObject(border);
    DrawTextCentered(dc, t, rc, accent ? RGB(255,255,255) : Colors::TEXT, 14, true);
}

void DrawSlider(HDC dc, const RECT& rc, int value, int maxVal, bool hovered) {
    int midY = (rc.top + rc.bottom) / 2;
    RECT trackRc = { rc.left, midY - 4, rc.right, midY + 4 };
    FillRectC(dc, trackRc, Colors::SLIDER_BG);
    int fillW = (rc.right - rc.left) * value / (maxVal > 0 ? maxVal : 1);
    RECT fillRc = { rc.left, midY - 4, rc.left + fillW, midY + 4 };
    FillRectC(dc, fillRc, Colors::SLIDER_FG);
    int thumbX = rc.left + fillW;
    int thumbR = hovered ? 12 : 10;
    HBRUSH tb = CreateSolidBrush(hovered ? Colors::ACCENT_H : Colors::ACCENT);
    HBRUSH old = (HBRUSH)SelectObject(dc, tb);
    HPEN oldP = (HPEN)SelectObject(dc, GetStockObject(NULL_PEN));
    Ellipse(dc, thumbX - thumbR, midY - thumbR, thumbX + thumbR, midY + thumbR);
    SelectObject(dc, old); SelectObject(dc, oldP); DeleteObject(tb);
}

void DrawCheckbox(HDC dc, const RECT& rc, bool checked, bool hovered) {
    int sz = 18;
    RECT box = { rc.left, rc.top + (rc.bottom - rc.top - sz) / 2, rc.left + sz, rc.top + (rc.bottom - rc.top + sz) / 2 };
    FillRectC(dc, box, hovered ? Colors::BG_HOVER : Colors::BG_PANEL);
    HBRUSH border = CreateSolidBrush(Colors::BORDER);
    FrameRect(dc, &box, border); DeleteObject(border);
    if (checked) {
        RECT inner = { box.left + 3, box.top + 3, box.right - 3, box.bottom - 3 };
        FillRectC(dc, inner, Colors::CHECK_ON);
    }
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
std::string EscapeJson(const std::string& s) {
    std::string out;
    for (char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out += c;
        }
    }
    return out;
}

bool SaveProject() {
    if (g_editor.projectPath.empty() || g_editor.projectName.empty()) return false;
    // Не сохраняем если открыт редактор одного проекта, а активных блоков нет — не трогаем
    wchar_t filePath[MAX_PATH];
    swprintf_s(filePath, L"%s\\project.pforge", g_editor.projectPath.c_str());

    std::string j;
    j += "{\n  \"name\": \"" + EscapeJson(WideToUtf8(g_editor.projectName)) + "\",\n  \"blocks\": [\n";
    for (size_t i = 0; i < g_editor.blocks.size(); ++i) {
        Block& b = g_editor.blocks[i];
        j += "    {\n";
        j += "      \"id\": " + std::to_string(b.id) + ",\n";
        j += "      \"type\": " + std::to_string((int)b.type) + ",\n";
        j += "      \"x\": " + std::to_string(b.x) + ",\n";
        j += "      \"y\": " + std::to_string(b.y) + ",\n";
        j += "      \"welcomeText\": \"" + EscapeJson(WideToUtf8(b.welcomeText)) + "\",\n";
        j += "      \"gdiEffect\": " + std::to_string(b.gdiEffect) + ",\n";
        j += "      \"gdiDuration\": " + std::to_string(b.gdiDuration) + ",\n";
        j += "      \"gdiIntensity\": " + std::to_string(b.gdiIntensity) + ",\n";
        j += "      \"bbTrack\": " + std::to_string(b.bbTrack) + ",\n";
        j += "      \"bbVolume\": " + std::to_string(b.bbVolume) + ",\n";
        j += "      \"msgText\": \"" + EscapeJson(WideToUtf8(b.msgText)) + "\",\n";
        j += "      \"msgIcon\": " + std::to_string(b.msgIcon) + ",\n";
        j += "      \"msgCount\": " + std::to_string(b.msgCount) + ",\n";
        j += "      \"msgTrajectory\": " + std::to_string(b.msgTrajectory) + ",\n";
        j += "      \"msgDelay\": " + std::to_string(b.msgDelay) + ",\n";
        j += "      \"linkURL\": \"" + EscapeJson(WideToUtf8(b.linkURL)) + "\",\n";
        j += "      \"linkCount\": " + std::to_string(b.linkCount) + ",\n";
        j += "      \"linkDelay\": " + std::to_string(b.linkDelay) + ",\n";
        j += "      \"wpType\": " + std::to_string(b.wpType) + ",\n";
        j += "      \"wpColor\": " + std::to_string(b.wpColor) + ",\n";
        j += "      \"wpPath\": \"" + EscapeJson(WideToUtf8(b.wpPath)) + "\",\n";
        j += "      \"regTaskMgr\": " + std::to_string(b.regTaskMgr ? 1 : 0) + ",\n";
        j += "      \"regRegedit\": " + std::to_string(b.regRegedit ? 1 : 0) + ",\n";
        j += "      \"regControlPanel\": " + std::to_string(b.regControlPanel ? 1 : 0) + ",\n";
        j += "      \"regSettings\": " + std::to_string(b.regSettings ? 1 : 0) + ",\n";
        j += "      \"regInternet\": " + std::to_string(b.regInternet ? 1 : 0) + ",\n";
        j += "      \"regWarn\": " + std::to_string(b.regWarn ? 1 : 0) + ",\n";
        j += "      \"rebootDelay\": " + std::to_string(b.rebootDelay) + "\n";
        j += "    }";
        if (i + 1 < g_editor.blocks.size()) j += ",";
        j += "\n";
    }
    j += "  ]\n}\n";

    HANDLE f = CreateFileW(filePath, GENERIC_WRITE, 0, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (f == INVALID_HANDLE_VALUE) return false;
    DWORD wr; WriteFile(f, j.c_str(), (DWORD)j.size(), &wr, nullptr);
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
        if (sx >= tl.x && sx <= tl.x + sw && sy >= tl.y && sy <= tl.y + sh) return &(*it);
    }
    return nullptr;
}
Block* FindBlockById(int id) {
    for (auto& b : g_editor.blocks) if (b.id == id) return &b;
    return nullptr;
}

// ============================================================
// РИСОВАНИЕ
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
    FrameRect(dc, &rc, border); DeleteObject(border);
    RECT iconRc = { tl.x + 15, tl.y + 5, tl.x + 50, tl.y + sh - 5 };
    DrawTextCentered(dc, g_categories[b.type].icon, iconRc, Colors::TEXT, 22, false);
    RECT nameRc = { tl.x + 55, tl.y + 5, tl.x + sw - 10, tl.y + 35 };
    DrawTextLeft(dc, b.title.c_str(), nameRc, Colors::TEXT, 14, true);

    std::wstring subtitle = g_categories[b.type].ruName;
    wchar_t buf[128];
    if (b.type == BT_GDI) {
        swprintf_s(buf, L"%s · %d сек · %d%%",
            g_gdiEffects[b.gdiEffect].ruName, b.gdiDuration, b.gdiIntensity);
        subtitle = buf;
    } else if (b.type == BT_BYTEBEAT) {
        swprintf_s(buf, L"%s · %d%%", g_byteTracks[b.bbTrack].name, b.bbVolume);
        subtitle = buf;
    } else if (b.type == BT_MSGBOX) {
        swprintf_s(buf, L"%s · x%d", b.msgText.c_str(), b.msgCount);
        subtitle = buf;
    } else if (b.type == BT_LINK) {
        swprintf_s(buf, L"%s · x%d", b.linkURL.c_str(), b.linkCount);
        subtitle = buf;
    } else if (b.type == BT_WALLPAPER) {
        subtitle = (b.wpType == 0) ? g_wpColors[b.wpColor].name : L"PNG файл";
    } else if (b.type == BT_REGISTRY) {
        int c = (b.regTaskMgr?1:0)+(b.regRegedit?1:0)+(b.regControlPanel?1:0)+(b.regSettings?1:0)+(b.regInternet?1:0);
        swprintf_s(buf, L"Активно: %d/5", c);
        subtitle = buf;
    } else if (b.type == BT_REBOOT) {
        swprintf_s(buf, L"Задержка: %d сек", b.rebootDelay);
        subtitle = buf;
    }
    RECT typeRc = { tl.x + 55, tl.y + 35, tl.x + sw - 10, tl.y + sh - 5 };
    DrawTextLeft(dc, subtitle.c_str(), typeRc, Colors::TEXT_DIM, 11, false);

    POINT inPort = CanvasToScreen(b.x, b.y + b.h / 2);
    POINT outPort = CanvasToScreen(b.x + b.w, b.y + b.h / 2);
    HBRUSH pb = CreateSolidBrush(catColor);
    int pr = (int)(5 * g_editor.zoom); if (pr < 3) pr = 3;
    HBRUSH oldB = (HBRUSH)SelectObject(dc, pb);
    HPEN oldP = (HPEN)SelectObject(dc, GetStockObject(NULL_PEN));
    Ellipse(dc, inPort.x - pr, inPort.y - pr, inPort.x + pr, inPort.y + pr);
    Ellipse(dc, outPort.x - pr, outPort.y - pr, outPort.x + pr, outPort.y + pr);
    SelectObject(dc, oldB); SelectObject(dc, oldP); DeleteObject(pb);
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
                    int cx = c.x + c.w / 2, cy = c.y + c.h / 2;
                    if (cx > a.x + a.w && cx < b.x && std::abs(cy - (a.y + a.h / 2)) < 60) {
                        blocked = true; break;
                    }
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
    SelectObject(dc, oldPen); DeleteObject(linkPen);
}

void DrawGrid(HDC dc, const RECT& rc) {
    int gs = (int)(50 * g_editor.zoom); if (gs < 10) gs = 10;
    int sx = -(int)(g_editor.scrollX * g_editor.zoom) % gs;
    int sy = -(int)(g_editor.scrollY * g_editor.zoom) % gs;
    if (sx > 0) sx -= gs; if (sy > 0) sy -= gs;
    HPEN pen = CreatePen(PS_SOLID, 1, Colors::GRID);
    HPEN oldPen = (HPEN)SelectObject(dc, pen);
    for (int x = sx; x < rc.right; x += gs) { MoveToEx(dc, x, 0, nullptr); LineTo(dc, x, rc.bottom); }
    for (int y = sy; y < rc.bottom; y += gs) { MoveToEx(dc, 0, y, nullptr); LineTo(dc, rc.right, y); }
    SelectObject(dc, oldPen); DeleteObject(pen);
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
        FillRectC(dc, itemRc, (hovered || expanded) ? Colors::BG_HOVER : Colors::BG_PANEL);
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
// ПАНЕЛЬ СВОЙСТВ
// ============================================================
void DrawProperties(HDC dc, HWND h) {
    RECT rc; GetClientRect(h, &rc);
    int left = rc.right - PROPS_W;
    RECT propRc = { left, TOPBAR_H, rc.right, rc.bottom - BOTTOM_H };
    FillRectC(dc, propRc, Colors::BG_DARK);

    g_editor.sliderCount = 0;
    g_editor.buttonCount = 0;
    g_editor.checkboxCount = 0;
    g_editor.swatchCount = 0;

    RECT titleRc = { left + 15, TOPBAR_H + 15, rc.right - 15, TOPBAR_H + 50 };
    DrawTextLeft(dc, (g_language == 0) ? L"Свойства" : L"Properties",
        titleRc, Colors::TEXT, 16, true);

    if (g_editor.selectedId < 0) {
        RECT emptyRc = { left + 15, TOPBAR_H + 70, rc.right - 15, TOPBAR_H + 200 };
        DrawTextCentered(dc, (g_language == 0)
            ? L"Выбери блок,\nчтобы увидеть настройки"
            : L"Select a block\nto see settings",
            emptyRc, Colors::TEXT_DIM, 13, false);
        return;
    }

    Block* b = FindBlockById(g_editor.selectedId);
    if (!b) return;

    int y = TOPBAR_H + 70;
    int iL = left + 15;
    int iR = rc.right - 15;

    // Название и тип
    RECT labelName = { iL, y, iR, y + 20 };
    DrawTextLeft(dc, (g_language == 0) ? L"Название:" : L"Name:", labelName, Colors::TEXT_DIM, 11, false);
    y += 22;
    RECT valueName = { iL, y, iR, y + 26 };
    FillRectC(dc, valueName, Colors::BG_PANEL);
    DrawTextLeft(dc, b->title.c_str(), valueName, Colors::TEXT, 13, false);
    y += 34;

    RECT labelType = { iL, y, iR, y + 20 };
    DrawTextLeft(dc, (g_language == 0) ? L"Тип:" : L"Type:", labelType, Colors::TEXT_DIM, 11, false);
    y += 22;
    RECT valueType = { iL, y, iR, y + 26 };
    FillRectC(dc, valueType, Colors::BG_PANEL);
    const wchar_t* typeName = (g_language == 0) ? g_categories[b->type].ruName : g_categories[b->type].enName;
    DrawTextLeft(dc, typeName, valueType, Colors::TEXT, 13, false);
    y += 34;

    g_editor.editRect = { 0,0,0,0 };
    g_editor.editField = EF_NONE;
    g_editor.editBlockId = -1;

    // === ВЫБОР ПО ТИПУ ===
    if (b->type == BT_WELCOME) {
        RECT label = { iL, y, iR, y + 20 };
        DrawTextLeft(dc, (g_language == 0) ? L"Текст приветствия:" : L"Welcome text:",
            label, Colors::TEXT_DIM, 11, false);
        y += 22;
        g_editor.editRect = { iL, y, iR, y + 50 };
        g_editor.editField = EF_WELCOME;
        g_editor.editBlockId = b->id;
        y += 60;
    }

    else if (b->type == BT_GDI) {
        RECT label = { iL, y, iR, y + 20 };
        DrawTextLeft(dc, (g_language == 0) ? L"Эффект:" : L"Effect:", label, Colors::TEXT_DIM, 11, false);
        y += 22;
        RECT effBtn = { iL, y, iR, y + 30 };
        g_editor.buttonRects[g_editor.buttonCount++] = effBtn;
        DrawCustomButton(dc, effBtn, g_gdiEffects[b->gdiEffect].ruName, false, false);
        y += 38;
        // Сложность
        std::wstring stars = (g_language == 0) ? L"Сложность: " : L"Complexity: ";
        for (int i = 0; i < g_gdiEffects[b->gdiEffect].complexity; ++i) stars += L"★";
        RECT sr = { iL, y, iR, y + 18 };
        DrawTextLeft(dc, stars.c_str(), sr, RGB(255,200,80), 11, false);
        y += 26;
        // Intensity
        RECT li = { iL, y, iR - 60, y + 20 };
        DrawTextLeft(dc, (g_language == 0) ? L"Интенсивность:" : L"Intensity:", li, Colors::TEXT_DIM, 11, false);
        wchar_t b1[16]; swprintf_s(b1, L"%d%%", b->gdiIntensity);
        RECT vi = { iR - 55, y, iR, y + 20 };
        DrawTextLeft(dc, b1, vi, Colors::TEXT, 12, true);
        y += 22;
        RECT sl = { iL, y, iR, y + 26 };
        g_editor.sliderRects[g_editor.sliderCount++] = sl;
        DrawSlider(dc, sl, b->gdiIntensity, 100, false);
        y += 36;
        // Duration
        RECT ld = { iL, y, iR - 80, y + 20 };
        DrawTextLeft(dc, (g_language == 0) ? L"Длительность:" : L"Duration:", ld, Colors::TEXT_DIM, 11, false);
        wchar_t b2[16]; swprintf_s(b2, L"%d сек", b->gdiDuration);
        RECT vd = { iR - 80, y, iR, y + 20 };
        DrawTextLeft(dc, b2, vd, Colors::TEXT, 12, true);
        y += 22;
        RECT sd = { iL, y, iR, y + 26 };
        g_editor.sliderRects[g_editor.sliderCount++] = sd;
        DrawSlider(dc, sd, b->gdiDuration, 60, false);
        y += 36;
    }

    else if (b->type == BT_BYTEBEAT) {
        RECT label = { iL, y, iR, y + 20 };
        DrawTextLeft(dc, (g_language == 0) ? L"Трек:" : L"Track:", label, Colors::TEXT_DIM, 11, false);
        y += 22;
        RECT tb = { iL, y, iR, y + 30 };
        g_editor.buttonRects[g_editor.buttonCount++] = tb;
        DrawCustomButton(dc, tb, g_byteTracks[b->bbTrack].name, false, false);
        y += 38;
        RECT lv = { iL, y, iR - 60, y + 20 };
        DrawTextLeft(dc, (g_language == 0) ? L"Громкость:" : L"Volume:", lv, Colors::TEXT_DIM, 11, false);
        wchar_t b1[16]; swprintf_s(b1, L"%d%%", b->bbVolume);
        RECT vv = { iR - 55, y, iR, y + 20 };
        DrawTextLeft(dc, b1, vv, Colors::TEXT, 12, true);
        y += 22;
        RECT sv = { iL, y, iR, y + 26 };
        g_editor.sliderRects[g_editor.sliderCount++] = sv;
        DrawSlider(dc, sv, b->bbVolume, 100, false);
        y += 36;
    }

    else if (b->type == BT_MSGBOX) {
        RECT l1 = { iL, y, iR, y + 20 };
        DrawTextLeft(dc, (g_language == 0) ? L"Текст:" : L"Text:", l1, Colors::TEXT_DIM, 11, false);
        y += 22;
        g_editor.editRect = { iL, y, iR, y + 40 };
        g_editor.editField = EF_MSGTEXT;
        g_editor.editBlockId = b->id;
        y += 48;

        RECT l2 = { iL, y, iR, y + 20 };
        DrawTextLeft(dc, (g_language == 0) ? L"Иконка:" : L"Icon:", l2, Colors::TEXT_DIM, 11, false);
        y += 22;
        const wchar_t* icons[] = { L"Info", L"Warning", L"Error", L"Question" };
        RECT ib = { iL, y, iR, y + 28 };
        g_editor.buttonRects[g_editor.buttonCount++] = ib;
        DrawCustomButton(dc, ib, icons[b->msgIcon], false, false);
        y += 36;

        RECT l3 = { iL, y, iR - 60, y + 20 };
        DrawTextLeft(dc, (g_language == 0) ? L"Количество:" : L"Count:", l3, Colors::TEXT_DIM, 11, false);
        wchar_t b1[16]; swprintf_s(b1, L"x%d", b->msgCount);
        RECT v3 = { iR - 55, y, iR, y + 20 };
        DrawTextLeft(dc, b1, v3, Colors::TEXT, 12, true);
        y += 22;
        RECT s3 = { iL, y, iR, y + 26 };
        g_editor.sliderRects[g_editor.sliderCount++] = s3;
        DrawSlider(dc, s3, b->msgCount, 30, false);
        y += 36;

        RECT l4 = { iL, y, iR, y + 20 };
        DrawTextLeft(dc, (g_language == 0) ? L"Траектория:" : L"Trajectory:", l4, Colors::TEXT_DIM, 11, false);
        y += 22;
        const wchar_t* trajs[] = { L"Круг вокруг курсора", L"Без курсора (центр)", L"Вертикально", L"Горизонтально", L"Случайно" };
        RECT tb = { iL, y, iR, y + 28 };
        g_editor.buttonRects[g_editor.buttonCount++] = tb;
        DrawCustomButton(dc, tb, trajs[b->msgTrajectory], false, false);
        y += 36;

        RECT l5 = { iL, y, iR - 60, y + 20 };
        DrawTextLeft(dc, (g_language == 0) ? L"Задержка (мс):" : L"Delay (ms):", l5, Colors::TEXT_DIM, 11, false);
        wchar_t b5[16]; swprintf_s(b5, L"%d", b->msgDelay);
        RECT v5 = { iR - 60, y, iR, y + 20 };
        DrawTextLeft(dc, b5, v5, Colors::TEXT, 12, true);
        y += 22;
        RECT s5 = { iL, y, iR, y + 26 };
        g_editor.sliderRects[g_editor.sliderCount++] = s5;
        DrawSlider(dc, s5, b->msgDelay, 3000, false);
        y += 36;
    }

    else if (b->type == BT_LINK) {
        RECT l1 = { iL, y, iR, y + 20 };
        DrawTextLeft(dc, (g_language == 0) ? L"URL:" : L"URL:", l1, Colors::TEXT_DIM, 11, false);
        y += 22;
        g_editor.editRect = { iL, y, iR, y + 32 };
        g_editor.editField = EF_LINKURL;
        g_editor.editBlockId = b->id;
        y += 40;

        RECT l2 = { iL, y, iR - 60, y + 20 };
        DrawTextLeft(dc, (g_language == 0) ? L"Открыть раз:" : L"Open count:", l2, Colors::TEXT_DIM, 11, false);
        wchar_t b1[16]; swprintf_s(b1, L"x%d", b->linkCount);
        RECT v2 = { iR - 55, y, iR, y + 20 };
        DrawTextLeft(dc, b1, v2, Colors::TEXT, 12, true);
        y += 22;
        RECT s2 = { iL, y, iR, y + 26 };
        g_editor.sliderRects[g_editor.sliderCount++] = s2;
        DrawSlider(dc, s2, b->linkCount, 10, false);
        y += 36;

        RECT l3 = { iL, y, iR - 80, y + 20 };
        DrawTextLeft(dc, (g_language == 0) ? L"Задержка (мс):" : L"Delay (ms):", l3, Colors::TEXT_DIM, 11, false);
        wchar_t b3[16]; swprintf_s(b3, L"%d", b->linkDelay);
        RECT v3 = { iR - 60, y, iR, y + 20 };
        DrawTextLeft(dc, b3, v3, Colors::TEXT, 12, true);
        y += 22;
        RECT s3 = { iL, y, iR, y + 26 };
        g_editor.sliderRects[g_editor.sliderCount++] = s3;
        DrawSlider(dc, s3, b->linkDelay, 5000, false);
        y += 36;
    }

    else if (b->type == BT_WALLPAPER) {
        RECT l1 = { iL, y, iR, y + 20 };
        DrawTextLeft(dc, (g_language == 0) ? L"Тип:" : L"Type:", l1, Colors::TEXT_DIM, 11, false);
        y += 22;
        RECT typeBtn = { iL, y, iR, y + 28 };
        g_editor.buttonRects[g_editor.buttonCount++] = typeBtn;
        DrawCustomButton(dc, typeBtn, b->wpType == 0 ? L"Цвет" : L"PNG файл", false, false);
        y += 36;

        if (b->wpType == 0) {
            RECT l2 = { iL, y, iR, y + 20 };
            DrawTextLeft(dc, (g_language == 0) ? L"Палитра:" : L"Palette:", l2, Colors::TEXT_DIM, 11, false);
            y += 22;
            int perRow = 6;
            int cellSz = 36;
            for (int i = 0; i < kWpColorCount; ++i) {
                int row = i / perRow, col = i % perRow;
                int cx = iL + col * (cellSz + 6);
                int cy = y + row * (cellSz + 6);
                RECT sw = { cx, cy, cx + cellSz, cy + cellSz };
                g_editor.swatchRects[g_editor.swatchCount++] = sw;
                FillRectC(dc, sw, g_wpColors[i].color);
                HBRUSH br = CreateSolidBrush(g_wpColors[i].color);
                FrameRect(dc, &sw, br); DeleteObject(br);
                if (b->wpColor == i) {
                    HBRUSH ring = CreateSolidBrush(Colors::ACCENT);
                    RECT outer = { sw.left - 2, sw.top - 2, sw.right + 2, sw.bottom + 2 };
                    FrameRect(dc, &outer, ring); DeleteObject(ring);
                }
            }
            y += 2 * (cellSz + 6) + 10;
        } else {
            RECT l2 = { iL, y, iR, y + 20 };
            DrawTextLeft(dc, (g_language == 0) ? L"Путь к PNG:" : L"PNG path:", l2, Colors::TEXT_DIM, 11, false);
            y += 22;
            g_editor.editRect = { iL, y, iR, y + 32 };
            g_editor.editField = EF_WPPATH;
            g_editor.editBlockId = b->id;
            y += 40;
            RECT browseBtn = { iL, y, iR, y + 28 };
            g_editor.buttonRects[g_editor.buttonCount++] = browseBtn;
            DrawCustomButton(dc, browseBtn, L"📂 Выбрать PNG", false, false);
            y += 36;
        }
    }

    else if (b->type == BT_REGISTRY) {
        RECT warnRc = { iL, y, iR, y + 50 };
        FillRectC(dc, warnRc, RGB(60, 40, 20));
        HBRUSH wb = CreateSolidBrush(Colors::WARN);
        FrameRect(dc, &warnRc, wb); DeleteObject(wb);
        RECT wr = warnRc; wr.left += 8;
        DrawTextLeft(dc, L"⚠ Изменит реестр Windows.",
            wr, Colors::WARN, 11, true);
        RECT wr2 = warnRc; wr2.left += 8; wr2.top += 20;
        DrawTextLeft(dc, L"Только для виртуалки!", wr2, Colors::WARN, 11, false);
        y += 60;

        RECT l1 = { iL, y, iR, y + 20 };
        DrawTextLeft(dc, (g_language == 0) ? L"Блокировки:" : L"Blocks:", l1, Colors::TEXT_DIM, 11, false);
        y += 22;

        struct { const wchar_t* label; bool* ptr; } items[] = {
            { L"Диспетчер задач",  &b->regTaskMgr },
            { L"Редактор реестра", &b->regRegedit },
            { L"Панель управления", &b->regControlPanel },
            { L"Параметры Windows", &b->regSettings },
            { L"Интернет",         &b->regInternet }
        };
        for (int i = 0; i < 5; ++i) {
            RECT cb = { iL, y, iR, y + 26 };
            g_editor.checkboxRects[g_editor.checkboxCount++] = cb;
            DrawCheckbox(dc, cb, *items[i].ptr, false);
            RECT lr = cb; lr.left += 28;
            DrawTextLeft(dc, items[i].label, lr, Colors::TEXT, 12, false);
            y += 30;
        }

        y += 6;
        RECT cbW = { iL, y, iR, y + 26 };
        g_editor.checkboxRects[g_editor.checkboxCount++] = cbW;
        DrawCheckbox(dc, cbW, b->regWarn, false);
        RECT lrW = cbW; lrW.left += 28;
        DrawTextLeft(dc, L"Показать предупреждение", lrW, Colors::TEXT, 12, false);
        y += 30;
    }

    else if (b->type == BT_REBOOT) {
        RECT l1 = { iL, y, iR - 80, y + 20 };
        DrawTextLeft(dc, (g_language == 0) ? L"Задержка (сек):" : L"Delay (sec):", l1, Colors::TEXT_DIM, 11, false);
        wchar_t b1[16]; swprintf_s(b1, L"%d сек", b->rebootDelay);
        RECT v1 = { iR - 70, y, iR, y + 20 };
        DrawTextLeft(dc, b1, v1, Colors::TEXT, 12, true);
        y += 22;
        RECT s1 = { iL, y, iR, y + 26 };
        g_editor.sliderRects[g_editor.sliderCount++] = s1;
        DrawSlider(dc, s1, b->rebootDelay, 30, false);
        y += 40;

        RECT noteRc = { iL, y, iR, y + 40 };
        FillRectC(dc, noteRc, RGB(50, 30, 30));
        DrawTextCentered(dc, L"Реальная перезагрузка ПК", noteRc, RGB(255,150,150), 11, false);
        y += 50;
    }

    // Позиция
    wchar_t posBuf[64];
    swprintf_s(posBuf, L"X: %d, Y: %d", b->x, b->y);
    RECT labelPos = { iL, y, iR, y + 18 };
    DrawTextLeft(dc, posBuf, labelPos, Colors::TEXT_DIM, 10, false);
}

// ============================================================
// МОДАЛЬНОЕ ОКНО ВЫБОРА GDI-ЭФФЕКТА
// ============================================================
LRESULT CALLBACK EffectSelProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    static int hoverOk = 0, hoverCancel = 0;
    switch (m) {
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC dc = BeginPaint(h, &ps);
        RECT rc; GetClientRect(h, &rc);
        FillRectC(dc, rc, Colors::BG);
        RECT titleRc = { 20, 15, rc.right - 20, 50 };
        DrawTextLeft(dc, (g_language == 0) ? L"Выбор GDI эффекта" : L"Select GDI effect",
            titleRc, Colors::TEXT, 18, true);
        RECT listRc = { 20, 60, rc.right - 20, rc.bottom - 70 };
        FillRectC(dc, listRc, Colors::BG_PANEL);
        HRGN hRgn = CreateRectRgn(listRc.left, listRc.top, listRc.right, listRc.bottom);
        SelectClipRgn(dc, hRgn);
        int itemH = 44;
        int startY = listRc.top - g_effectSel.scrollY;
        for (int i = 0; i < kGdiEffectCount; ++i) {
            RECT itemRc = { listRc.left, startY + i * itemH, listRc.right, startY + (i + 1) * itemH - 4 };
            if (itemRc.bottom < listRc.top || itemRc.top > listRc.bottom) continue;
            bool hov = (i == g_effectSel.hovered);
            bool sel = (i == g_effectSel.selected);
            FillRectC(dc, itemRc, sel ? Colors::ACCENT : (hov ? Colors::BG_HOVER : Colors::BG_PANEL));
            RECT nameRc = { itemRc.left + 15, itemRc.top, itemRc.right - 100, itemRc.bottom };
            DrawTextLeft(dc, g_gdiEffects[i].ruName, nameRc, Colors::TEXT, 14, true);
            std::wstring stars;
            for (int j = 0; j < g_gdiEffects[i].complexity; ++j) stars += L"★";
            RECT starRc = { itemRc.right - 90, itemRc.top, itemRc.right - 10, itemRc.bottom };
            DrawTextLeft(dc, stars.c_str(), starRc, RGB(255,200,80), 14, false);
        }
        SelectClipRgn(dc, nullptr); DeleteObject(hRgn);
        RECT cancelBtn = { rc.right - 240, rc.bottom - 55, rc.right - 140, rc.bottom - 15 };
        RECT okBtn = { rc.right - 130, rc.bottom - 55, rc.right - 20, rc.bottom - 15 };
        DrawCustomButton(dc, cancelBtn, (g_language == 0) ? L"Отмена" : L"Cancel", hoverCancel, false);
        DrawCustomButton(dc, okBtn, L"OK", hoverOk, true);
        EndPaint(h, &ps); return 0;
    }
    case WM_MOUSEMOVE: {
        int x = LOWORD(l), y = HIWORD(l); RECT rc; GetClientRect(h, &rc);
        RECT listRc = { 20, 60, rc.right - 20, rc.bottom - 70 };
        RECT cancelBtn = { rc.right - 240, rc.bottom - 55, rc.right - 140, rc.bottom - 15 };
        RECT okBtn = { rc.right - 130, rc.bottom - 55, rc.right - 20, rc.bottom - 15 };
        auto ins = [](const RECT& r, int px, int py) {
            return px >= r.left && px <= r.right && py >= r.top && py <= r.bottom; };
        int nh = -1;
        if (ins(listRc, x, y)) {
            int relY = y - listRc.top + g_effectSel.scrollY;
            int idx = relY / 44;
            if (idx >= 0 && idx < kGdiEffectCount) nh = idx;
        }
        int nOk = ins(okBtn, x, y), nC = ins(cancelBtn, x, y);
        if (nh != g_effectSel.hovered || nOk != hoverOk || nC != hoverCancel) {
            g_effectSel.hovered = nh; hoverOk = nOk; hoverCancel = nC;
            InvalidateRect(h, nullptr, FALSE);
        }
        if (nh >= 0 || nOk || nC) SetCursor(LoadCursor(nullptr, IDC_HAND));
        else SetCursor(LoadCursor(nullptr, IDC_ARROW));
        return 0;
    }
    case WM_LBUTTONDOWN: {
        int x = LOWORD(l), y = HIWORD(l); RECT rc; GetClientRect(h, &rc);
        RECT listRc = { 20, 60, rc.right - 20, rc.bottom - 70 };
        RECT cancelBtn = { rc.right - 240, rc.bottom - 55, rc.right - 140, rc.bottom - 15 };
        RECT okBtn = { rc.right - 130, rc.bottom - 55, rc.right - 20, rc.bottom - 15 };
        auto ins = [](const RECT& r, int px, int py) {
            return px >= r.left && px <= r.right && py >= r.top && py <= r.bottom; };
        if (ins(listRc, x, y)) {
            int relY = y - listRc.top + g_effectSel.scrollY;
            int idx = relY / 44;
            if (idx >= 0 && idx < kGdiEffectCount) {
                g_effectSel.selected = idx;
                InvalidateRect(h, nullptr, FALSE);
            }
            return 0;
        }
        if (ins(okBtn, x, y)) {
            if (g_effectSel.selected >= 0) {
                Block* b = FindBlockById(g_effectSel.forBlockId);
                if (b) { b->gdiEffect = g_effectSel.selected; g_editor.modified = true; }
            }
            DestroyWindow(h); return 0;
        }
        if (ins(cancelBtn, x, y)) { DestroyWindow(h); return 0; }
        return 0;
    }
    case WM_MOUSEWHEEL: {
        int d = GET_WHEEL_DELTA_WPARAM(w);
        g_effectSel.scrollY -= d / 2;
        if (g_effectSel.scrollY < 0) g_effectSel.scrollY = 0;
        InvalidateRect(h, nullptr, FALSE);
        return 0;
    }
    case WM_CLOSE: DestroyWindow(h); return 0;
    }
    return DefWindowProcW(h, m, w, l);
}

void OpenEffectSelector(HWND parent, int blockId, int curEff) {
    g_effectSel.parent = parent;
    g_effectSel.scrollY = 0;
    g_effectSel.hovered = -1;
    g_effectSel.selected = curEff;
    g_effectSel.forBlockId = blockId;
    int w = 500, h = 600;
    int px = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    int py = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;
    HWND dlg = CreateWindowExW(WS_EX_DLGMODALFRAME, CLASS_EFFECTSEL, L"GDI эффекты",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, px, py, w, h,
        parent, nullptr, g_hInst, nullptr);
    ShowWindow(dlg, SW_SHOW); UpdateWindow(dlg);
}

// ============================================================
// Синхронизация EDIT-контрола
// ============================================================
void SyncPropEdit(HWND h) {
    if (!g_editor.propEdit) return;

    // Скрываем если не нужно
    if (g_editor.editField == EF_NONE || g_editor.editBlockId < 0) {
        if (IsWindowVisible(g_editor.propEdit)) ShowWindow(g_editor.propEdit, SW_HIDE);
        return;
    }

    Block* b = FindBlockById(g_editor.editBlockId);
    if (!b) { ShowWindow(g_editor.propEdit, SW_HIDE); return; }

    // Позиционируем
    RECT er = g_editor.editRect;
    RECT cr; GetClientRect(h, &cr);
    int left = cr.right - PROPS_W;
    RECT absolute = { left + (er.left - left), er.top, left + (er.right - left), er.bottom };
    // er уже в клиентских координатах главного окна
    SetWindowPos(g_editor.propEdit, HWND_TOP,
        g_editor.editRect.left, g_editor.editRect.top,
        g_editor.editRect.right - g_editor.editRect.left,
        g_editor.editRect.bottom - g_editor.editRect.top,
        SWP_NOACTIVATE);

    // Устанавливаем текст, если ещё не тот
    wchar_t cur[1024] = {};
    GetWindowTextW(g_editor.propEdit, cur, 1024);
    std::wstring want;
    switch (g_editor.editField) {
        case EF_WELCOME: want = b->welcomeText; break;
        case EF_MSGTEXT: want = b->msgText; break;
        case EF_LINKURL: want = b->linkURL; break;
        case EF_WPPATH:  want = b->wpPath;  break;
    }
    if (want != cur) {
        SetWindowTextW(g_editor.propEdit, want.c_str());
    }

    if (!IsWindowVisible(g_editor.propEdit)) ShowWindow(g_editor.propEdit, SW_SHOW);
}

// ============================================================
// EDITOR WINDOW
// ============================================================
LRESULT CALLBACK EditorProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    static int hoverSave = 0, hoverPreview = 0, hoverBuild = 0;

    switch (m) {
    case WM_CREATE:
        g_editorWnd = h;
        g_editor.propEdit = CreateWindowExW(0, L"EDIT", L"",
            WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            0, 0, 100, 30, h, (HMENU)1001, g_hInst, nullptr);
        {
            HFONT f = CreateFontW(13, 0, 0, 0, FW_NORMAL, 0, 0, 0,
                DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
            SendMessageW(g_editor.propEdit, WM_SETFONT, (WPARAM)f, TRUE);
        }
        SetTimer(h, 1, 30000, nullptr);
        return 0;

    case WM_TIMER:
        if (w == 1 && g_editor.modified) SaveProject();
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps; HDC dc = BeginPaint(h, &ps);
        RECT rc; GetClientRect(h, &rc);
        FillRectC(dc, rc, Colors::BG);

        RECT topbar = { 0, 0, rc.right, TOPBAR_H };
        FillRectC(dc, topbar, Colors::BG_DARK);
        std::wstring title = L"⚒ PrankForge — " + g_editor.projectName;
        if (g_editor.modified) title += L" ●";
        RECT titleRc = { 15, 0, rc.right - 300, TOPBAR_H };
        DrawTextLeft(dc, title.c_str(), titleRc, Colors::TEXT, 15, true);
        RECT saveBtn = { rc.right - 280, 8, rc.right - 180, TOPBAR_H - 8 };
        DrawCustomButton(dc, saveBtn,
            (g_language == 0) ? L"💾 Сохранить" : L"💾 Save", hoverSave, false);

        RECT sidebar = { 0, TOPBAR_H, SIDEBAR_W, rc.bottom - BOTTOM_H };
        FillRectC(dc, sidebar, Colors::BG_DARK);
        RECT sideTitle = { 15, TOPBAR_H + 10, SIDEBAR_W - 15, TOPBAR_H + 40 };
        DrawTextLeft(dc, (g_language == 0) ? L"Палитра блоков" : L"Block palette",
            sideTitle, Colors::TEXT, 14, true);
        DrawPalette(dc, TOPBAR_H + 50, 0, SIDEBAR_W);

        RECT canvas = { SIDEBAR_W, TOPBAR_H, rc.right - PROPS_W, rc.bottom - BOTTOM_H };
        FillRectC(dc, canvas, Colors::BG_CANVAS);
        HRGN hRgn = CreateRectRgn(canvas.left, canvas.top, canvas.right, canvas.bottom);
        SelectClipRgn(dc, hRgn);
        DrawGrid(dc, canvas);
        DrawConnections(dc);
        for (auto& b : g_editor.blocks) DrawBlock(dc, b);
        SelectClipRgn(dc, nullptr); DeleteObject(hRgn);

        DrawProperties(dc, h);

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

        // Синхронизация EDIT после отрисовки
        SyncPropEdit(h);
        return 0;
    }

    case WM_COMMAND: {
        if (LOWORD(w) == 1001 && HIWORD(w) == EN_CHANGE) {
            // Обновляем параметр блока на лету
            if (g_editor.editBlockId >= 0) {
                Block* b = FindBlockById(g_editor.editBlockId);
                if (b) {
                    wchar_t buf[2048] = {};
                    GetWindowTextW(g_editor.propEdit, buf, 2048);
                    switch (g_editor.editField) {
                        case EF_WELCOME: b->welcomeText = buf; break;
                        case EF_MSGTEXT: b->msgText = buf; break;
                        case EF_LINKURL: b->linkURL = buf; break;
                        case EF_WPPATH:  b->wpPath  = buf; break;
                    }
                    g_editor.modified = true;
                }
            }
            return 0;
        }
        return 0;
    }

    case WM_MOUSEMOVE: {
        int x = LOWORD(l), y = HIWORD(l); RECT rc; GetClientRect(h, &rc);
        RECT previewBtn = { rc.right - 480, 12, rc.right - 260, BOTTOM_H - 12 };
        RECT buildBtn = { rc.right - 250, 12, rc.right - 15, BOTTOM_H - 12 };
        RECT saveBtn = { rc.right - 280, 8, rc.right - 180, TOPBAR_H - 8 };
        auto ins = [](const RECT& r, int px, int py) {
            return px >= r.left && px <= r.right && py >= r.top && py <= r.bottom; };
        int np = ins(previewBtn, x, y), nb = ins(buildBtn, x, y), ns = ins(saveBtn, x, y);
        int ch = 0;
        if (np != hoverPreview) { hoverPreview = np; ch = 1; }
        if (nb != hoverBuild) { hoverBuild = nb; ch = 1; }
        if (ns != hoverSave) { hoverSave = ns; ch = 1; }

        if (x < SIDEBAR_W && y > TOPBAR_H) {
            int yy = TOPBAR_H + 60;
            int nh = -1;
            for (int i = 0; i < BT_COUNT; ++i) {
                RECT itemRc = { 5, yy, SIDEBAR_W - 5, yy + 36 };
                if (ins(itemRc, x, y)) { nh = i; break; }
                yy += 40;
                if (i == g_editor.expandedCategory) yy += 36;
            }
            if (nh != g_editor.hoveredCategory) { g_editor.hoveredCategory = nh; ch = 1; }
        } else if (g_editor.hoveredCategory != -1) {
            g_editor.hoveredCategory = -1; ch = 1;
        }

        if (g_editor.activeSlider >= 0 && g_editor.selectedId >= 0) {
            Block* b = FindBlockById(g_editor.selectedId);
            if (b && g_editor.activeSlider < g_editor.sliderCount) {
                RECT sr = g_editor.sliderRects[g_editor.activeSlider];
                int val = (x - sr.left) * 100 / (sr.right - sr.left);
                if (val < 0) val = 0; if (val > 100) val = 100;
                if (b->type == BT_GDI) {
                    if (g_editor.activeSlider == 0) b->gdiIntensity = val;
                    else if (g_editor.activeSlider == 1) b->gdiDuration = val * 60 / 100;
                } else if (b->type == BT_BYTEBEAT && g_editor.activeSlider == 0) {
                    b->bbVolume = val;
                } else if (b->type == BT_MSGBOX) {
                    if (g_editor.activeSlider == 0) b->msgCount = 1 + val * 29 / 100;
                    else if (g_editor.activeSlider == 1) b->msgDelay = val * 30;
                } else if (b->type == BT_LINK) {
                    if (g_editor.activeSlider == 0) b->linkCount = 1 + val * 9 / 100;
                    else if (g_editor.activeSlider == 1) b->linkDelay = val * 50;
                } else if (b->type == BT_REBOOT && g_editor.activeSlider == 0) {
                    b->rebootDelay = val * 30 / 100;
                }
                g_editor.modified = true;
                ch = 1;
            }
        }

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
        if (ch) InvalidateRect(h, nullptr, FALSE);

        if (np || nb || ns) SetCursor(LoadCursor(nullptr, IDC_HAND));
        else if (g_editor.activeSlider >= 0) SetCursor(LoadCursor(nullptr, IDC_HAND));
        else if (x > SIDEBAR_W && x < rc.right - PROPS_W && y > TOPBAR_H && y < rc.bottom - BOTTOM_H) {
            if (FindBlockAtScreen(x, y)) SetCursor(LoadCursor(nullptr, IDC_SIZEALL));
            else SetCursor(LoadCursor(nullptr, IDC_ARROW));
        } else SetCursor(LoadCursor(nullptr, IDC_ARROW));
        return 0;
    }

    case WM_LBUTTONDOWN: {
        int x = LOWORD(l), y = HIWORD(l); RECT rc; GetClientRect(h, &rc);
        RECT previewBtn = { rc.right - 480, 12, rc.right - 260, BOTTOM_H - 12 };
        RECT buildBtn = { rc.right - 250, 12, rc.right - 15, BOTTOM_H - 12 };
        RECT saveBtn = { rc.right - 280, 8, rc.right - 180, TOPBAR_H - 8 };
        auto ins = [](const RECT& r, int px, int py) {
            return px >= r.left && px <= r.right && py >= r.top && py <= r.bottom; };

        if (ins(saveBtn, x, y)) {
            if (SaveProject())
                MessageBoxW(h, (g_language == 0) ? L"Сохранено." : L"Saved.", L"PrankForge", MB_OK);
            InvalidateRect(h, nullptr, TRUE); return 0;
        }
        if (ins(previewBtn, x, y)) {
            MessageBoxW(h, L"Предпросмотр — Этап 5.", L"PrankForge", MB_OK); return 0;
        }
        if (ins(buildBtn, x, y)) {
            MessageBoxW(h, L"Сборка — Этап 5.", L"PrankForge", MB_OK); return 0;
        }

        // Клик по палитре — добавить блок
        if (x < SIDEBAR_W && y > TOPBAR_H + 50) {
            int yy = TOPBAR_H + 60;
            for (int i = 0; i < BT_COUNT; ++i) {
                RECT itemRc = { 5, yy, SIDEBAR_W - 5, yy + 36 };
                if (ins(itemRc, x, y)) {
                    g_editor.expandedCategory = (g_editor.expandedCategory == i) ? -1 : i;
                    if (g_editor.expandedCategory == i) {
                        Block b;
                        b.id = g_editor.nextId++;
                        b.type = (BlockType)i;
                        b.title = (g_language == 0) ? g_categories[i].ruName : g_categories[i].enName;
                        int cx = (GetCanvasLeft() + GetCanvasRight(h)) / 2;
                        int cy = (GetCanvasTop() + GetCanvasBottom(h)) / 2;
                        POINT cpt = ScreenToCanvas(cx, cy);
                        b.x = cpt.x - b.w / 2 + (int)(g_editor.blocks.size() % 5) * 30;
                        b.y = cpt.y - b.h / 2 + (int)(g_editor.blocks.size() % 5) * 30;
                        g_editor.blocks.push_back(b);
                        g_editor.selectedId = b.id;
                        g_editor.modified = true;
                    }
                    InvalidateRect(h, nullptr, TRUE); return 0;
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
                    if (ins(g_editor.buttonRects[i], x, y)) {
                        if (b->type == BT_GDI && i == 0) {
                            OpenEffectSelector(h, b->id, b->gdiEffect); return 0;
                        }
                        if (b->type == BT_BYTEBEAT && i == 0) {
                            b->bbTrack = (b->bbTrack + 1) % kByteTrackCount;
                            g_editor.modified = true; InvalidateRect(h, nullptr, TRUE); return 0;
                        }
                        if (b->type == BT_MSGBOX) {
                            if (i == 0) b->msgIcon = (b->msgIcon + 1) % 4;
                            else if (i == 1) b->msgTrajectory = (b->msgTrajectory + 1) % 5;
                            g_editor.modified = true; InvalidateRect(h, nullptr, TRUE); return 0;
                        }
                        if (b->type == BT_WALLPAPER) {
                            if (i == 0) {
                                b->wpType = (b->wpType + 1) % 2;
                                g_editor.modified = true; InvalidateRect(h, nullptr, TRUE); return 0;
                            }
                            if (i == 1 && b->wpType == 1) {
                                wchar_t file[MAX_PATH] = {};
                                OPENFILENAMEW ofn = {};
                                ofn.lStructSize = sizeof(ofn);
                                ofn.hwndOwner = h;
                                ofn.lpstrFilter = L"PNG файлы\0*.png\0Все файлы\0*.*\0";
                                ofn.lpstrFile = file;
                                ofn.nMaxFile = MAX_PATH;
                                ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
                                if (GetOpenFileNameW(&ofn)) {
                                    b->wpPath = file;
                                    g_editor.modified = true;
                                    InvalidateRect(h, nullptr, TRUE);
                                }
                                return 0;
                            }
                        }
                    }
                }

                // Чекбоксы (Registry)
                for (int i = 0; i < g_editor.checkboxCount; ++i) {
                    if (ins(g_editor.checkboxRects[i], x, y)) {
                        if (b->type == BT_REGISTRY) {
                            switch (i) {
                                case 0: b->regTaskMgr = !b->regTaskMgr; break;
                                case 1: b->regRegedit = !b->regRegedit; break;
                                case 2: b->regControlPanel = !b->regControlPanel; break;
                                case 3: b->regSettings = !b->regSettings; break;
                                case 4: b->regInternet = !b->regInternet; break;
                                case 5: b->regWarn = !b->regWarn; break;
                            }
                            g_editor.modified = true;
                            InvalidateRect(h, nullptr, TRUE);
                        }
                        return 0;
                    }
                }

                // Свотчи
                for (int i = 0; i < g_editor.swatchCount; ++i) {
                    if (ins(g_editor.swatchRects[i], x, y)) {
                        b->wpColor = i; g_editor.modified = true;
                        InvalidateRect(h, nullptr, TRUE); return 0;
                    }
                }

                // Слайдеры
                for (int i = 0; i < g_editor.sliderCount; ++i) {
                    if (ins(g_editor.sliderRects[i], x, y)) {
                        g_editor.activeSlider = i;
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
                        } else if (b->type == BT_LINK) {
                            if (i == 0) b->linkCount = 1 + val * 9 / 100;
                            else if (i == 1) b->linkDelay = val * 50;
                        } else if (b->type == BT_REBOOT && i == 0) {
                            b->rebootDelay = val * 30 / 100;
                        }
                        g_editor.modified = true;
                        InvalidateRect(h, nullptr, TRUE); return 0;
                    }
                }
            }
            return 0;
        }

        // Канвас
        if (x > SIDEBAR_W && x < rc.right - PROPS_W && y > TOPBAR_H && y < rc.bottom - BOTTOM_H) {
            Block* b = FindBlockAtScreen(x, y);
            if (b) {
                g_editor.dragging = true;
                g_editor.dragBlockId = b->id;
                POINT cpt = ScreenToCanvas(x, y);
                g_editor.dragOffsetX = cpt.x - b->x;
                g_editor.dragOffsetY = cpt.y - b->y;
                for (auto& bl : g_editor.blocks) bl.selected = false;
                b->selected = true;
                g_editor.selectedId = b->id;
            } else {
                for (auto& bl : g_editor.blocks) bl.selected = false;
                g_editor.selectedId = -1;
            }
            InvalidateRect(h, nullptr, TRUE); return 0;
        }
        return 0;
    }

    case WM_LBUTTONUP:
        g_editor.dragging = false;
        g_editor.dragBlockId = -1;
        g_editor.activeSlider = -1;
        return 0;

    case WM_MOUSEWHEEL: {
        int d = GET_WHEEL_DELTA_WPARAM(w);
        bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
        if (ctrl) {
            if (d > 0) g_editor.zoom *= 1.1f; else g_editor.zoom /= 1.1f;
            if (g_editor.zoom < 0.25f) g_editor.zoom = 0.25f;
            if (g_editor.zoom > 3.0f) g_editor.zoom = 3.0f;
        } else {
            g_editor.scrollY -= d / 3;
            if (g_editor.scrollY < 0) g_editor.scrollY = 0;
        }
        InvalidateRect(h, nullptr, TRUE); return 0;
    }

    case WM_KEYDOWN: {
        bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
        if (ctrl && w == 'S') {
            if (SaveProject())
                MessageBoxW(h, (g_language == 0) ? L"Сохранено." : L"Saved.", L"PrankForge", MB_OK);
            InvalidateRect(h, nullptr, TRUE); return 0;
        }
        if (w == VK_DELETE && g_editor.selectedId >= 0) {
            for (auto it = g_editor.blocks.begin(); it != g_editor.blocks.end(); ++it) {
                if (it->id == g_editor.selectedId) {
                    g_editor.blocks.erase(it);
                    g_editor.selectedId = -1;
                    g_editor.modified = true;
                    InvalidateRect(h, nullptr, TRUE); break;
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
    g_editor.scrollX = 0; g_editor.scrollY = 0;
    g_editor.zoom = 1.0f;
    g_editor.projectName = g_projects[projectIndex].name;
    g_editor.projectPath = g_projects[projectIndex].path;
    g_editor.modified = false;

    RECT rc = { 0, 0, 1400, 800 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    int w = rc.right - rc.left, h = rc.bottom - rc.top;
    int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;
    HWND editor = CreateWindowExW(0, CLASS_EDITOR,
        (L"PrankForge — " + g_editor.projectName).c_str(),
        WS_OVERLAPPEDWINDOW, x, y, w, h, parent, nullptr, g_hInst, nullptr);
    ShowWindow(editor, SW_SHOW); UpdateWindow(editor);
}

// ============================================================
// WELCOME / MAIN / CREATE (без изменений, сжато)
// ============================================================
LRESULT CALLBACK WelcomeProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    static int hS = 0, hR = 0, hE = 0, hD = 0, hA = 0, hX = 0;
    switch (m) {
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC dc = BeginPaint(h, &ps);
        RECT rc; GetClientRect(h, &rc);
        FillRectC(dc, rc, Colors::BG);
        RECT tb = { 0, 0, rc.right, 40 }; FillRectC(dc, tb, Colors::BG_DARK);
        RECT bc = { rc.right - 40, 0, rc.right, 40 };
        if (hX) FillRectC(dc, bc, RGB(200, 50, 50));
        DrawTextCentered(dc, L"✕", bc, Colors::TEXT, 16, false);
        RECT lg = { 0, 100, rc.right, 180 };
        DrawTextCentered(dc, L"⚒ PrankForge", lg, Colors::ACCENT, 56, true);
        RECT sl = { 0, 180, rc.right, 220 };
        DrawTextCentered(dc, L"Build your chaos.", sl, Colors::TEXT_DIM, 18, false);
        RECT wl = { 0, 260, rc.right, 300 };
        DrawTextCentered(dc, (g_language == 0)
            ? L"Добро пожаловать в конструктор пранков"
            : L"Welcome to the prank builder", wl, Colors::TEXT, 22, false);
        RECT ds = { 60, 310, rc.right - 60, 400 };
        const wchar_t* desc = (g_language == 0)
            ? L"Собирайте пранки из блоков."
            : L"Build pranks from blocks.";
        SetBkMode(dc, TRANSPARENT); SetTextColor(dc, Colors::TEXT_DIM);
        HFONT f = CreateFontW(15, 0, 0, 0, FW_NORMAL, 0, 0, 0,
            DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
        HFONT old = (HFONT)SelectObject(dc, f);
        DrawTextW(dc, desc, -1, &ds, DT_CENTER | DT_TOP | DT_WORDBREAK);
        SelectObject(dc, old); DeleteObject(f);
        RECT sb = { rc.right / 2 - 100, 430, rc.right / 2 + 100, 480 };
        DrawCustomButton(dc, sb, (g_language == 0) ? L"Начать" : L"Start", hS, true);
        RECT rb = { 30, rc.bottom - 60, 110, rc.bottom - 30 };
        RECT eb = { 120, rc.bottom - 60, 200, rc.bottom - 30 };
        DrawCustomButton(dc, rb, L"Русский", hR, g_language == 0);
        DrawCustomButton(dc, eb, L"English", hE, g_language == 1);
        RECT db = { 240, rc.bottom - 60, 360, rc.bottom - 30 };
        RECT ab = { 370, rc.bottom - 60, 490, rc.bottom - 30 };
        DrawCustomButton(dc, db, (g_language == 0) ? L"Документация" : L"Docs", hD, false);
        DrawCustomButton(dc, ab, (g_language == 0) ? L"О программе" : L"About", hA, false);
        RECT vr = { rc.right - 120, rc.bottom - 40, rc.right - 10, rc.bottom - 10 };
        DrawTextCentered(dc, L"v1.0.0", vr, Colors::TEXT_DIM, 12, false);
        HPEN pen = CreatePen(PS_SOLID, 2, Colors::ACCENT);
        HPEN oldPen = (HPEN)SelectObject(dc, pen);
        for (int i = 0; i < 5; ++i) {
            int yy = 250 + i * 8;
            MoveToEx(dc, 80, yy, nullptr);
            for (int x = 80; x < rc.right - 80; x += 10)
                LineTo(dc, x, yy + (int)(5 * sin((x + i * 30) * 0.05)));
        }
        SelectObject(dc, oldPen); DeleteObject(pen);
        EndPaint(h, &ps); return 0;
    }
    case WM_MOUSEMOVE: {
        int x = LOWORD(l), y = HIWORD(l); RECT rc; GetClientRect(h, &rc);
        RECT sb = { rc.right / 2 - 100, 430, rc.right / 2 + 100, 480 };
        RECT rb = { 30, rc.bottom - 60, 110, rc.bottom - 30 };
        RECT eb = { 120, rc.bottom - 60, 200, rc.bottom - 30 };
        RECT db = { 240, rc.bottom - 60, 360, rc.bottom - 30 };
        RECT ab = { 370, rc.bottom - 60, 490, rc.bottom - 30 };
        RECT xb = { rc.right - 40, 0, rc.right, 40 };
        auto ins = [](const RECT& r, int px, int py) {
            return px >= r.left && px <= r.right && py >= r.top && py <= r.bottom; };
        int ns = ins(sb,x,y), nr = ins(rb,x,y), ne = ins(eb,x,y);
        int nd = ins(db,x,y), na = ins(ab,x,y), nx = ins(xb,x,y);
        if (ns!=hS||nr!=hR||ne!=hE||nd!=hD||na!=hA||nx!=hX) {
            hS=ns; hR=nr; hE=ne; hD=nd; hA=na; hX=nx;
            InvalidateRect(h,nullptr,FALSE);
        }
        if (ns||nr||ne||nd||na||nx) SetCursor(LoadCursor(nullptr,IDC_HAND));
        else SetCursor(LoadCursor(nullptr,IDC_ARROW));
        return 0;
    }
    case WM_LBUTTONDOWN: {
        int x = LOWORD(l), y = HIWORD(l); RECT rc; GetClientRect(h, &rc);
        RECT sb = { rc.right / 2 - 100, 430, rc.right / 2 + 100, 480 };
        RECT rb = { 30, rc.bottom - 60, 110, rc.bottom - 30 };
        RECT eb = { 120, rc.bottom - 60, 200, rc.bottom - 30 };
        RECT xb = { rc.right - 40, 0, rc.right, 40 };
        auto ins = [](const RECT& r, int px, int py) {
            return px >= r.left && px <= r.right && py >= r.top && py <= r.bottom; };
        if (ins(sb,x,y)) { DestroyWindow(h); extern void OpenMainWindow(); OpenMainWindow(); return 0; }
        if (ins(rb,x,y)) { g_language = 0; InvalidateRect(h,nullptr,TRUE); return 0; }
        if (ins(eb,x,y)) { g_language = 1; InvalidateRect(h,nullptr,TRUE); return 0; }
        if (ins(xb,x,y)) { PostQuitMessage(0); return 0; }
        return 0;
    }
    case WM_DESTROY: if (!g_mainWnd) PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(h, m, w, l);
}

HWND g_createEditName = nullptr;
HWND g_createEditPath = nullptr;

LRESULT CALLBACK CreateProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    static int hB = 0, hO = 0, hC = 0;
    switch (m) {
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC dc = BeginPaint(h, &ps);
        RECT rc; GetClientRect(h, &rc);
        FillRectC(dc, rc, Colors::BG);
        RECT t = { 20, 15, rc.right - 20, 50 };
        DrawTextLeft(dc, (g_language == 0) ? L"Создание проекта" : L"Create project", t, Colors::TEXT, 20, true);
        RECT l1 = { 20, 70, rc.right - 20, 95 };
        DrawTextLeft(dc, (g_language == 0) ? L"Название:" : L"Name:", l1, Colors::TEXT_DIM, 14, false);
        RECT l2 = { 20, 140, rc.right - 20, 165 };
        DrawTextLeft(dc, (g_language == 0) ? L"Расположение:" : L"Location:", l2, Colors::TEXT_DIM, 14, false);
        RECT bb = { rc.right - 120, 165, rc.right - 20, 200 };
        DrawCustomButton(dc, bb, L"...", hB, false);
        RECT cb = { rc.right - 240, rc.bottom - 60, rc.right - 140, rc.bottom - 20 };
        RECT ob = { rc.right - 130, rc.bottom - 60, rc.right - 20, rc.bottom - 20 };
        DrawCustomButton(dc, cb, (g_language == 0) ? L"Отмена" : L"Cancel", hC, false);
        DrawCustomButton(dc, ob, (g_language == 0) ? L"Создать" : L"Create", hO, true);
        EndPaint(h, &ps); return 0;
    }
    case WM_CTLCOLOREDIT: {
        HDC dc = (HDC)w;
        SetTextColor(dc, Colors::TEXT); SetBkColor(dc, Colors::BG_PANEL);
        static HBRUSH br = CreateSolidBrush(Colors::BG_PANEL);
        return (LRESULT)br;
    }
    case WM_CREATE: {
        g_createEditName = CreateWindowExW(0, L"EDIT", L"",
            WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL, 20, 100, 560, 32, h, nullptr, g_hInst, nullptr);
        g_createEditPath = CreateWindowExW(0, L"EDIT", L"",
            WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL, 20, 170, 440, 32, h, nullptr, g_hInst, nullptr);
        HFONT f = CreateFontW(15,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
        SendMessageW(g_createEditName, WM_SETFONT, (WPARAM)f, TRUE);
        SendMessageW(g_createEditPath, WM_SETFONT, (WPARAM)f, TRUE);
        return 0;
    }
    case WM_MOUSEMOVE: {
        int x = LOWORD(l), y = HIWORD(l); RECT rc; GetClientRect(h, &rc);
        RECT bb = { rc.right - 120, 165, rc.right - 20, 200 };
        RECT cb = { rc.right - 240, rc.bottom - 60, rc.right - 140, rc.bottom - 20 };
        RECT ob = { rc.right - 130, rc.bottom - 60, rc.right - 20, rc.bottom - 20 };
        auto ins = [](const RECT& r, int px, int py) {
            return px >= r.left && px <= r.right && py >= r.top && py <= r.bottom; };
        int nb=ins(bb,x,y),nc=ins(cb,x,y),no=ins(ob,x,y);
        if (nb!=hB||nc!=hC||no!=hO) { hB=nb;hC=nc;hO=no; InvalidateRect(h,nullptr,FALSE); }
        if (nb||nc||no) SetCursor(LoadCursor(nullptr,IDC_HAND));
        else SetCursor(LoadCursor(nullptr,IDC_ARROW));
        return 0;
    }
    case WM_LBUTTONDOWN: {
        int x = LOWORD(l), y = HIWORD(l); RECT rc; GetClientRect(h, &rc);
        RECT bb = { rc.right - 120, 165, rc.right - 20, 200 };
        RECT cb = { rc.right - 240, rc.bottom - 60, rc.right - 140, rc.bottom - 20 };
        RECT ob = { rc.right - 130, rc.bottom - 60, rc.right - 20, rc.bottom - 20 };
        auto ins = [](const RECT& r, int px, int py) {
            return px >= r.left && px <= r.right && py >= r.top && py <= r.bottom; };
        if (ins(bb,x,y)) {
            BROWSEINFOW bi = {}; bi.hwndOwner = h;
            bi.lpszTitle = L"Выбери папку"; bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
            LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
            if (pidl) { wchar_t p[MAX_PATH]; SHGetPathFromIDListW(pidl, p);
                SetWindowTextW(g_createEditPath, p); CoTaskMemFree(pidl); }
            return 0;
        }
        if (ins(ob,x,y)) {
            wchar_t name[256] = {}, path[MAX_PATH] = {};
            GetWindowTextW(g_createEditName, name, 256);
            GetWindowTextW(g_createEditPath, path, MAX_PATH);
            if (!wcslen(name)) { MessageBoxW(h, L"Введи название.", L"!", MB_OK); return 0; }
            wchar_t pd[MAX_PATH]; swprintf_s(pd, L"%s\\%s", path, name);
            CreateDirectoryW(pd, nullptr);
            wchar_t pf[MAX_PATH]; swprintf_s(pf, L"%s\\project.pforge", pd);
            HANDLE f = CreateFileW(pf, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (f != INVALID_HANDLE_VALUE) {
                const char* j = "{\n  \"name\": \"\",\n  \"blocks\": []\n}\n";
                DWORD wr; WriteFile(f, j, (DWORD)strlen(j), &wr, nullptr); CloseHandle(f);
            }
            ProjectInfo pi;
            pi.name = name; pi.path = pd;
            SYSTEMTIME st; GetLocalTime(&st);
            wchar_t d[64]; swprintf_s(d, L"%04d-%02d-%02d %02d:%02d",
                st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
            pi.date = d;
            g_projects.push_back(pi);
            if (g_mainWnd) InvalidateRect(g_mainWnd, nullptr, TRUE);
            DestroyWindow(h); return 0;
        }
        if (ins(cb,x,y)) { DestroyWindow(h); return 0; }
        return 0;
    }
    case WM_CLOSE: DestroyWindow(h); return 0;
    }
    return DefWindowProcW(h, m, w, l);
}

LRESULT CALLBACK MainProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    static int hC = 0, hS = 0, hP = -1;
    switch (m) {
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC dc = BeginPaint(h, &ps);
        RECT rc; GetClientRect(h, &rc);
        FillRectC(dc, rc, Colors::BG);
        RECT sb = { 0, 0, 250, rc.bottom }; FillRectC(dc, sb, Colors::BG_DARK);
        RECT lg = { 0, 20, 250, 70 };
        DrawTextCentered(dc, L"⚒ PrankForge", lg, Colors::ACCENT, 22, true);
        const wchar_t* items[] = {
            (g_language == 0) ? L"📁 Проекты" : L"📁 Projects",
            (g_language == 0) ? L"📋 Шаблоны" : L"📋 Templates",
            (g_language == 0) ? L"🎨 Эффекты" : L"🎨 Effects",
            (g_language == 0) ? L"📜 История" : L"📜 History"
        };
        for (int i = 0; i < 4; ++i) {
            RECT ir = { 10, 100 + i * 45, 240, 140 + i * 45 };
            if (i == 0) FillRectC(dc, ir, Colors::BG_PANEL);
            DrawTextLeft(dc, items[i], ir, Colors::TEXT, 15, false);
        }
        RECT sBtn = { 10, rc.bottom - 60, 240, rc.bottom - 15 };
        DrawCustomButton(dc, sBtn,
            (g_language == 0) ? L"⚙ Настройки" : L"⚙ Settings", hS, false);
        RECT tb = { 250, 0, rc.right, 70 }; FillRectC(dc, tb, Colors::BG_DARK);
        RECT tR = { 270, 15, 500, 55 };
        DrawTextLeft(dc, (g_language == 0) ? L"Мои проекты" : L"My projects", tR, Colors::TEXT, 22, true);
        RECT cBtn = { rc.right - 280, 15, rc.right - 20, 55 };
        DrawCustomButton(dc, cBtn,
            (g_language == 0) ? L"+ Создать проект" : L"+ Create project", hC, true);
        int y = 100;
        if (g_projects.empty()) {
            RECT er = { 250, rc.bottom / 2 - 40, rc.right, rc.bottom / 2 + 40 };
            DrawTextCentered(dc, (g_language == 0)
                ? L"Нет проектов. Нажми «Создать»." : L"No projects yet.",
                er, Colors::TEXT_DIM, 18, false);
        } else {
            for (size_t i = 0; i < g_projects.size(); ++i) {
                RECT card = { 270, y, rc.right - 20, y + 90 };
                FillRectC(dc, card, ((int)i == hP) ? Colors::BG_HOVER : Colors::BG_PANEL);
                HBRUSH br = CreateSolidBrush(Colors::BORDER);
                FrameRect(dc, &card, br); DeleteObject(br);
                RECT nr = { card.left + 20, card.top + 10, card.right - 20, card.top + 45 };
                DrawTextLeft(dc, g_projects[i].name.c_str(), nr, Colors::TEXT, 18, true);
                RECT ir = { card.left + 20, card.top + 45, card.right - 20, card.top + 80 };
                std::wstring info = g_projects[i].path + L"   ·   " + g_projects[i].date;
                DrawTextLeft(dc, info.c_str(), ir, Colors::TEXT_DIM, 13, false);
                y += 100;
            }
        }
        EndPaint(h, &ps); return 0;
    }
    case WM_MOUSEMOVE: {
        int x = LOWORD(l), y = HIWORD(l); RECT rc; GetClientRect(h, &rc);
        RECT cB = { rc.right - 280, 15, rc.right - 20, 55 };
        RECT sB = { 10, rc.bottom - 60, 240, rc.bottom - 15 };
        auto ins = [](const RECT& r, int px, int py) {
            return px >= r.left && px <= r.right && py >= r.top && py <= r.bottom; };
        int nc = ins(cB,x,y), ns = ins(sB,x,y);
        int hp = -1;
        if (x > 250) {
            int yy = 100;
            for (size_t i = 0; i < g_projects.size(); ++i) {
                RECT card = { 270, yy, rc.right - 20, yy + 90 };
                if (ins(card,x,y)) { hp = (int)i; break; }
                yy += 100;
            }
        }
        if (nc!=hC||ns!=hS||hp!=hP) { hC=nc;hS=ns;hP=hp; InvalidateRect(h,nullptr,FALSE); }
        if (nc||ns||hp>=0) SetCursor(LoadCursor(nullptr,IDC_HAND));
        else SetCursor(LoadCursor(nullptr,IDC_ARROW));
        return 0;
    }
    case WM_LBUTTONDOWN: {
        int x = LOWORD(l), y = HIWORD(l); RECT rc; GetClientRect(h, &rc);
        RECT cB = { rc.right - 280, 15, rc.right - 20, 55 };
        auto ins = [](const RECT& r, int px, int py) {
            return px >= r.left && px <= r.right && py >= r.top && py <= r.bottom; };
        if (ins(cB,x,y)) {
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

void RegisterClasses() {
    WNDCLASSW wc = {};
    wc.hInstance = g_hInst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;

    wc.lpfnWndProc = WelcomeProc;   wc.lpszClassName = CLASS_WELCOME;   wc.style = CS_DBLCLKS; RegisterClassW(&wc);
    wc.lpfnWndProc = MainProc;      wc.lpszClassName = CLASS_MAIN;      wc.style = CS_DBLCLKS; RegisterClassW(&wc);
    wc.lpfnWndProc = CreateProc;    wc.lpszClassName = CLASS_CREATE;    wc.style = 0;          RegisterClassW(&wc);
    wc.lpfnWndProc = EditorProc;    wc.lpszClassName = CLASS_EDITOR;    wc.style = CS_DBLCLKS; RegisterClassW(&wc);
    wc.lpfnWndProc = EffectSelProc; wc.lpszClassName = CLASS_EFFECTSEL; wc.style = 0;          RegisterClassW(&wc);
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int) {
    g_hInst = hInst;
    HANDLE hM = CreateMutexW(nullptr, TRUE, L"PrankForgeMutex_v1");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBoxW(nullptr, L"PrankForge уже запущен.", L"PrankForge", MB_OK); return 0;
    }
    SetProcessDPIAware();
    RegisterClasses();

    RECT rc = { 0, 0, 700, 540 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, FALSE);
    int w = rc.right - rc.left, h = rc.bottom - rc.top;
    int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

    HWND welcome = CreateWindowExW(0, CLASS_WELCOME, L"PrankForge — Добро пожаловать",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, x, y, 700, 540, nullptr, nullptr, hInst, nullptr);
    ShowWindow(welcome, SW_SHOW); UpdateWindow(welcome);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg); DispatchMessageW(&msg);
    }
    if (hM) { ReleaseMutex(hM); CloseHandle(hM); }
    return 0;
}
