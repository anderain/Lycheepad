#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "Lycheepad.h"
#include "Dialogs.h"
#include "Utils.h"

#define MAX_LOADSTRING      100
#define CONFIG_FILE_PATH    _T("lycheepad.cfg")

// Global Variables:
HINSTANCE   hInst;
HWND        hwndCB;
APPSLOT     AppSlots[NUM_BTN_SLOTS];

ATOM                MyRegisterClass     (HINSTANCE hInstance, LPTSTR szWindowClass);
BOOL                InitInstance        (HINSTANCE, int);
LRESULT CALLBACK    WndProc             (HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    About               (HWND, UINT, WPARAM, LPARAM);
void                ReloadSlotIcons     ();
void                DestroySlots        ();
BOOL                LoadConfiguration   (LPCTSTR szFileName);
BOOL                SaveConfiguration   (LPCTSTR szFileName);

int WINAPI WinMain( HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPTSTR    lpCmdLine,
                    int       nCmdShow) {
    MSG msg;
    HACCEL hAccelTable;
    HANDLE hMutex;

    // Prevent multiple instances
    hMutex = CreateMutex(NULL, FALSE, _T("MySingleInstanceMutex"));
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        TCHAR szWindowClass[MAX_LOADSTRING];
        HWND hExistingWnd;
        LoadString(hInstance, IDC_LYCHEEPAD, szWindowClass, MAX_LOADSTRING);
        hExistingWnd = FindWindow(szWindowClass, NULL);
        if (hExistingWnd) {
            SetForegroundWindow(hExistingWnd);
            ShowWindow(hExistingWnd, SW_SHOWNORMAL);
        }
        return 0;
    }

    // Load AppSlots from file
    memset(AppSlots, 0, sizeof(AppSlots));
    LoadConfiguration(CONFIG_FILE_PATH);
    ReloadSlotIcons();

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow)) {
        CloseHandle(hMutex);
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_LYCHEEPAD);

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    CloseHandle(hMutex);
    return msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass) {
    WNDCLASS    wc;

    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = (WNDPROC) WndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInstance;
    wc.hIcon            = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LYCHEEPAD));
    wc.hCursor          = 0;
    wc.hbrBackground    = (HBRUSH) GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName     = 0;
    wc.lpszClassName    = szWindowClass;

    return RegisterClass(&wc);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    HWND    hWnd;
    TCHAR    szTitle[MAX_LOADSTRING];       // The title bar text
    TCHAR    szWindowClass[MAX_LOADSTRING]; // The window class name

    // Store instance handle in our global variable
    hInst = hInstance;

    // Initialize global strings
    LoadString(hInstance, IDC_LYCHEEPAD, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance, szWindowClass);

    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
        0, 0, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    if (!hWnd) {    
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    if (hwndCB) {
        CommandBar_Show(hwndCB, TRUE);   
    }

    return TRUE;
    
}

static void DrawIconButton(LPDRAWITEMSTRUCT dis, int buttonIndex)
{
    APPSLOT* pSlot = AppSlots + buttonIndex;
    HDC hdc = dis->hDC;
    RECT rc = dis->rcItem;
    UINT state = dis->itemState;

    int cx = rc.right - rc.left;
    int cy = rc.bottom - rc.top;

    int iconW, iconH, iconX, iconY;

    RECT tr;

    /* --- Select background color --- */
    HBRUSH hBrush;

    if (state & ODS_DISABLED)       hBrush = CreateSolidBrush(RGB(200, 200, 200));
    else if (state & ODS_SELECTED)  hBrush = CreateSolidBrush(RGB(180, 180, 255));
    else                            hBrush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));

    FillRect(hdc, &rc, hBrush);
    DeleteObject(hBrush);

    /* --- Icon --- */
    iconW = GetSystemMetrics(SM_CXICON);
    iconH = GetSystemMetrics(SM_CYICON);

    iconX = rc.left + (cx - iconW) / 2;
    iconY = rc.top + 6;

    DrawIconEx(hdc, iconX, iconY, pSlot->hIcon, iconW, iconH, 0, NULL, DI_NORMAL);

    /* --- Text Label --- */
    tr.left   = rc.left;
    tr.right  = rc.right;
    tr.top    = iconY + iconH;
    tr.bottom = rc.bottom;

    SetBkMode(hdc, TRANSPARENT);

    if (state & ODS_DISABLED)       SetTextColor(hdc, RGB(130, 130, 130));
    else if (state & ODS_SELECTED)  SetTextColor(hdc, RGB(0, 0, 128));
    else                            SetTextColor(hdc, RGB(0, 0, 0));

    DrawText(hdc, pSlot->szLabel, -1, &tr, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    /* --- Border --- */
    if (state & ODS_DISABLED) {
        DrawEdge(hdc, &rc, EDGE_RAISED, BF_RECT);
    }
    else if (state & ODS_SELECTED) {
        DrawEdge(hdc, &rc, EDGE_SUNKEN, BF_RECT);
    }
    else {
        DrawEdge(hdc, &rc, EDGE_RAISED, BF_RECT);
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    int wmId, wmEvent;

    switch (message)  {
        case WM_DRAWITEM:
            if (IsIdButtonSlot(wParam)) {
                DrawIconButton((LPDRAWITEMSTRUCT)lParam, GetSlotIndexFromId(wParam));
                return TRUE;
            }
            break;
        case WM_COMMAND:
            wmId    = LOWORD(wParam); 
            wmEvent = HIWORD(wParam); 
            // App Slot Buttons
            if (IsIdButtonSlot(wmId)) {
                TCHAR* pExePath = AppSlots[GetSlotIndexFromId(wmId)].szPath;
                if (pExePath && _tcslen(pExePath) > 0) {
                    STARTUPINFO si;
                    PROCESS_INFORMATION pi;
                    memset(&si, 0, sizeof(si));
                    si.cb = sizeof(si);
                    memset(&pi, 0, sizeof(pi));
                    if (!CreateProcessW(
                        pExePath,
                        NULL,
                        NULL, NULL,
                        FALSE,
                        0,
                        NULL,
                        NULL,
                        &si, &pi)) {
                        // TODO: Failed to start
                    }
                    else {
                        CloseHandle(pi.hProcess);
                        CloseHandle(pi.hThread);
                    }
                }
            }
            // Parse the menu selections:
            switch (wmId) {
                case IDM_FILE_APP: {
                    int i;
                    ShowDialogAppList(hInst, hWnd);
                    for (i = 0; i < NUM_BTN_SLOTS; ++i) {
                        HWND hBtnWnd = GetDlgItem(hWnd, GetIdFromSlotIndex(i));
                        InvalidateRect(hBtnWnd, NULL, TRUE);
                    }
                    SaveConfiguration(CONFIG_FILE_PATH);
                    break;
                }
                case IDM_HELP_ABOUT:
                    ShowDialogAbout(hInst, hWnd);
                    break;
                case IDM_FILE_EXIT:
                    DestroyWindow(hWnd);
                    break;
                default:
                   return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        case WM_CREATE: {
            int i;
            RECT rt;
            int anchorLeft, anchorTop;
    
            hwndCB = CommandBar_Create(hInst, hWnd, 1);            
            CommandBar_InsertMenubar(hwndCB, hInst, IDM_MENU, 0);
            CommandBar_AddAdornments(hwndCB, 0, 0);

            // Create button slots
            GetClientRect(hWnd, &rt);

            anchorLeft = (rt.right - rt.left - BTN_GRID_COLUMN * (BTN_SLOT_WIDTH + BTN_SLOT_MARGIN) + BTN_SLOT_MARGIN) / 2 ;
            anchorTop = (rt.bottom - rt.top - (NUM_BTN_SLOTS / BTN_GRID_COLUMN) * (BTN_SLOT_HEIGHT + BTN_SLOT_MARGIN) + BTN_SLOT_MARGIN) / 2 + BTN_GRID_MARGIN_TOP;

            for (i = 0; i < NUM_BTN_SLOTS; ++i) {
                HMENU buttonId = (HMENU)GetIdFromSlotIndex(i);
                int row = i / BTN_GRID_COLUMN;
                int col = i % BTN_GRID_COLUMN;
                int left = anchorLeft + col * (BTN_SLOT_WIDTH + BTN_SLOT_MARGIN);
                int top = anchorTop + row * (BTN_SLOT_HEIGHT + BTN_SLOT_MARGIN);
                CreateWindow(_T("BUTTON"), _T(""), WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                    left, top, BTN_SLOT_WIDTH, BTN_SLOT_HEIGHT, hWnd, buttonId, hInst, NULL);

            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            RECT rc;
            int i, n = 20;
            int r1 = 30, g1 = 144, b1 = 255;
            int r2 = 0, g2 = 255, b2 = 128;
            int height;

            GetClientRect(hWnd, &rc);
    
            height = rc.bottom - rc.top;
        
            for (i = 0; i < n; i++) {
                RECT rct;

                double t = (double)i / (n - 1);
        
                int r = (int)(r1 + (r2 - r1) * t);
                int g = (int)(g1 + (g2 - g1) * t);
                int b = (int)(b1 + (b2 - b1) * t);
        
                HBRUSH hBrush = CreateSolidBrush(RGB(r, g, b));
        
                rct.left   = rc.left;
                rct.right  = rc.right;
                rct.top    = rc.top + i * height / n;
                rct.bottom = rct.top + height / n + 1;

                FillRect(hdc, &rct, hBrush);
                DeleteObject(hBrush);
            }
            EndPaint(hWnd, &ps);
            break;
        }
        case WM_DESTROY:
            DestroySlots();
            CommandBar_Destroy(hwndCB);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

void ReloadSlotIcons() {
    int i;
    APPSLOT* pSlot;
    for (i = 0; i < NUM_BTN_SLOTS; ++i) {
        pSlot = AppSlots + i;
        if (pSlot->szPath && _tcslen(pSlot->szPath) > 0) {
            pSlot->hIcon = ExtractIconFromExe(pSlot->szPath);
        }
    }
}

void DestroySlots() {
    int i;
    APPSLOT* pSlot;

    for (i = 0; i < NUM_BTN_SLOTS; ++i) {
        pSlot = AppSlots + i;
        if (pSlot->hIcon) {
            DestroyIcon(pSlot->hIcon);
            pSlot->hIcon = NULL;
        }
        if (pSlot->szPath) {
            LocalFree(pSlot->szPath);
            pSlot->szPath = NULL;
        }
    }
}

// Save configuration to file
// Each AppSlot occupies two lines: first line is szPath, second line is szLabel
BOOL SaveConfiguration(LPCTSTR szFileName) {
    HANDLE hFile;
    DWORD dwBytesWritten;
    int i;
    APPSLOT* pSlot;
    TCHAR szNewLine[] = _T("\r\n");
    DWORD dwLabelLen;

    hFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
    
    for (i = 0; i < NUM_BTN_SLOTS; ++i) {
        pSlot = AppSlots + i;
        
        // Write szPath (first line)
        if (pSlot->szPath) {
            DWORD dwPathLen = lstrlen(pSlot->szPath);
            if (!WriteFile(hFile, pSlot->szPath, dwPathLen * sizeof(TCHAR), &dwBytesWritten, NULL)) {
                CloseHandle(hFile);
                return FALSE;
            }
        }
        
        // Write newline after path
        
        if (!WriteFile(hFile, szNewLine, lstrlen(szNewLine) * sizeof(TCHAR), &dwBytesWritten, NULL)) {
            CloseHandle(hFile);
            return FALSE;
        }
        
        // Write szLabel (second line)
        dwLabelLen = lstrlen(pSlot->szLabel);
        if (!WriteFile(hFile, pSlot->szLabel, dwLabelLen * sizeof(TCHAR), &dwBytesWritten, NULL)) {
            CloseHandle(hFile);
            return FALSE;
        }
        
        // Write newline after label
        if (!WriteFile(hFile, szNewLine, lstrlen(szNewLine) * sizeof(TCHAR), &dwBytesWritten, NULL)) {
            CloseHandle(hFile);
            return FALSE;
        }
    }
    
    CloseHandle(hFile);
    return TRUE;
}

// Load configuration from file
// Each AppSlot occupies two lines: first line is szPath, second line is szLabel
BOOL LoadConfiguration(LPCTSTR szFileName) {
    HANDLE hFile;
    DWORD dwFileSize, dwBytesRead;
    TCHAR* pFileBuffer;
    TCHAR* pCurrentPos;
    int i;
    APPSLOT* pSlot;
    
    hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
    
    // Get file size
    dwFileSize = GetFileSize(hFile, NULL);
    if (dwFileSize == INVALID_FILE_SIZE) {
        CloseHandle(hFile);
        return FALSE;
    }
    
    // Allocate buffer for file content
    pFileBuffer = (TCHAR*)LocalAlloc(LPTR, dwFileSize + sizeof(TCHAR));
    if (!pFileBuffer) {
        CloseHandle(hFile);
        return FALSE;
    }
    
    // Read entire file
    if (!ReadFile(hFile, pFileBuffer, dwFileSize, &dwBytesRead, NULL)) {
        LocalFree(pFileBuffer);
        CloseHandle(hFile);
        return FALSE;
    }
    
    CloseHandle(hFile);
    
    // Null-terminate the buffer
    pFileBuffer[dwFileSize / sizeof(TCHAR)] = _T('\0');
    
    pCurrentPos = pFileBuffer;
    
    for (i = 0; i < NUM_BTN_SLOTS; ++i) {
        TCHAR* szPathLine = pCurrentPos;
        TCHAR* szLabelLine;

        pSlot = AppSlots + i;

        // Find end of path line (first line)
        while (*pCurrentPos && *pCurrentPos != _T('\r') && *pCurrentPos != _T('\n')) {
            pCurrentPos++;
        }
        
        // Null-terminate path line
        if (*pCurrentPos) {
            *pCurrentPos = _T('\0');
            pCurrentPos++;
        }
        
        // Skip newline characters
        while (*pCurrentPos == _T('\r') || *pCurrentPos == _T('\n')) {
            pCurrentPos++;
        }
        
        szLabelLine = pCurrentPos;
        
        // Find end of label line (second line)
        while (*pCurrentPos && *pCurrentPos != _T('\r') && *pCurrentPos != _T('\n')) {
            pCurrentPos++;
        }
        
        // Null-terminate label line
        if (*pCurrentPos) {
            *pCurrentPos = _T('\0');
            pCurrentPos++;
        }
        
        // Skip newline characters
        while (*pCurrentPos == _T('\r') || *pCurrentPos == _T('\n')) {
            pCurrentPos++;
        }
        
        // Copy path (allocate memory if needed)
        if (pSlot->szPath) {
            LocalFree(pSlot->szPath);
            pSlot->szPath = NULL;
        }
        
        if (_tcslen(szPathLine) > 0) {
            pSlot->szPath = (TCHAR*)LocalAlloc(LPTR, (lstrlen(szPathLine) + 1) * sizeof(TCHAR));
            if (pSlot->szPath) {
                lstrcpy(pSlot->szPath, szPathLine);
            }
        }
        
        // Copy label (fixed buffer)
        if (_tcslen(szLabelLine) > 0) {
            _tcscpy(pSlot->szLabel, szLabelLine);
        } else {
            pSlot->szLabel[0] = _T('\0');
        }
        
        // If we reached end of file, break early
        if (!*pCurrentPos) {
            break;
        }
    }
    
    LocalFree(pFileBuffer);
    return TRUE;
}