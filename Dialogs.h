#ifndef _DIALOGS_H_
#define _DIALOGS_H_

#include <windows.h>
#include "Lycheepad.h"

int ShowDialogAbout         (HINSTANCE hInst, HWND hWnd);
int ShowDialogAppList       (HINSTANCE hInst, HWND hWnd);
int ShowDialogSelectExe     (HINSTANCE hInst, HWND hWnd, APPSLOT* pAppSlot);
int ShowDialogFolderViewer  (HINSTANCE hInst, HWND hWnd, LPTSTR szSelectedPath);

#endif