#include "Utils.h"

int CenterDialog(HWND hDlg) {
  RECT rt, rtParent;
  int DlgWidth, DlgHeight;
  int NewPosX, NewPosY;
  if (GetWindowRect(hDlg, &rtParent)) {
      GetClientRect(GetParent(hDlg), &rt);
      DlgWidth    = rtParent.right - rtParent.left;
      DlgHeight   = rtParent.bottom - rtParent.top ;
      NewPosX     = (rt.right - rt.left - DlgWidth)/2;
      NewPosY     = (rt.bottom - rt.top - DlgHeight)/2;
      
      // if the About box is larger than the physical screen 
      if (NewPosX < 0) NewPosX = 0;
      if (NewPosY < 0) NewPosY = 0;
      SetWindowPos(hDlg, 0, NewPosX, NewPosY,
          0, 0, SWP_NOZORDER | SWP_NOSIZE);
  }
  return 0;
}

HICON ExtractIconFromExe(LPCTSTR szFilePath) {
  HICON hIcon = NULL;
  
  // Extract the first icon from the executable
  ExtractIconEx(szFilePath, 0, &hIcon, NULL, 1);
  
  return hIcon;
}