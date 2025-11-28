#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "Utils.h"
#include "Dialogs.h"
#include "Lycheepad.h"

static HINSTANCE hLocalInst;
static HWND hListBox;
static APPSLOT* pAppSlots;

// Forward declaration
static void RefreshAppList(void);

static LRESULT CALLBACK AppListProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_INITDIALOG:
            CenterDialog(hDlg);
            hListBox = GetDlgItem(hDlg, IDC_LIST_APP);
            pAppSlots = AppSlots;
            
            // Populate list box with existing apps
            RefreshAppList();
            return TRUE;
            
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK:
                    // TODO: Save the configuration
                    EndDialog(hDlg, LOWORD(wParam));
                    break;
                    
                case IDCANCEL:
                    EndDialog(hDlg, LOWORD(wParam));
                    break;
                    
                case IDC_BTN_ADD:
                    if (ShowDialogSelectExe(hLocalInst, hDlg, NULL) == IDOK) {
                        RefreshAppList();
                    }
                    break;
                    
                case IDC_BTN_REMOVE:
                    {
                        int selected = SendMessage(hListBox, LB_GETCURSEL, 0, 0);
                        int i;
                        if (selected == LB_ERR) {
                            break;
                        }
                        // Delete the selected one, and move the subsequent ones forward one by one.
                        i = selected;
                        if (pAppSlots[i].hIcon) {
                            DestroyIcon(pAppSlots[i].hIcon);
                            pAppSlots[i].hIcon = NULL;
                        }
                        if (pAppSlots[i].szPath) {
                            LocalFree(pAppSlots[i].szPath);
                            pAppSlots[i].szPath = NULL;
                        }
                        for (i = selected; i < NUM_BTN_SLOTS - 1; ++i) {
                            memcpy(pAppSlots + i, pAppSlots + i + 1, sizeof(APPSLOT));
                        }
                        SendMessage(hListBox, LB_DELETESTRING, selected, 0);
                    }
                    break;
                    
                case IDC_BTN_MOVE_UP:
                    {
                        int selected = SendMessage(hListBox, LB_GETCURSEL, 0, 0);
                        if (selected != LB_ERR && selected > 0) {
                            // Find the corresponding slots
                            TCHAR label1[40], label2[40];
                            int i;
                            APPSLOT* slot1 = NULL;
                            APPSLOT* slot2 = NULL;

                            SendMessage(hListBox, LB_GETTEXT, selected, (LPARAM)label1);
                            SendMessage(hListBox, LB_GETTEXT, selected - 1, (LPARAM)label2);
                            
                            // Find slots in AppSlots array
                            
                            for (i = 0; i < NUM_BTN_SLOTS; i++) {
                                if (_tcscmp(pAppSlots[i].szLabel, label1) == 0) {
                                    slot1 = &pAppSlots[i];
                                }
                                if (_tcscmp(pAppSlots[i].szLabel, label2) == 0) {
                                    slot2 = &pAppSlots[i];
                                }
                                if (slot1 && slot2) break;
                            }
                            
                            if (slot1 && slot2) {
                                // Swap the two slots
                                APPSLOT temp = *slot1;
                                *slot1 = *slot2;
                                *slot2 = temp;
                                
                                // Update list box
                                SendMessage(hListBox, LB_DELETESTRING, selected, 0);
                                SendMessage(hListBox, LB_DELETESTRING, selected - 1, 0);
                                SendMessage(hListBox, LB_INSERTSTRING, selected - 1, (LPARAM)label1);
                                SendMessage(hListBox, LB_INSERTSTRING, selected, (LPARAM)label2);
                                SendMessage(hListBox, LB_SETCURSEL, selected - 1, 0);
                            }
                        }
                    }
                    break;
                    
                case IDC_BTN_MOVE_DOWN:
                    {
                        int selected = SendMessage(hListBox, LB_GETCURSEL, 0, 0);
                        int count = SendMessage(hListBox, LB_GETCOUNT, 0, 0);
                        if (selected != LB_ERR && selected < count - 1) {
                            // Find the corresponding slots
                            TCHAR label1[40], label2[40];
                            int i;
                            APPSLOT* slot1 = NULL;
                            APPSLOT* slot2 = NULL;

                            SendMessage(hListBox, LB_GETTEXT, selected, (LPARAM)label1);
                            SendMessage(hListBox, LB_GETTEXT, selected + 1, (LPARAM)label2);
                            
                            // Find slots in AppSlots array
                            
                            for (i = 0; i < NUM_BTN_SLOTS; i++) {
                                if (_tcscmp(pAppSlots[i].szLabel, label1) == 0) {
                                    slot1 = &pAppSlots[i];
                                }
                                if (_tcscmp(pAppSlots[i].szLabel, label2) == 0) {
                                    slot2 = &pAppSlots[i];
                                }
                                if (slot1 && slot2) break;
                            }
                            
                            if (slot1 && slot2) {
                                // Swap the two slots
                                APPSLOT temp = *slot1;
                                *slot1 = *slot2;
                                *slot2 = temp;
                                
                                // Update list box
                                SendMessage(hListBox, LB_DELETESTRING, selected, 0);
                                SendMessage(hListBox, LB_DELETESTRING, selected, 0);
                                SendMessage(hListBox, LB_INSERTSTRING, selected, (LPARAM)label2);
                                SendMessage(hListBox, LB_INSERTSTRING, selected + 1, (LPARAM)label1);
                                SendMessage(hListBox, LB_SETCURSEL, selected + 1, 0);
                            }
                        }
                    }
                    break;
                    
                case IDC_LIST_APP:
                    if (HIWORD(wParam) == LBN_DBLCLK) {
                        int selected = SendMessage(hListBox, LB_GETCURSEL, 0, 0);
                        if (selected != LB_ERR) {
                            int i;
                            // Edit the selected app
                            TCHAR szLabel[40];
                            SendMessage(hListBox, LB_GETTEXT, selected, (LPARAM)szLabel);
                            
                            // Find the corresponding slot
                            for (i = 0; i < NUM_BTN_SLOTS; i++) {
                                if (_tcscmp(pAppSlots[i].szLabel, szLabel) == 0) {
                                    // Show the select exe dialog with existing data
                                    if (ShowDialogSelectExe(hLocalInst, hDlg, &pAppSlots[i]) == IDOK) {
                                        RefreshAppList();
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    break;
            }
            break;
    }
    return FALSE;
}

static void RefreshAppList(void) {
    int i;
    SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
    
    // Populate list box with existing apps
    for (i = 0; i < NUM_BTN_SLOTS; i++) {
        if (pAppSlots[i].szPath != NULL && pAppSlots[i].szPath[0] != _T('\0')) {
            SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)pAppSlots[i].szLabel);
        }
    }
}

int ShowDialogAppList(HINSTANCE hInst, HWND hWnd) {
    hLocalInst = hInst;
    return DialogBox(hInst, (LPCTSTR)IDD_APP_LIST, hWnd, (DLGPROC)AppListProc);
}