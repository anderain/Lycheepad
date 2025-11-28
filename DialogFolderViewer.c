#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include "resource.h"
#include "Utils.h"
#include "Dialogs.h"
#include "Lycheepad.h"

static HINSTANCE hLocalInst;
static HWND hListFiles;
static HWND hListFolders;
static HWND hEditPath;
static TCHAR szCurrentPath[MAX_PATH] = _T("");
static TCHAR szSelectedFile[MAX_PATH];

// Forward declarations
static void RefreshFileAndFolderList(void);
static BOOL IsExecutableFile(LPCTSTR szFileName);
static BOOL ParseLnkFile(LPCTSTR szLnkPath, LPTSTR szExePath);

static LRESULT CALLBACK FolderViewerProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_INITDIALOG:
            CenterDialog(hDlg);
            hListFiles = GetDlgItem(hDlg, IDC_LIST_FILES);
            hListFolders = GetDlgItem(hDlg, IDC_LIST_FOLDERS);
            hEditPath = GetDlgItem(hDlg, IDC_EDIT_PATH);
            
            SetWindowText(hEditPath, szCurrentPath);
            RefreshFileAndFolderList();
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK:
                    if (_tcslen(szSelectedFile) > 0) {
                        // Check if the selected file is an executable or LNK file
                        LPTSTR pExt = _tcsrchr(szSelectedFile, _T('.'));
                        if (pExt != NULL) {
                            if (_tcsicmp(pExt, _T(".exe")) == 0) {
                                // It's an EXE file, directly use it
                                EndDialog(hDlg, LOWORD(wParam));
                            } else if (_tcsicmp(pExt, _T(".lnk")) == 0) {
                                // It's a LNK file, parse it
                                TCHAR szExePath[MAX_PATH];
                                if (ParseLnkFile(szSelectedFile, szExePath)) {
                                    _tcscpy(szSelectedFile, szExePath);
                                    EndDialog(hDlg, LOWORD(wParam));
                                } else {
                                    MessageBox(hDlg, _T("Failed to parse the LNK file."), _T("Error"), MB_OK | MB_ICONERROR);
                                }
                            } else {
                                MessageBox(hDlg, _T("Please select an executable file (.exe) or shortcut (.lnk)."), _T("Error"), MB_OK | MB_ICONERROR);
                            }
                        } else {
                            MessageBox(hDlg, _T("Please select an executable file (.exe) or shortcut (.lnk)."), _T("Error"), MB_OK | MB_ICONERROR);
                        }
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
                        RefreshFileAndFolderList();
                    }
                    break;
                    
                case IDC_LIST_FILES:
                    if (HIWORD(wParam) == LBN_SELCHANGE) {
                        int selected = SendMessage(hListFiles, LB_GETCURSEL, 0, 0);
                        if (selected != LB_ERR) {
                            TCHAR szFileName[MAX_PATH];
                            TCHAR szFullPath[MAX_PATH];

                            SendMessage(hListFiles, LB_GETTEXT, selected, (LPARAM)szFileName);
                            
                            // Build full path and select file
                            _tcscpy(szFullPath, szCurrentPath);
                            
                            // Add trailing backslash if needed
                            if (szCurrentPath[_tcslen(szCurrentPath)-1] != _T('\\')) {
                                _tcscat(szFullPath, _T("\\"));
                            }
                            _tcscat(szFullPath, szFileName);
                            _tcscpy(szSelectedFile, szFullPath);
                        }
                    }
                    break;
                    
                case IDC_LIST_FOLDERS:
                    if (HIWORD(wParam) == LBN_DBLCLK) {
                        int selected = SendMessage(hListFolders, LB_GETCURSEL, 0, 0);
                        if (selected != LB_ERR) {
                            TCHAR szFolderName[MAX_PATH];
                            TCHAR szFullPath[MAX_PATH];

                            SendMessage(hListFolders, LB_GETTEXT, selected, (LPARAM)szFolderName);
                        
                            // Navigate into the selected directory
                            _tcscpy(szFullPath, szCurrentPath);
                            
                            // Add trailing backslash if needed
                            if (szCurrentPath[_tcslen(szCurrentPath)-1] != _T('\\')) {
                                _tcscat(szFullPath, _T("\\"));
                            }
                            _tcscat(szFullPath, szFolderName);
                            
                            // Update current path and refresh lists
                            _tcscpy(szCurrentPath, szFullPath);
                            SetWindowText(hEditPath, szCurrentPath);
                            RefreshFileAndFolderList();
                        }
                    }
                    break;
            }
            break;
    }
    return FALSE;
}

static void RefreshFileAndFolderList(void) {
    WIN32_FIND_DATA ffd;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    TCHAR szSearchPath[MAX_PATH];
    
    // Reset both lists
    SendMessage(hListFiles, LB_RESETCONTENT, 0, 0);
    SendMessage(hListFolders, LB_RESETCONTENT, 0, 0);
    
    // Build search path
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
        
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // It's a directory, add to folder list
            SendMessage(hListFolders, LB_ADDSTRING, 0, (LPARAM)ffd.cFileName);
        }
    } while (FindNextFile(hFind, &ffd) != 0);
    
    FindClose(hFind);
    
    // Now list only executable and link files
    hFind = FindFirstFile(szSearchPath, &ffd);
    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }
    
    do {
        if (_tcscmp(ffd.cFileName, _T(".")) == 0 || _tcscmp(ffd.cFileName, _T("..")) == 0) {
            continue;
        }
        
        if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            // It's a file, check if it's executable or link
            if (IsExecutableFile(ffd.cFileName)) {
                SendMessage(hListFiles, LB_ADDSTRING, 0, (LPARAM)ffd.cFileName);
            }
        }
    } while (FindNextFile(hFind, &ffd) != 0);
    
    FindClose(hFind);
}

static BOOL IsExecutableFile(LPCTSTR szFileName) {
    LPTSTR pExt = _tcsrchr(szFileName, _T('.'));
    if (pExt) {
        if (_tcsicmp(pExt, _T(".exe")) == 0 || _tcsicmp(pExt, _T(".lnk")) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

// Function to parse Windows CE LNK file and extract executable path
static BOOL ParseLnkFile(LPCTSTR szLnkPath, LPTSTR szExePath) {
    HANDLE hFile;
    DWORD dwBytesRead;
    unsigned char byteBuffer[MAX_PATH * 2];
    TCHAR szBuffer[MAX_PATH * 2];
    TCHAR* pPoundSign;
    TCHAR* pCommandLine;
    int nArgLength;
    TCHAR* pSpace;
    unsigned int i;
    
    hFile = CreateFile(szLnkPath, GENERIC_READ, FILE_SHARE_READ, NULL, 
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
    
    // Read the LNK file content in ASCII format
    if (!ReadFile(hFile, byteBuffer, sizeof(byteBuffer) - sizeof(byteBuffer[0]), &dwBytesRead, NULL)) {
        CloseHandle(hFile);
        return FALSE;
    }
    
    CloseHandle(hFile);

    // convert ASCII bytes to wide char
    byteBuffer[dwBytesRead] = '\0';
    for (i = 0; i < dwBytesRead; ++i) {
        szBuffer[i] = byteBuffer[i];
    }
    szBuffer[i] = _T('\0');
    
    // Find the pound sign
    pPoundSign = _tcschr(szBuffer, _T('#'));
    if (pPoundSign == NULL) {
        return FALSE;
    }
    
    // Calculate argument length
    nArgLength = _ttoi(szBuffer);
    if (nArgLength <= 0) {
        return FALSE;
    }
    
    // Get the command line (skip the pound sign)
    pCommandLine = pPoundSign + 1;
    
    // Extract the executable path (everything before the first space)
    pSpace = _tcschr(pCommandLine, _T(' '));
    if (pSpace != NULL) {
        // Copy only the executable path
        int nPathLength = pSpace - pCommandLine;
        _tcsncpy(szExePath, pCommandLine, nPathLength);
        szExePath[nPathLength] = _T('\0');
    } else {
        // No parameters, copy the whole command line
        _tcscpy(szExePath, pCommandLine);
    }
    
    return TRUE;
}

int ShowDialogFolderViewer(HINSTANCE hInst, HWND hWnd, LPTSTR szSelectedPath) {
    hLocalInst = hInst;
     
    if (DialogBox(hInst, (LPCTSTR)IDD_FOLDER_VIEWER, hWnd, (DLGPROC)FolderViewerProc) == IDOK) {
        _tcscpy(szSelectedPath, szSelectedFile);
        return 1;
    }
    return 0;
}
