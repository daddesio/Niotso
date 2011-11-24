#include "Windows.hpp"
#include "GUI.hpp"

void CenterDialog(HWND hDlg){
    RECT parent, child;
    GetWindowRect(hWnd, &parent);
    GetWindowRect(hDlg, &child);
    int x = (parent.right + parent.left - child.right + child.left)/2,
        y = (parent.bottom + parent.top - child.bottom + child.top)/2;
    SetWindowPos(hDlg, 0, x, y, 0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER);
}