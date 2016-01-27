// API wrapper header

#ifndef API_H_
   #define API_H_


HWND winid(HWND hwnd, ULONG id);
BOOL wintxt(HWND hwnd, PSZ psz);
MRESULT DlgSend(HWND hwnd, ULONG id, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT Send(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
BOOL Post(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
BOOL Enable(HWND hwnd, BOOL fl);


#undef WinWindowFromID
#undef WinSetWindowText
#undef WinSendDlgItemMsg
#undef WinSendMsg
#undef WinPostMsg
#undef WinEnableWindow

#undef malloc
#undef free
#undef _heapmin

#define WinWindowFromID winid
#define WinSetWindowText wintxt
//#define WinSetDlgItemText SetItemText
#define WinSendDlgItemMsg DlgSend
#define WinSendMsg Send
#define WinPostMsg Post
#define WinEnableWindow Enable

#define malloc sts_malloc
#define free(p)  sts_free(p); (p) = NULL
#define _heapmin sts_heapmin

#endif
