#include "Windows.hpp"
#include <uxtheme.h>
#include "GUI.hpp"

namespace Archive {

wchar_t Path[1024], Filename[1024];

bool IsOpen = false;
bool IsModified;

bool Add(const wchar_t * Path){
    return true;
}

bool Close(){
    if(!Archive::IsOpen) return true;
    
    if(Archive::IsModified){
        //Ask for consent
        int result = MessageBox(hWnd, L"Save changes?", L"Save changes?", MB_YESNOCANCEL);
        if(result == IDYES){
            if(!Archive::Save())
                return false;
        }else if(result != IDNO)
            return false;
    }
    
    //Close the workspace
    
    Archive::IsOpen = false;

	SetWindowText(hWnd, L"FARDive");
	SendMessage(statusbar, SB_SETTEXT, 0, (LPARAM) L"");
    
    MENUITEMINFO mii;
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_STATE;
    mii.fState = MFS_DISABLED;
    
    int DisableItems[] = {
        ID_FILE_SAVE,
        ID_FILE_SAVEAS,
        ID_FILE_ADD,
        ID_FILE_EXPORTALL,
        ID_FILE_EXPORTSELECTED,
        ID_FILE_CHANGETYPE,
        ID_FILE_CLOSE,
        ID_EDIT_DUPLICATE,
        ID_EDIT_REMOVE,
        ID_EDIT_RENAME,
        ID_EDIT_PROPERTIES,
        ID_EDIT_CONTENTS,
        ID_EDIT_FIND,
        ID_EDIT_FINDNEXT,
        ID_EDIT_FINDPREVIOUS,
        ID_EDIT_MATCHES,
        ID_EDIT_GOTO,
        ID_TOOLS_RECOMPRESS,
        ID_TOOLS_RESORT,
        ID_TOOLS_REMOVETRASH,
        0};
    for(int i=0; DisableItems[i]; i++)
        SetMenuItemInfo(hMenu, DisableItems[i], FALSE, &mii);
    
    DestroyWindow(hList);
    
    return true;
}

bool Open(){
    if(!Archive::Close()) return false;
    
    GetFileTitle(Archive::Path, Archive::Filename, 1024);
    
    Archive::IsOpen = true;
    Archive::IsModified = false;
    
    SetWorkspace();
}

bool PopulateEntries(){
    unsigned EntryCount = 1;
    LVITEM item;
    memset(&item, 0x00, sizeof(LVITEM));
    item.mask = LVIF_TEXT;
    item.iItem = 0;
    item.pszText = (LPTSTR) L"Test";
    SendMessage(hList, LVM_SETITEMCOUNT, EntryCount, LVSICF_NOSCROLL);
    SendMessage(hList, LVM_INSERTITEM, 0, (LPARAM) &item);
    
    wchar_t buffer[17];
    wsprintf(buffer, L"%u file%s", EntryCount, (EntryCount == 1) ? L"" : L"s");
    SendMessage(statusbar, SB_SETTEXT, 0, (LPARAM) buffer);
    
    MENUITEMINFO mii;
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_STATE;
    mii.fState = (EntryCount > 0) ? MFS_ENABLED : MFS_DISABLED;
    SetMenuItemInfo(hMenu, ID_FILE_EXPORTALL, FALSE, &mii);
    SetMenuItemInfo(hMenu, ID_EDIT_FIND, FALSE, &mii);
    SetMenuItemInfo(hMenu, ID_EDIT_FINDNEXT, FALSE, &mii);
    SetMenuItemInfo(hMenu, ID_EDIT_FINDPREVIOUS, FALSE, &mii);
    SetMenuItemInfo(hMenu, ID_EDIT_MATCHES, FALSE, &mii);
    SetMenuItemInfo(hMenu, ID_EDIT_GOTO, FALSE, &mii);
    SetMenuItemInfo(hMenu, ID_TOOLS_RECOMPRESS, FALSE, &mii);
    SetMenuItemInfo(hMenu, ID_TOOLS_RESORT, FALSE, &mii);
    SetMenuItemInfo(hMenu, ID_TOOLS_REMOVETRASH, FALSE, &mii);
    
    return true;
}

bool Save(){
    if(!Archive::IsModified) return true;
    if(Path[0] == '\0'){ //No path because we're dealing with a newly created archive
        return SaveAs();
    }
    
    Archive::IsModified = false;
    wchar_t WindowTitle[1024+10];
    wsprintf(WindowTitle, L"%s%s - FARDive", Archive::Filename, L"");
    SetWindowText(hWnd, WindowTitle);
    return true;
}

bool SaveAs(){
    wchar_t OldPath[1024], OldFilename[1024];
    //Backup old settings
    wcscpy(OldPath, Archive::Path);
    wcscpy(OldFilename, Archive::Filename);
    
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = FILTER_ARCHIVES_ONLY;
    ofn.lpstrCustomFilter = ArchiveSaveFilter;
    ofn.lpstrFile = Path;
    ofn.lpstrFileTitle = Filename;
    ofn.Flags = OFN_DONTADDTORECENT | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    if(GetSaveFileName(&ofn) && Save())
        return true;
    
    //Restore old settings
    wcscpy(Archive::Path, OldPath);
    wcscpy(Archive::Filename, OldFilename);
    return false;
}

bool SetWorkspace(){
    wchar_t WindowTitle[1024+11];
    wsprintf(WindowTitle, L"%s%s - FARDive", Archive::Filename, Archive::IsModified ? L"*" : L"");
    SetWindowText(hWnd, WindowTitle);
    
    MENUITEMINFO mii;
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_STATE;
    mii.fState = MFS_ENABLED;
    
    SetMenuItemInfo(hMenu, ID_FILE_SAVE, FALSE, &mii);
    SetMenuItemInfo(hMenu, ID_FILE_SAVEAS, FALSE, &mii);
    SetMenuItemInfo(hMenu, ID_FILE_ADD, FALSE, &mii);
    SetMenuItemInfo(hMenu, ID_FILE_CHANGETYPE, FALSE, &mii);
    SetMenuItemInfo(hMenu, ID_FILE_CLOSE, FALSE, &mii);
    
    RECT ClientRect;
    GetClientRect(hWnd, &ClientRect);
    hList = CreateWindowEx(WS_EX_CLIENTEDGE | LVS_EX_DOUBLEBUFFER | WS_EX_COMPOSITED, WC_LISTVIEW, NULL, LVS_LIST | LVS_SHOWSELALWAYS | WS_CHILD | WS_VISIBLE,
        5, 5, 192, ClientRect.bottom-statusbarheight-10, hWnd, NULL, NULL, NULL);
    SetWindowTheme(hList, L"Explorer", NULL);
    
    PopulateEntries();
    
    return true;
}

}