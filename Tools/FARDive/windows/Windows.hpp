#define UNICODE
#define WINVER 0x0600
#include <windows.h>
#include <commctrl.h>
#include "../FARDive.hpp"
#include "resource.hpp"

#define DefineDialog(x) namespace x {INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);}

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
DefineDialog(AddToArchive);
DefineDialog(NewArchive);