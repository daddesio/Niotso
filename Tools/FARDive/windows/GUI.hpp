extern HINSTANCE hInst;

extern HMENU hMenu, listmenu;
extern HDC hDC;
extern OPENFILENAME ofn;
extern unsigned int statusbarheight;

extern HBITMAP PNGIcon[MENUICONS];
enum {
    BMP_FILE_NEW,
    BMP_FILE_OPEN,
    BMP_FILE_SAVE,
    BMP_FILE_SAVEAS,
    BMP_FILE_ADD,
    BMP_FILE_EXPORTALL,
    BMP_FILE_EXPORTSELECTED,
    BMP_FILE_CHANGETYPE,
    BMP_FILE_CLOSE,
    BMP_FILE_EXIT,
    BMP_EDIT_DUPLICATE,
    BMP_EDIT_REMOVE,
    BMP_EDIT_RENAME,
    BMP_EDIT_PROPERTIES,
    BMP_EDIT_CONTENTS,
    BMP_EDIT_FIND,
    BMP_EDIT_FINDNEXT,
    BMP_EDIT_FINDPREVIOUS,
    BMP_EDIT_MATCHES,
    BMP_EDIT_GOTO,
    BMP_EDIT_PREFERENCES,
    BMP_TOOLS_RECOMPRESS,
    BMP_TOOLS_RESORT,
    BMP_TOOLS_REMOVEHOLES,
    BMP_TOOLS_BATCH,
    BMP_HELP_HOWTOUSE
};

static const wchar_t FILTER_ARCHIVES_FILES[] = 
    L"All supported archives\0*.far;*.dbpf;*.dat;*.package\0"
    L"FAR (*.far, *.dat, *.package)\0*.far;*.dat;*.package\0"
    L"DBPF (*.dbpf, *.dat, *.package)\0*.dbpf;*.dat;*.package\0"
    L"All Files\0*.*\0\0";

static const wchar_t FILTER_ARCHIVES_ONLY[] = 
    L"All supported archives\0*.far;*.dbpf;*.dat;*.package\0"
    L"FAR (*.far, *.dat, *.package)\0*.far;*.dat;*.package\0"
    L"DBPF (*.dbpf, *.dat, *.package)\0*.dbpf;*.dat;*.package\0"
    L"All Files\0*.*\0\0";

static const wchar_t FILTER_FILES[] = 
    L"All Files\0*.*\0\0";

extern wchar_t ArchiveOpenFilter[128], ArchiveAddFilter[128], ArchiveSaveFilter[128];

//Controls
extern HWND hWnd, statusbar, hList;

void CenterDialog(HWND hDlg);
HBITMAP ReadPNGIcon(int ID);