#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdlib.h>
#include "resource.h"
#include "Utils.h"
#include "Dialogs.h"
#include "Lycheepad.h"

static HINSTANCE hLocalInst;
static HWND hEditExe;
static HWND hEditLabel;

static BOOL SelectExecutableFile(HWND hWnd, LPTSTR szFilePath) {
    TCHAR szSelectedPath[MAX_PATH] = {0};
    
    ShowDialogFolderViewer(hLocalInst, hWnd, szSelectedPath);
    
    if (_tcslen(szSelectedPath) > 0) {
        _tcscpy(szFilePath, szSelectedPath);
        return TRUE;
    }
    return FALSE;
}

static BOOL AddAppToSlot(LPCTSTR szPath, LPCTSTR szLabel) {
    int i;
    // Find an empty slot
    for (i = 0; i < NUM_BTN_SLOTS; i++) {
        if (AppSlots[i].szPath == NULL || AppSlots[i].szPath[0] == _T('\0')) {
        // Allocate memory for path
        AppSlots[i].szPath = (TCHAR*)LocalAlloc(LPTR, (_tcslen(szPath) + 1) * sizeof(TCHAR));
        if (AppSlots[i].szPath == NULL) {
            return FALSE;
        }
        
        _tcscpy(AppSlots[i].szPath, szPath);
        _tcscpy(AppSlots[i].szLabel, szLabel);
        
        // Extract icon
        AppSlots[i].hIcon = ExtractIconFromExe(szPath);
        
        return TRUE;
    }
    }
    
    return FALSE; // No empty slots
}

static LRESULT CALLBACK SelectExeProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static APPSLOT* pEditingSlot = NULL;
    
    switch (message) {
        case WM_INITDIALOG:
            CenterDialog(hDlg);
            hEditExe = GetDlgItem(hDlg, IDC_EDIT_EXE);
            hEditLabel = GetDlgItem(hDlg, IDC_EDIT_LABEL);
            
            // Check if we're in edit mode (lParam contains pointer to APPSLOT)
            if (lParam != NULL) {
                pEditingSlot = (APPSLOT*)lParam;
                SetWindowLong(hDlg, GWL_USERDATA, (LONG)lParam);
            
                if (pEditingSlot != NULL) {
                    // Set existing data for editing
                    SetWindowText(hEditExe, pEditingSlot->szPath);
                    SetWindowText(hEditLabel, pEditingSlot->szLabel);
                }
            }
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK:
                    {
                        TCHAR szPath[MAX_PATH];
                        TCHAR szLabel[40];
                        
                        GetWindowText(hEditExe, szPath, MAX_PATH);
                        GetWindowText(hEditLabel, szLabel, 40);
                        
                        if (_tcslen(szPath) == 0) {
                            MessageBox(hDlg, _T("Please select an executable file."), _T("Error"), MB_OK | MB_ICONERROR);
                        } else if (_tcslen(szLabel) == 0) {
                            MessageBox(hDlg, _T("Please enter a label for the application."), _T("Error"), MB_OK | MB_ICONERROR);
                        } else {
                            pEditingSlot = (APPSLOT*)GetWindowLong(hDlg, GWL_USERDATA);
                            
                            if (pEditingSlot != NULL) {
                                // Edit mode: update existing slot
                                if (pEditingSlot->szPath != NULL) {
                                    LocalFree(pEditingSlot->szPath);
                                }
                                
                                // Allocate new memory for path
                                pEditingSlot->szPath = (TCHAR*)LocalAlloc(LPTR, (_tcslen(szPath) + 1) * sizeof(TCHAR));
                                if (pEditingSlot->szPath == NULL) {
                                    MessageBox(hDlg, _T("Memory allocation failed."), _T("Error"), MB_OK | MB_ICONERROR);
                                    break;
                                }
                                
                                _tcscpy(pEditingSlot->szPath, szPath);
                                _tcscpy(pEditingSlot->szLabel, szLabel);
                                
                                // Update icon
                                if (pEditingSlot->hIcon) {
                                    DestroyIcon(pEditingSlot->hIcon);
                                }
                                pEditingSlot->hIcon = ExtractIconFromExe(szPath);
                                
                                EndDialog(hDlg, LOWORD(wParam));
                            } else {
                                // Add mode: add new slot
                                if (AddAppToSlot(szPath, szLabel)) {
                                    EndDialog(hDlg, LOWORD(wParam));
                                } else {
                                    MessageBox(hDlg, _T("No empty slots available."), _T("Error"), MB_OK | MB_ICONERROR);
                                }
                            }
                        }
                    }
                    break;
                    
                case IDCANCEL:
                    EndDialog(hDlg, LOWORD(wParam));
                    break;
                    
                case IDC_BTN_SELECT:
                    {
                        TCHAR szFilePath[MAX_PATH] = {0};
                        TCHAR szFileName[MAX_PATH];
                        TCHAR szLabelText[40];
                        LPTSTR pFileName;
                        LPTSTR pExt;
                        if (SelectExecutableFile(hDlg, szFilePath)) {
                            SetWindowText(hEditExe, szFilePath);
                            // Auto-fill label with filename without extension
                            _tcscpy(szFileName, szFilePath);
                            pFileName = _tcsrchr(szFileName, _T('\\'));
                            if (pFileName) {
                                pFileName++; // Skip the backslash
                            } else {
                                pFileName = szFileName;
                            }
                            
                            // Remove extension
                            pExt = _tcsrchr(pFileName, _T('.'));
                            if (pExt) {
                                *pExt = _T('\0');
                            }
                            
                            _tcscpy(szLabelText, pFileName);
                            SetWindowText(hEditLabel, szLabelText);
                        }
                    }
                    break;
            }
            break;
    }
    return FALSE;
}

int ShowDialogSelectExe(HINSTANCE hInst, HWND hWnd, APPSLOT* pAppSlot) {
    hLocalInst = hInst;
    return DialogBoxParam(hInst, (LPCTSTR)IDD_SELECT_EXE, hWnd, (DLGPROC)SelectExeProc, (LPARAM)pAppSlot);
}
