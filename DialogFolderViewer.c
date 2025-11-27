#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include "resource.h"
#include "Utils.h"
#include "Dialogs.h"
#include "Lycheepad.h"

static HINSTANCE hLocalInst;
static HWND hListFiles;
static HWND hEditPath;
static TCHAR szCurrentPath[MAX_PATH];
static TCHAR szSelectedFile[MAX_PATH];

// Forward declarations
static void RefreshFileList(void);
static BOOL IsExecutableFile(LPCTSTR szFileName);

static LRESULT CALLBACK FolderViewerProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_INITDIALOG:
            CenterDialog(hDlg);
            hListFiles = GetDlgItem(hDlg, IDC_LIST_FILES);
            hEditPath = GetDlgItem(hDlg, IDC_EDIT_PATH);
            
            // Initialize with root directory
            _tcscpy(szCurrentPath, _T(""));
            SetWindowText(hEditPath, szCurrentPath);
            RefreshFileList();
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK:
                    if (_tcslen(szSelectedFile) > 0) {
                        EndDialog(hDlg, LOWORD(wParam));
                    } else {
                        MessageBox(hDlg, _T("Please select an executable file."), _T("Error"), MB_OK | MB_ICONERROR);
                    }
                    break;
                    
                case IDCANCEL:
                    EndDialog(hDlg, LOWORD(wParam));
                    break;
                    
                case IDC_BTN_UP:
                    {
                        TCHAR szParentPath[MAX_PATH];
                        LPTSTR pLastSlash;
                        
                        _tcscpy(szParentPath, szCurrentPath);
                        pLastSlash = _tcsrchr(szParentPath, _T('\\'));
                        *pLastSlash = _T('\0');
                        if (_tcslen(szParentPath) > 0) {
                            _tcscpy(szCurrentPath, szParentPath);
                        } else {
                            _tcscpy(szCurrentPath, _T(""));
                        }
                        SetWindowText(hEditPath, szCurrentPath);
                        RefreshFileList();
                    }
                    break;
                    
                case IDC_LIST_FILES:
                    if (HIWORD(wParam) == LBN_DBLCLK) {
                        int selected = SendMessage(hListFiles, LB_GETCURSEL, 0, 0);
                        if (selected != LB_ERR) {
                            TCHAR szFileName[MAX_PATH];
                            TCHAR szFullPath[MAX_PATH];
                            DWORD attr;

                            SendMessage(hListFiles, LB_GETTEXT, selected, (LPARAM)szFileName);
                        
                            // Check if it's a directory or file
                            _tcscpy(szFullPath, szCurrentPath);
                            
                            // Add trailing backslash if needed
                            if (szCurrentPath[_tcslen(szCurrentPath)-1] != _T('\\')) {
                                _tcscat(szFullPath, _T("\\"));
                            }
                            _tcscat(szFullPath, szFileName);
                            
                            attr = GetFileAttributes(szFullPath);
                            if (attr & FILE_ATTRIBUTE_DIRECTORY) {
                                // It's a directory, navigate into it
                                _tcscpy(szCurrentPath, szFullPath);
                                SetWindowText(hEditPath, szCurrentPath);
                                RefreshFileList();
                            } else if (IsExecutableFile(szFileName)) {
                                // It's an executable file, select it
                                _tcscpy(szSelectedFile, szFullPath);
                                EndDialog(hDlg, IDOK);
                            }
                        }
                    } else if (HIWORD(wParam) == LBN_SELCHANGE) {
                        int selected = SendMessage(hListFiles, LB_GETCURSEL, 0, 0);
                        if (selected != LB_ERR) {
                            TCHAR szFileName[MAX_PATH];
                            TCHAR szFullPath[MAX_PATH];
                            DWORD attr;

                            SendMessage(hListFiles, LB_GETTEXT, selected, (LPARAM)szFileName);
                            _tcscpy(szFullPath, szCurrentPath);
                            
                            // Add trailing backslash if needed
                            if (szCurrentPath[_tcslen(szCurrentPath)-1] != _T('\\')) {
                                _tcscat(szFullPath, _T("\\"));
                            }
                            _tcscat(szFullPath, szFileName);
                            
                            attr = GetFileAttributes(szFullPath);
                            if (!(attr & FILE_ATTRIBUTE_DIRECTORY)) {
                                if (IsExecutableFile(szFileName)) {
                                    _tcscpy(szSelectedFile, szFullPath);
                                }
                            } else {
                                szSelectedFile[0] = _T('\0');
                            }
                        }
                    }
                    break;
            }
            break;
    }
    return FALSE;
}

static void RefreshFileList(void) {
    WIN32_FIND_DATA ffd;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    TCHAR szSearchPath[MAX_PATH];
    
    SendMessage(hListFiles, LB_RESETCONTENT, 0, 0);
    
    // List files and directories
    _tcscpy(szSearchPath, szCurrentPath);
    _tcscat(szSearchPath, _T("\\*"));
    
    hFind = FindFirstFile(szSearchPath, &ffd);
    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }
    
    do {
        if (_tcscmp(ffd.cFileName, _T(".")) == 0 || _tcscmp(ffd.cFileName, _T("..")) == 0) {
            continue;
        }
        
        SendMessage(hListFiles, LB_ADDSTRING, 0, (LPARAM)ffd.cFileName);
    } while (FindNextFile(hFind, &ffd) != 0);
    
    FindClose(hFind);
}

static BOOL IsExecutableFile(LPCTSTR szFileName) {
    LPTSTR pExt = _tcsrchr(szFileName, _T('.'));
    if (pExt) {
        if (_tcsicmp(pExt, _T(".exe")) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

int ShowDialogFolderViewer(HINSTANCE hInst, HWND hWnd, LPTSTR szSelectedPath) {
    hLocalInst = hInst;
    szSelectedFile[0] = _T('\0');
    
    if (DialogBox(hInst, (LPCTSTR)IDD_FOLDER_VIEWER, hWnd, (DLGPROC)FolderViewerProc) == IDOK) {
        _tcscpy(szSelectedPath, szSelectedFile);
        return 1;
    }
    return 0;
}
