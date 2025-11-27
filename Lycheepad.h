#ifndef _LYCHEEPAD_H_
#define _LYCHEEPAD_H_

#include <windows.h>

#define BTN_SLOT_ID_START   2001
#define NUM_BTN_SLOTS       12
#define BTN_SLOT_WIDTH      68
#define BTN_SLOT_HEIGHT     60
#define BTN_SLOT_MARGIN     6
#define BTN_GRID_COLUMN     3
#define BTN_GRID_MARGIN_TOP 12

#define IsIdButtonSlot(id)      ((id) >= BTN_SLOT_ID_START && (id) < BTN_SLOT_ID_START + NUM_BTN_SLOTS)
#define GetSlotIndexFromId(id)  ((id) - BTN_SLOT_ID_START)
#define GetIdFromSlotIndex(idx) ((idx) + BTN_SLOT_ID_START)

typedef struct tagAppSlot {
    HICON   hIcon;
    TCHAR   szLabel[40];
    TCHAR*  szPath;
} APPSLOT;

extern APPSLOT AppSlots[NUM_BTN_SLOTS];

#endif