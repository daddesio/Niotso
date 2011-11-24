#include "Windows.hpp"
#include "GUI.hpp"

HWND hList;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message){

	case WM_SIZE:
		SendMessage(statusbar, WM_SIZE, 0, 0);
		
		if(Archive::IsOpen)
			SetWindowPos(hList, 0, 0, 0, 192, HIWORD(lParam)-statusbarheight-10,
                SWP_ASYNCWINDOWPOS | SWP_NOMOVE | SWP_NOOWNERZORDER);
		break;
        
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case ID_FILE_NEW:
            if(Archive::Close())
                DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_NEWARCHIVE), hWnd, NewArchive::DlgProc, NULL);
			break;
		case ID_FILE_OPEN:
            if(!Archive::Close()) break;
            ofn.hwndOwner = hWnd;
            ofn.lpstrFilter = FILTER_ARCHIVES_FILES;
            ofn.lpstrCustomFilter = ArchiveOpenFilter;
            ofn.lpstrFile = Archive::Path;
            ofn.lpstrFileTitle = NULL;
            ofn.Flags = OFN_DONTADDTORECENT | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			if(GetOpenFileName(&ofn))
                Archive::Open();
			break;
		case ID_FILE_SAVE:
            Archive::Save();
			break;
        case ID_FILE_SAVEAS:
            Archive::SaveAs();
			break;
        case ID_FILE_ADD: {
            DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ADDTOARCHIVE_FAR1), hWnd, AddToArchive::DlgProc, 0);
            } break;
		case ID_HELP_ABOUT:
			MessageBox(hWnd, L"FARDive version " FDVERSION
                L"\r\n\r\nThis is an alpha release of FARDive. The About box is not yet complete.\r\n\r\n"
                L"Don't worry, file writing will not be implemented until it is guaranteed stable.\r\n\r\n"
                L"Saving the archive through File -> Save (or Save as) will not actually "
                L"take place until we reach that point.\r\n\r\n"
                L"=-----=\r\n\r\n"
                L"FARDive - Copyright (c) 2011 Fatbag <X-Fi6@phppoll.org>\r\n\r\n"
                L"Permission to use, copy, modify, and/or distribute this software for any "
                L"purpose with or without fee is hereby granted, provided that the above "
                L"copyright notice and this permission notice appear in all copies.\r\n\r\n"
                L"THE SOFTWARE IS PROVIDED \"AS IS\" AND THE AUTHOR DISCLAIMS ALL WARRANTIES "
                L"WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF "
                L"MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR "
                L"ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES "
                L"WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN "
                L"ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF "
                L"OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.\r\n\r\n", L"About", MB_OK);
			break;
        case ID_FILE_CLOSE:
            Archive::Close();
            break;
		case ID_FILE_EXIT:
            if(Archive::Close())
			    PostQuitMessage(0);
			break;
		}
		break;
    
    case WM_NOTIFY: {
        NMHDR *nmhdr = (NMHDR*) lParam;
        if(nmhdr->hwndFrom == hList){
            switch(nmhdr->code){
            case LVN_ITEMCHANGED: {
                unsigned selected = SendMessage(hList, LVM_GETSELECTEDCOUNT, 0, 0);
                
                MENUITEMINFO mii;
                mii.cbSize = sizeof(MENUITEMINFO);
                mii.fMask = MIIM_STATE;
                
                mii.fState = (selected > 0) ? MFS_ENABLED : MFS_DISABLED;
                SetMenuItemInfo(hMenu, ID_FILE_EXPORTSELECTED, FALSE, &mii);
                SetMenuItemInfo(hMenu, ID_EDIT_DUPLICATE, FALSE, &mii);
                SetMenuItemInfo(hMenu, ID_EDIT_REMOVE, FALSE, &mii);
                
                mii.fState = (selected == 1) ? MFS_ENABLED : MFS_DISABLED;
                SetMenuItemInfo(hMenu, ID_EDIT_RENAME, FALSE, &mii);
                SetMenuItemInfo(hMenu, ID_EDIT_PROPERTIES, FALSE, &mii);
                SetMenuItemInfo(hMenu, ID_EDIT_CONTENTS, FALSE, &mii);
            } break;
            
            case NM_RCLICK: {
                unsigned selected = SendMessage(hList, LVM_GETSELECTEDCOUNT, 0, 0);
                
                if(listmenu) DestroyMenu(listmenu);
                listmenu = CreatePopupMenu();
                
                #define AddSeparator() \
                    mii.fMask = MIIM_TYPE; \
                    InsertMenuItem(listmenu, ++position, TRUE, &mii); \
                    mii.fMask = MIIM_ID | MIIM_STRING | MIIM_BITMAP
                #define AddItem(x,y,z) \
                    mii.wID = x; \
                    mii.dwTypeData = (wchar_t*) y; \
                    mii.hbmpItem = PNGIcon[z]; \
                    InsertMenuItem(listmenu, ++position, TRUE, &mii)
                
                unsigned position = 0;
                MENUITEMINFO mii;
                mii.cbSize = sizeof(MENUITEMINFO);
                mii.fMask = MIIM_ID | MIIM_STRING | MIIM_BITMAP;
                mii.fType = MFT_SEPARATOR;
                
                AddItem(ID_LISTMENU_ADD, L"A&dd to archive", BMP_FILE_ADD);
                
                if(selected > 0){
                    AddSeparator();
                    AddItem(ID_LISTMENU_EXPORTSELECTED, L"Export &selected...", BMP_FILE_EXPORTSELECTED);
                    AddItem(ID_LISTMENU_DUPLICATE, L"D&uplicate", BMP_EDIT_DUPLICATE);
                    AddItem(ID_LISTMENU_REMOVE, L"&Remove", BMP_EDIT_REMOVE);
                    if(selected == 1){
                        AddSeparator();
                        AddItem(ID_LISTMENU_RENAME, L"Re&name", BMP_EDIT_RENAME);
                        AddItem(ID_LISTMENU_PROPERTIES, L"Change pr&operties...", BMP_EDIT_PROPERTIES);
                        AddItem(ID_LISTMENU_CONTENTS, L"Change file &contents...", BMP_EDIT_CONTENTS);
                    }
                }
                
                POINT p;
                GetCursorPos(&p);
                TrackPopupMenu(listmenu, TPM_RIGHTBUTTON, p.x, p.y, 0, hWnd, NULL);
                PostMessage(hWnd, WM_NULL, 0, 0);
            } break;
            }
        }
        } break;
    
    case WM_DROPFILES:
        if(!Archive::IsOpen){
            //Open as an archive
            unsigned strlen = DragQueryFile((HDROP) wParam, 0, Archive::Path, 1024);
            DragFinish((HDROP) wParam);
            if(strlen)
                Archive::Open();
        }else{
            //Add to the current archive
            wchar_t EntryPath[1024];
            unsigned strlen = DragQueryFile((HDROP) wParam, 0, EntryPath, 1024);
            DragFinish((HDROP) wParam);
            DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ADDTOARCHIVE_FAR1), hWnd, AddToArchive::DlgProc, (LPARAM) EntryPath);
        }
        break;

	case WM_CLOSE:
        if(Archive::Close())
		    PostQuitMessage(0);
		break;
        
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}