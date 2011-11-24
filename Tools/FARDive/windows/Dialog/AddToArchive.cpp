#include "../Windows.hpp"
#include "../GUI.hpp"

namespace AddToArchive {

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
	switch (message){
    case WM_INITDIALOG: {
        CenterDialog(hDlg);
        
        HWND hBox = GetDlgItem(hDlg, IDC_A2A_FILE);
        SendMessage(hBox, EM_SETLIMITTEXT, 1023, 0);
        hBox = GetDlgItem(hDlg, IDC_A2A_FILENAME);
        SendMessage(hBox, EM_SETLIMITTEXT, 1023, 0);
        
        if(lParam){
            HWND hBox = GetDlgItem(hDlg, IDC_A2A_FILE);
            SetWindowText(hBox, (wchar_t*) lParam);
            hBox = GetDlgItem(hDlg, IDC_A2A_FILENAME);
            wchar_t Filename[1024];
            GetFileTitle((wchar_t*) lParam, Filename, 1024);
            SetWindowText(hBox, Filename);
        }
        
        } return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
        case IDC_A2A_BROWSE: {
            wchar_t Path[1024] = L"", Filename[1024] = L"";
            ofn.hwndOwner = hDlg;
            ofn.lpstrFilter = FILTER_FILES;
            ofn.lpstrCustomFilter = ArchiveAddFilter;
            ofn.lpstrFile = Path;
            ofn.lpstrFileTitle = Filename;
            ofn.Flags = OFN_DONTADDTORECENT | OFN_HIDEREADONLY;
			if(GetOpenFileName(&ofn)){
                SetWindowText(GetDlgItem(hDlg, IDC_A2A_FILE), Path);
                SetWindowText(GetDlgItem(hDlg, IDC_A2A_FILENAME), Filename);
            }
            } break;
        case IDOK:
            EndDialog(hDlg, true);
            break;
        case IDCANCEL:
            EndDialog(hDlg, false);
        } break;
    case WM_CLOSE:
		EndDialog(hDlg, false);
    }
    return 0;
}

}