#include "Windows.hpp"
#include "GUI.hpp"
#include <png.h>

HBITMAP PNGIcon[MENUICONS];

HINSTANCE hInst;

HWND hWnd, statusbar;
HDC hDC;
unsigned int statusbarheight;

OPENFILENAME ofn;

wchar_t ArchiveOpenFilter[128] = L"", ArchiveAddFilter[128] = L"", ArchiveSaveFilter[128] = L"";

HMENU hMenu, listmenu = 0;

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int){
	hInst = hInstance;
	
	WNDCLASS wc = {
        CS_HREDRAW | CS_VREDRAW,        //style
        WndProc,                        //lpfnWndProc
        0,                              //cbClsExtra
        0,                              //cbWndExtra
        hInst,                          //hInstance
        (HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_FARDIVE), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR), //hIcon
        (HCURSOR) LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_SHARED | LR_DEFAULTSIZE), //hCursor
        (HBRUSH) (COLOR_MENU+1),        //hbrBackground
        MAKEINTRESOURCE(IDM_FARDIVE),   //lpszMenuName
        L"F"                            //lpszClassName
    };
	RegisterClass(&wc);
	
	hWnd = CreateWindowEx(WS_EX_ACCEPTFILES | WS_EX_COMPOSITED, L"F", L"FARDive",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 616, 616, 0, 0, hInst, 0);
	statusbar = CreateWindowEx(WS_EX_COMPOSITED, L"msctls_statusbar32", NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hWnd, 0, hInst, 0);
	RECT rect;
	GetWindowRect(statusbar, &rect);
	statusbarheight = rect.bottom - rect.top;
    
    hDC = GetDC(NULL);
	hMenu = GetMenu(hWnd);
	
	memset(&ofn, sizeof(ofn), 0x00);
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.nMaxCustFilter = 128;
	ofn.nMaxFile = 1024;
    ofn.nMaxFileTitle = 1024;
	ofn.lpstrDefExt = L"dat";

    MENUITEMINFO mii;
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_BITMAP;
    
    static const short iconmenulist[] = {
        2100, 2101, 2102, 2103, 2104, 2105, 2106, 2107,
        2108, 2109, 2200, 2201, 2202, 2203, 2204, 2205,
        2206, 2207, 2208, 2209, 2210, 2300, 2301, 2302,
        2303, 2400
    };
	
	for(unsigned i=0; i<MENUICONS; i++){
        int ID = iconmenulist[i];
        mii.hbmpItem = PNGIcon[i] = ReadPNGIcon(ID);
		SetMenuItemInfo(hMenu, ID-1000, FALSE, &mii);
	}
	
    HACCEL acceltable = LoadAccelerators(hInst, MAKEINTRESOURCE(ID_ACCELERATOR));
	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0)){
        TranslateAccelerator(hWnd, acceltable, &msg);
		DispatchMessage(&msg);
    }
	
	return 0;
}