//==========================================================================\
// (c) 1999 * Alessandro Cantatore * Team OS/2 Italy                        |
// API wrapper
//==========================================================================/

#pragma strings(readonly)


#define INCL_WIN
#define INCL_DOSSEMAPHORES
#define INCL_DOSPROCESS
#define INCL_DOSMODULEMGR
#define INCL_DOSMISC
#define INCL_GPI
#include <os2.h>
#include <stdlib.h>


ULONG sysinfo(ULONG ulid) {
   ULONG ul = 0;
   DosQuerySysInfo(ulid, ulid, &ul, sizeof(ULONG));
   return ul;
}


HWND winid(HWND hwnd, ULONG id) {
   return WinWindowFromID(hwnd, id);
}


BOOL wintxt(HWND hwnd, PSZ psz) {
   return WinSetWindowText(hwnd, psz);
}


MRESULT DlgSend(HWND hwnd, ULONG id, ULONG msg, MPARAM mp1, MPARAM mp2) {
   return WinSendDlgItemMsg(hwnd, id, msg, mp1, mp2);
}


MRESULT Send(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
   return WinSendMsg(hwnd, msg, mp1, mp2);
}


BOOL Post(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
   return WinPostMsg(hwnd, msg, mp1, mp2);
}


BOOL Enable(HWND hwnd, BOOL fl) {
   return WinEnableWindow(hwnd, fl);
}
