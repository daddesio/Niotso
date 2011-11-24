#include "../Windows.hpp"
#include "../GUI.hpp"

#ifndef TTI_INFO_LARGE
#define TTI_INFO_LARGE 4
#endif

namespace NewArchive {

enum {TYPE_FAR, TYPE_DBPF};
void SetType(HWND hDlg, int type){
    for(int i=IDC_NA_FARVERSIONTEXT; i<=IDC_NA_FARVERSION; i++)
        EnableWindow(GetDlgItem(hDlg, i), type==TYPE_FAR);
    for(int i=IDC_NA_DBPFVERSIONTEXT; i<=IDC_NA_DBPFCOMPRESS; i++)
        EnableWindow(GetDlgItem(hDlg, i), type==TYPE_DBPF);
}

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
	switch (message){
    case WM_INITDIALOG: {
        CenterDialog(hDlg);
        
        //Fill the drop-down boxes
        HWND hBox = GetDlgItem(hDlg, IDC_NA_TYPE);
        SendMessage(hBox, CB_INSERTSTRING, -1, (LPARAM) L"FAR");
        SendMessage(hBox, CB_INSERTSTRING, -1, (LPARAM) L"DBPF");
        SendMessage(hBox, CB_SETCURSEL, 0, 0);
        hBox = GetDlgItem(hDlg, IDC_NA_FARVERSION);
        SendMessage(hBox, CB_INSERTSTRING, -1, (LPARAM) L"1a");
        SendMessage(hBox, CB_INSERTSTRING, -1, (LPARAM) L"1b");
        SendMessage(hBox, CB_INSERTSTRING, -1, (LPARAM) L"3");
        SendMessage(hBox, CB_SETCURSEL, 0, 0);
        hBox = GetDlgItem(hDlg, IDC_NA_DBPFVERSION);
        SendMessage(hBox, CB_INSERTSTRING, -1, (LPARAM) L"1.0");
        SendMessage(hBox, CB_INSERTSTRING, -1, (LPARAM) L"1.1");
        SendMessage(hBox, CB_INSERTSTRING, -1, (LPARAM) L"2.0");
        SendMessage(hBox, CB_SETCURSEL, 0, 0);
        hBox = GetDlgItem(hDlg, IDC_NA_INDEXVERSION);
        SendMessage(hBox, CB_INSERTSTRING, -1, (LPARAM) L"7.0");
        SendMessage(hBox, CB_INSERTSTRING, -1, (LPARAM) L"7.1");
        SendMessage(hBox, CB_INSERTSTRING, -1, (LPARAM) L"3.0");
        SendMessage(hBox, CB_SETCURSEL, 0, 0);
        
        //Create the tooltips
        HWND FARInfo = CreateWindowEx(0, TOOLTIPS_CLASS, NULL,
			WS_POPUP | TTS_NOPREFIX | TTS_BALLOON | TTS_ALWAYSTIP,
			CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT,
			hDlg, NULL, hInst, NULL),
		DBPFInfo = CreateWindowEx(0, TOOLTIPS_CLASS, NULL,
			WS_POPUP | TTS_NOPREFIX | TTS_BALLOON | TTS_ALWAYSTIP,
			CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT,
			hDlg, NULL, hInst, NULL);
        
        TOOLINFO tinfo = {
            sizeof(TOOLINFO),            //cbSize
            TTF_IDISHWND | TTF_SUBCLASS, //uFlags
            hDlg,                        //hwnd
            0,                           //uId
            NULL,                        //rect
            0                            //hinst
        };
        
        tinfo.uId = (UINT_PTR) GetDlgItem(hDlg, IDC_NA_FARINFO);
        tinfo.lpszText = (wchar_t*)
            L"FAR version 1a is found in The Sims 1.\r\n\r\n"
            L"Version 1b appears to be a mistake, in which it was intended to take on the version number 2.\r\n\r\n"
            L"1b and 3 are both found exclusively in The Sims Online.";
        SendMessage(FARInfo, TTM_SETMAXTIPWIDTH, 2000, 200);
		SendMessage(FARInfo, TTM_SETTITLE, TTI_INFO_LARGE, (LPARAM) L"FAR version");
		SendMessage(FARInfo, TTM_ADDTOOL, 0, (LPARAM) &tinfo);
        SendMessage(FARInfo, TTM_SETDELAYTIME, TTDT_AUTOPOP, 12000);
        tinfo.uId = (UINT_PTR) GetDlgItem(hDlg, IDC_NA_DBPFINFO);
        tinfo.lpszText = (wchar_t*)
            L"DBPF version numbers can be shortened to the form a.b;ix,y, where "
            L"a.b is the Archive version and x.y is the Index version.\r\n\r\n"
            L"DBPF 1.0;i7.0 is found in The Sims Online and SimCity 4.\r\n\r\n"
            L"1.0;i7.0 and 1.1;i7.1 are found in The Sims 2.\r\n\r\n"
            L"2.0;i3.0 is found in Spore.";
        SendMessage(DBPFInfo, TTM_SETMAXTIPWIDTH, 2000, 200);
		SendMessage(DBPFInfo, TTM_SETTITLE, TTI_INFO_LARGE, (LPARAM) L"DBPF version");
		SendMessage(DBPFInfo, TTM_ADDTOOL, 0, (LPARAM) &tinfo);
        SendMessage(DBPFInfo, TTM_SETDELAYTIME, TTDT_AUTOPOP, 20000);
        
        SetType(hDlg, TYPE_FAR);
        } return TRUE;
    case WM_CTLCOLORSTATIC:
		if((HWND) lParam == GetDlgItem(hDlg, IDC_NA_TYPETEXT)){
			SetBkColor((HDC) wParam, GetSysColor(COLOR_WINDOW));
			return (INT_PTR) GetSysColorBrush(COLOR_WINDOW);
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
        case IDC_NA_TYPE:
            if(HIWORD(wParam) == CBN_SELCHANGE)
                SetType(hDlg, SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0));
            break;
        case IDOK:
            Archive::IsOpen = true;
            Archive::IsModified = true;
            
            Archive::Path[0] = '\0'; //No actual path
            wcscpy(Archive::Filename, L"Untitled");
            Archive::SetWorkspace();
            
            EndDialog(hDlg, 0);
        case IDCANCEL:
            EndDialog(hDlg, 0);
        } break;
    case WM_CLOSE:
		EndDialog(hDlg, 0);
    }
    return 0;
}

}