//==========================================================================\
// main.c: finestra principale applicazione                                 |
//                                                                          |
// funzioni: -------------------------------------------------------------- |
// int main(void);                                                          |
// MRESULT EXPENTRY MainDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
// VOID ProcessControl(HWND hwnd, ULONG ulId, ULONG ulEvent, HWND hctl);
// VOID ProcessCommand(HWND hwnd, ULONG ulId);
// VOID _StartProcessing(PAPPDATA pad, HWND h1, HWND h2);
// BOOL MainInit(PAPPDATA pad);
// BOOL MainDlgInit(PAPPDATA pad);
// BOOL MainEnd(PAPPDATA pad, BOOL rc);
// BOOL enableBtns(HWND hwnd);
// MRESULT EXPENTRY dropLbxProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
//==========================================================================/

#pragma strings(readonly)

#define INCL_WIN
#define INCL_DOSMISC
#define INCL_GPI
#define INCL_DOSMODULEMGR
#define INCL_DOSFILEMGR
#define INCL_DOSPROCESS
#include <os2.h>
#include <winutil.h>
#include <string.h>
#include <stdlib.h>
#include <afcbsu00.h>
#include <afcpmu00.h>
#include "msgs.h"
#include "main.h"
#include "api.h"


// prototipi funzioni

MRESULT EXPENTRY MainDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID ProcessControl(HWND hwnd, ULONG ulId, ULONG ulEvent, HWND hctl);
VOID ProcessCommand(HWND hwnd, ULONG ulId);
VOID _StartProcessing(PAPPDATA pad, HWND h1, HWND h2);
BOOL MainInit(PAPPDATA pad, int argc, char** argv);
BOOL MainDlgInit(PAPPDATA pad);
BOOL MainEnd(PAPPDATA pad, BOOL rc);
MRESULT EXPENTRY dropLbxProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

// variabili globali

static PFNWP pfnwplbox;
static PAPPDATA pg;

//==========================================================================\
// main                                                                     |
//==========================================================================/

int main(int argc, char** argv) {
   PVOID pbase;
   PAPPDATA pad;
   QMSG qmsg;

   // alloca la memoria per la struttura dati dell'applicazione
   if (!(pad = malloc(sizeof(APPDATA)))) {
      Wprint(HWND_DESKTOP, SZERR_ALLOCMAIN, 0);
      return 2;
   } /* endif */
   memset(pad, 0, sizeof(APPDATA));
   pg = pad;

   // inizializzazione applicazione -----------------------------------------
   if (MainInit(pad, argc, argv)) return 1;

   // creazione dialogo -----------------------------------------------------
   if (MainDlgInit(pad)) return 1;

   // loop messaggi
   forever {
      if(WinGetMsg(pad->hab, &qmsg, NULLHANDLE, 0, 0)) {    // loop standard
         WinDispatchMsg(pad->hab, &qmsg);
      } else {                   // WM_QUIT
         if(qmsg.hwnd == NULLHANDLE)                       // SHUTDOWN
            WinPostMsg(pad->hMain, WM_CLOSE, MPFROMLONG(TRUE), NULL);
         else if(qmsg.hwnd == HWNDFROMMP(qmsg.mp2))        // TASKLIST
            WinPostMsg(qmsg.hwnd, WM_CLOSE, 0L, 0L);
         else                            // chiusura regolare: termina
            break;
      } // end if
   } // end forever

   return (int)MainEnd(pad, FALSE);
}


//==========================================================================\
// procedura dialogo principale                                             |
// messaggi:                                                                |
// WM_INITDLG                                                               |
// WM_QUERYTRACKINFO                                                        |
// WM_QUERYBORDERSIZE                                                       |
// WM_WINDOWPOSCHANGED                                                      |
// WM_MINMAXFRAME                                                           |
// WM_CONTROL                                                               |
// WM_COMMAND                                                               |
// WM_BUBBLEHELP                                                            |
// WM_CLOSE                                                                 |
// WM_QUERYFRAMEINFO                                                        |
//==========================================================================/

MRESULT EXPENTRY MainDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
   switch (msg) {
      case WM_INITDLG:
         WinSetWindowPtr(hwnd, 0L, (PVOID)mp2);
         pfnwplbox = WinSubclassWindow(WinWindowFromID(hwnd, LB_FILE),
                                       dropLbxProc);
         break;
      case WM_CONTROL:
         ProcessControl(hwnd, (ULONG)SHORT1FROMMP(mp1),
                        (ULONG)SHORT2FROMMP(mp1), (HWND)mp2);
         break;
      case WM_COMMAND:
         ProcessCommand(hwnd, (ULONG)mp1);
         break;
      case WM_STARTPROCESSING:
         _StartProcessing((PAPPDATA)GetWinData(hwnd), (HWND)mp1, (HWND)mp2);
         break;
      case WM_DEFERFOCUS:
         WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, (ULONG)mp1));
         break;
      case WM_MOUSEMOVE: {
         PAPPDATA pad = GetWinData(hwnd);
         if (pad->is.working) return (MRESULT)WsetWaitPtr();
         return WinDefDlgProc(hwnd, msg, mp1, mp2);
      }
      case WM_CONTROLPOINTER: {
         PAPPDATA pad = GetWinData(hwnd);
         HPOINTER hptr = WinQuerySysPointer(HWND_DESKTOP, SPTR_WAIT, FALSE);
         if (pad->is.working && hptr) return (MRESULT)hptr;
         return WinDefDlgProc(hwnd, msg, mp1, mp2);
      }
      case WM_OBJWERROR: {
         PAPPDATA pad = GetWinData(hwnd);
         if ((HWND)mp1 == pad->hObj && (HWND)mp2 == pad->hObj) {
            WinEnableWindow(pad->hMain, TRUE);
            Wprint(hwnd, SZERR_PROCESSING, 0);
            WinEnableControl(hwnd, BTN_START, TRUE);
            WinEnableControl(hwnd, BTN_EXIT, TRUE);
            pad->is.error = 0;
            pad->is.working = 0;
         } // end if
      }  break;
      case WM_CLOSE: {
         PAPPDATA pad = GetWinData(hwnd);
         if (pad->is.working) {
            pad->is.ending = 1;
            DosSleep(1);
            WinPostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
         } else {
            WinPostMsg(hwnd, WM_QUIT, 0, 0);
         } /* endif */
      }  break;
      default:
      return WinDefDlgProc(hwnd, msg, mp1, mp2);
   } /* endswitch */
   return (MRESULT)FALSE;
}


//==========================================================================\
// determina quale pagina del notebook Š attiva, assegnandone il valore     |
// corrispondente a pad->ulCurrPage. Se la pagina richiede il bottone apply |
// lo abilita, altrimenti lo disabilita                                     |
// parametri:                                                               |
// HWND hwnd: handle finestra principale dialogo                            |
// MPARAM mp1: ID controllo + codice notifica                               |
// MPARAM mp2: dati relativi all'evento                                     |
// valore restituito:                                                       |
// VOID                                                                     |
//==========================================================================/

VOID ProcessControl(HWND hwnd, ULONG ulId, ULONG ulEvent, HWND hctl) {
   switch (ulId) {
      // listbox file
      case LB_FILE:
         if (ulEvent == LN_SELECT) {        // selezione item
            SHORT item = DlgLboxQuerySelectedItem(hwnd, LB_FILE);
            WinCheckButton(hwnd, CHK_RECUR, (item == LIT_NONE?
                                           0: wLboxQueryItemHnd(hctl, item)));
            WinEnableControl(hwnd, CHK_RECUR, item != LIT_NONE);
            WinEnableControl(hwnd, BTN_REM0, item != LIT_NONE);
            if (item != LIT_NONE) WsetDefBtn(hwnd, BTN_REM0);
         } else if (ulEvent == LN_ENTER) {  // doppio clic su item
            PAPPDATA pad = GetWinData(hwnd);
            SHORT item = wLboxQuerySelItem(hctl);
            PSZ pszItem = malloc(MAXPATH);
            if (!pszItem) {
               Wprint(HWND_DESKTOP, SZERR_ALLOC, 0);
               break;
            } /* endif */
            wLboxQueryItemText(hctl, item, pszItem, MAXPATH);
            WinSetDlgItemText(hwnd, EF_FILE, pszItem);
            free(pszItem);
            WsetDefBtn(hwnd, BTN_REM0);
         } // end if
         break;
      // entry field FILE
      case EF_FILE:
         if (ulEvent == EN_CHANGE) {
            ULONG cb = WinQueryWindowTextLength(hctl);
            WinEnableControl(hwnd, BTN_ADD0, cb);
         } else if (ulEvent == EN_SETFOCUS) {
            WsetDefBtn(hwnd, BTN_ADD0);
         } /* endif */
         break;
      // checkbox Recur subdirectories
      case CHK_RECUR:
         if (ulEvent == BN_CLICKED || ulEvent == BN_DBLCLICKED)
            dLboxSetSelItemHnd(hwnd, LB_FILE,
                               WinQueryButtonCheckstate(hwnd, CHK_RECUR));
         break;
      // listbox strings
      case LB_STRINGS:
         if (ulEvent == LN_SELECT) {
            FRIDATA fd;
            SHORT item = wLboxQuerySelItem(hctl);
            SHORT iMax = wLboxQueryItemCount(hctl) - 1;
            WinEnableControl(hwnd, BTN_MOVEUP, item > 0);
            WinEnableControl(hwnd, BTN_MOVEDOWN, (item >= 0) && (item < iMax));
            fd.ul = (item >= 0) ? wLboxQueryItemHnd(hctl, item): 0;
            resetStrCtls(hwnd, item != LIT_NONE, fd.fri.ins, fd.fri.esc);
         } else if (ulEvent == LN_ENTER) {
            PAPPDATA pad = GetWinData(hwnd);
            SHORT item = wLboxQuerySelItem(hctl);
            PSZ pszItem = malloc(MAXSTRITEM);
            FRIDATA fd;
            if (!pszItem) {
               Wprint(HWND_DESKTOP, SZERR_ALLOC, 0);
               break;
            } /* endif */
            wLboxQueryItemText(hctl, item, pszItem, MAXSTRITEM);
            fd.ul = wLboxQueryItemHnd(hctl, item);
            *(pszItem + fd.fri.cbfind) = 0;
            WinSetDlgItemText(hwnd, EF_STRFIND, pszItem);
            WinSetDlgItemText(hwnd, EF_STRREPL, pszItem + fd.fri.cbfind + 4);
            free(pszItem);
            WsetDefBtn(hwnd, BTN_REM1);
         } // end if
         break;
      // entryfield Find
      case EF_STRFIND:
         if (ulEvent == EN_CHANGE) {
            ULONG cb = WinQueryWindowTextLength(hctl);
            WinEnableControl(hwnd, BTN_ADD1, cb);
           // WinEnableControl(hwnd, BTN_FOCUSREPL, cb);
         } else if (ulEvent == EN_SETFOCUS) {
            WsetDefBtn(hwnd, BTN_FOCUSREPL);
         } /* endif */
         break;
      // entryfield Replace
      case EF_STRREPL:
         if (ulEvent == EN_SETFOCUS)
            WsetDefBtn(hwnd, BTN_ADD1);
         break;
      // checkbox Case insensitive
      case CHK_NOCASE:
         if (ulEvent == BN_CLICKED || ulEvent == BN_DBLCLICKED) {
            SHORT item;
            hctl = WinWindowFromID(hwnd, LB_STRINGS);
            item = wLboxQuerySelItem(hctl);
            if (item >= 0) {
               FRIDATA fd;
               fd.ul = wLboxQueryItemHnd(hctl, item);
               fd.fri.ins = WinQueryButtonCheckstate(hwnd, CHK_NOCASE);
               wLboxSetItemHnd(hctl, item, fd.ul);
            } // end if
         } // end if
         break;
      // checkbox Escape chars
      case CHK_ESCAPE:
         if (ulEvent == BN_CLICKED || ulEvent == BN_DBLCLICKED) {
            FRIDATA fd;
            SHORT item;
            hctl = WinWindowFromID(hwnd, LB_STRINGS);
            item = wLboxQuerySelItem(hctl);
            if (item < 0) break;
            fd.ul = wLboxQueryItemHnd(hctl, item);
            fd.fri.esc = WinQueryButtonCheckstate(hwnd, CHK_ESCAPE);
            if (fd.fri.esc) {
               if (chkEscSeq(hctl, item)) {
                  wLboxSetItemHnd(hctl, item, fd.ul);
               } else {
                 WinCheckButton(hwnd, CHK_ESCAPE, FALSE);
               } /* endif */
            } else {
               wLboxSetItemHnd(hctl, item, fd.ul);
            } // end if
         } // end if
         break;
   } /* endswitch */
}


//==========================================================================\
// HWND hwnd: handle finestra principale dialogo                            |
// MPARAM mp1: ID controllo                                                 |
// valore restituito:                                                       |
// MRESULT FALSE                                                            |
//==========================================================================/

VOID ProcessCommand(HWND hwnd, ULONG ulId) {
   PAPPDATA pad = GetWinData(hwnd);
   switch (ulId) {
      case BTN_FINDCLP:
      case BTN_REPLACECLP: {
         HWND hef = WinWindowFromID(hwnd, ulId + 1);
         WinSetWindowText(hef, SZ_NULL);
         WinSendMsg(hef, EM_PASTE, MPVOID, MPVOID);
         WinPostMsg(hwnd, WM_DEFERFOCUS, (MPARAM)(ulId + 1), MPVOID);
      }  break;
      case BTN_FIND: {
         PFILEDLG pfdlg = fileDlg(hwnd, pad->achpath, SZ_ADDEXT,
                                  SZ_ADDTOLIST, SZ_ADD, 1);
         if (pfdlg) {
            INT i;
            PSZ psz;
            // se si inserisce il nome di un file non in lista con un
            // dialogo a selezione multipla ulFQFCount sar… 1, ma il
            // membro papszFQFilename sar… NULL per questo Š necessario
            // controllarne il valore
            if (pfdlg->papszFQFilename) {
               for (i = 0; i < pfdlg->ulFQFCount; ++i) {
                  if (DlgLboxSearchString(hwnd, LB_FILE,
                                          *pfdlg->papszFQFilename[i], 0) ==
                      LIT_NONE) {
                     DlgLboxInsertItem(hwnd, LB_FILE, LIT_SORTASCENDING,
                                       *pfdlg->papszFQFilename[i]);
                  } /* endif */
               } /* endfor */
               // aggiorna percorso corrente
               psz = ioFNameFromPath(*pfdlg->papszFQFilename[0]);
               *psz = 0;
               strcpy(pad->achpath, *pfdlg->papszFQFilename[0]);
               WinFreeFileDlgList(pfdlg->papszFQFilename);
            } else {
               if (DlgLboxSearchString(hwnd, LB_FILE, pfdlg->szFullFile, 0) ==
                   LIT_NONE)
                  DlgLboxInsertItem(hwnd, LB_FILE, LIT_SORTASCENDING,
                                    pfdlg->szFullFile);
               psz = ioFNameFromPath(pfdlg->szFullFile);
               *psz = 0;
               strcpy(pad->achpath, pfdlg->szFullFile);
            } /* endif */
            // aggiorna percorso corrente
            free(pfdlg);
            enableBtns(hwnd);
         } /* endif */
      }  break;
      case BTN_ADD0: {
         PSZ psz = malloc(MAXPATH);
         if (!psz) {
            Wprint(HWND_DESKTOP, SZERR_ALLOC, 0);
            break;
         } /* endif */
         WinQueryDlgItemText(hwnd, EF_FILE, MAXPATH, psz);
         if (LIT_NONE == DlgLboxSearchString(hwnd, LB_FILE, psz, 0)) {
            DlgLboxInsertItem(hwnd, LB_FILE, LIT_SORTASCENDING, psz);
         } // end if
         // aggiorna percorso corrente ricerca file
         strcpy(pad->achpath, psz);
         free(psz);
         psz = ioFNameFromPath(pad->achpath);
         *psz = 0;
         WinSetDlgItemText(hwnd, EF_FILE, SZ_NULL);
         enableBtns(hwnd);
         if (LIT_NONE != DlgLboxQuerySelectedItem(hwnd, LB_FILE))
            WsetDefBtn(hwnd, BTN_REM0);
         else
            WinPostMsg(hwnd, WM_DEFERFOCUS, (MPARAM)EF_FILE, MPVOID);
      }  break;
      case BTN_REM0:
         DlgLboxDeleteSelectedItem(hwnd, LB_FILE);
         enableBtns(hwnd);
         WinEnableControl(hwnd, CHK_RECUR, FALSE);
         WinCheckButton(hwnd, CHK_RECUR, 0);
         break;
      case BTN_ADD1: {
         PSZ pszFind, pszRepl, pszItem;
         SHORT i;
         FRIDATA fd;
         HWND hlbstr = WinWindowFromID(hwnd, LB_STRINGS);
         fd.ul = 0;
         pszFind = malloc(MAXSTR);
         pszRepl = malloc(MAXSTR);
         pszItem = malloc(MAXSTRITEM);
         fd.fri.cbfind = WinQueryDlgItemText(hwnd, EF_STRFIND, MAXSTR, pszFind);
         WinQueryDlgItemText(hwnd, EF_STRREPL, MAXSTR, pszRepl);
         pszcpy(pszcpy(pszcpy(pszItem, pszFind), " -> "), pszRepl);
         if ((i = wLboxInsItem(hlbstr, LIT_END, pszItem)) >= 0) {
            wLboxSetTopIdx(hlbstr, i);
            wLboxSetItemHnd(hlbstr, i, fd.ul);
            WinSetDlgItemText(hwnd, EF_STRFIND, SZ_NULL);
            WinSetDlgItemText(hwnd, EF_STRREPL, SZ_NULL);
         } /* endif */
         free(pszFind);
         free(pszRepl);
         free(pszItem);
         enableBtns(hwnd);
         if (LIT_NONE != wLboxQuerySelItem(hlbstr))
            WsetDefBtn(hwnd, BTN_REM1);
         else
            WinPostMsg(hwnd, WM_DEFERFOCUS, (MPARAM)EF_STRFIND, MPVOID);
      }  break;
      case BTN_REM1:
         DlgLboxDeleteSelectedItem(hwnd, LB_STRINGS);
         resetStrCtls(hwnd, 0, 0, 0);
         enableBtns(hwnd);
         WinEnableControl(hwnd, BTN_MOVEUP, FALSE);
         WinEnableControl(hwnd, BTN_MOVEDOWN, FALSE);
         break;
      case BTN_REMALL:
         DlgLboxDeleteAll(hwnd, LB_STRINGS);
         resetStrCtls(hwnd, 0, 0, 0);
         enableBtns(hwnd);
         break;
      case BTN_FOCUSREPL:      // bottone nascosto
         WinPostMsg(hwnd, WM_DEFERFOCUS, (MPARAM)EF_STRREPL, MPVOID);
         break;
      case BTN_LOAD:
         if (fileDlg(hwnd, pad->achload, SZ_BEPEXT, SZ_BEPTITLE, SZ_BEPBTN, 0))
            WinPostMsg(pad->hObj, OBJM_LOAD, (MPARAM)pad->hMain, (MPARAM)pad);
         break;
      case BTN_SAVE:
         if (fileDlg(hwnd, pad->achload, SZ_BEPEXT, SZ_SVTITLE, SZ_SVBTN, 2))
            WinPostMsg(pad->hObj, OBJM_SAVE, (MPARAM)pad->hMain, (MPARAM)pad);
         break;
      case BTN_START:
         WinEnableControl(hwnd, BTN_START, FALSE);
         WinEnableControl(hwnd, BTN_EXIT, FALSE);
         WinPostMsg(pad->hObj, OBJM_START,
                    (MPARAM)pad->hMain, (MPARAM)pad->hMain);
         break;
      case BTN_EXIT:
         WinPostMsg(hwnd, WM_QUIT, 0, 0);
         break;
      case BTN_MOVEUP:
         moveItem(hwnd, -1);
         break;
      case BTN_MOVEDOWN:
         moveItem(hwnd, 1);
         break;
   } /* endswitch */
}


//==========================================================================\
// Inizia la procedura di ricerca e sostituzione
// parametri:                                                               |
// PAPPDATA pad: indirizzo struttura dati applicazione                      |
// HWND h1, HWND h2: flag controllo validit… messaggio
// valore restituito:                                                       |
// BOOL rc: TRUE = errore, FALSE = successo                                 |
//==========================================================================/

VOID _StartProcessing(PAPPDATA pad, HWND h1, HWND h2) {
   if (h1 == pad->hObj && h2 == pad->hObj) {
      PROGRESSDLG prgs;
      memset((PVOID)&prgs, 0, sizeof(PROGRESSDLG));
      prgs.hNotify = pad->hObj;
      prgs.color = 0x80;
      prgs.pszTitle = SZ_SUBSTIT;
      prgs.pszPrgrss = SZ_PROGRESS;
      prgs.pszTime = SZ_ELAPSED;
      prgs.pszStop = SZ_STOP;
      prgs.FontNameSize = pad->is.osver > 3? "9.WarpSans": "8.Helv";
      prgs.fl = PRGSS_CENTER;
      WprogressBar(HWND_DESKTOP, pad->hMain, &prgs);
      pad->is.error = 0;
      WinEnableControl(pad->hMain, BTN_START, TRUE);
      WinEnableControl(pad->hMain, BTN_EXIT, TRUE);
   } // end if
}


//==========================================================================\
// Inizializzazione applicazione                                            |
// parametri:                                                               |
// PAPPDATA pad: indirizzo struttura dati applicazione                      |
// valore restituito:                                                       |
// BOOL rc: TRUE = errore, FALSE = successo                                 |
//==========================================================================/

BOOL MainInit(PAPPDATA pad, INT argc, char** argv) {
   // controlla versione S.O.
   pad->is.osver = sysinfo(QSV_VERSION_MINOR) / 10;
   // inizializza path ricerca file da editare e path profili da caricare
   strcpy(pad->achpath, "X:\\");
   *pad->achpath = 'A' + sysinfo(QSV_BOOT_DRIVE) - 1;
   strcpy(pad->achload, pad->achpath);
   // avvia thread secondario
   if (StartObjectThread(pad)) return MainEnd(pad, TRUE);
   // se sono definiti argomenti prova ad aprire profilo editazione
   if (argc == 2 && ioFExists(argv[1], NULL)) {
      strcpy(pad->achload, argv[1]);
      pad->is.loadarg = 1;      // setta flag che provoca caricamento profilo
   } /* endif */
   return FALSE;
}


//==========================================================================\
// procedura creazione e inizializzazione dialogo                           |
// setta l'icona del dialogo, legge le preferenze correnti, legge precedente|
// posizione/dimensioni finestra dal file INI, inserisce le pagine nel      |
// notebook, mostra la finestra                                             |
// parametri:                                                               |
// PAPPDATA pad : indirizzo struttura dati applicazione                     |
// valore restituito:                                                       |
// BOOL : TRUE = errore , FALSE = SUCCESSO                                  |
//==========================================================================/

BOOL MainDlgInit(PAPPDATA pad) {
   HPOINTER hico;
   // inizializza PM
   pad->hmq = WinCreateMsgQueue((pad->hab = WinInitialize(0)), 0);

   // crea dialogo
   if (NULLHANDLE == (pad->hMain = WinLoadDlg(HWND_DESKTOP, NULLHANDLE,
                                             (PFNWP)MainDlgProc,
                                             NULLHANDLE, DLG_MAIN, pad))) {
      Wprint(HWND_DESKTOP, SZ_LOADDLGFAILURE, PMPRNT_ERROR);
      return MainEnd(pad, TRUE);
   } // end if

   // setta icona dialogo
   WsetDlgIcon(pad->hMain, NULLHANDLE, 1);
   WsetDlgFonts(pad->hMain, NULLHANDLE,
                (pad->is.osver > 3? "9.WarpSans": "8.Helv"));
   WcenterWindow(pad->hMain);
   if (pad->is.loadarg)
      WinPostMsg(pad->hObj, OBJM_LOAD, (MPARAM)(pad->hMain), (MPARAM)pad);
   return FALSE;
}


//==========================================================================\
// liberazione risorse e terminazione (flag = TRUE -> CheckInstance(0L))    |
// parametri:                                                               |
// PAPPDATA pad: indirizzo struttura dati applicazione                      |
// BOOL flag: se TRUE setta a NULLHANDLE l'handle della finestra delle      |
//            impostazioni memorizzato nell'area condivisa di SmartWin      |
// BOOL rc: valore da restituire                                            |
// valore restituito:                                                       |
// BOOL = a parametro BOOL rc                                               |
//==========================================================================/

BOOL MainEnd(PAPPDATA pad, BOOL rc) {
   pad->is.closing = 1;
   if (pad->hMain) WinDestroyWindow(pad->hMain);
   if (pad->hmq) WinDestroyMsgQueue(pad->hmq);
   WinTerminate(pad->hab);
   free(pad);
   return rc;
}


//==========================================================================\
// Subclassa listbox per fargli accettare il drop di oggetti file           |
//==========================================================================/

MRESULT EXPENTRY dropLbxProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
   if (msg == DM_DRAGOVER) {
      if (DrgAccessDraginfo((PDRAGINFO)mp1)) {
         PDRAGITEM pditem;
         LONG cItem;
         MRESULT mr = MRFROM2SHORT(DOR_NEVERDROP, 0);
         if (0 != (cItem = DrgQueryDragitemCount((PDRAGINFO)mp1))) {
            while (--cItem >= 0) {
               pditem = DrgQueryDragitemPtr((PDRAGINFO)mp1, cItem);
               if (DrgVerifyRMF(pditem, "DRM_OS2FILE", NULL)) {
                  mr = MRFROM2SHORT(DOR_DROP, DO_MOVE);
                  break;
               } /* endif */
            } /* endwhile */
         } /* endif */
         DrgFreeDraginfo((PDRAGINFO)mp1);
         return mr;
      } /* endif */
      return MRFROM2SHORT(DOR_NEVERDROP, 0);
   } else if (msg == DM_DROP) {
      PDRAGITEM pditem;         // indirizzo struttura dragitem
      LONG cItem;               // numero item draggati
      CHAR buf[260];
      ULONG cb;
      FILESTATUS3 fs;
      *buf = 0;
      if (DrgAccessDraginfo((PDRAGINFO)mp1) &&
          0 != (cItem = DrgQueryDragitemCount((PDRAGINFO)mp1))) {
         while (--cItem >= 0) {
            pditem = DrgQueryDragitemPtr((PDRAGINFO)mp1, cItem);
            // se l'oggetto Š un file (o directory)
            if (DrgVerifyRMF(pditem, "DRM_OS2FILE", NULL) &&
            // costruisce nome completo file
            0 != (cb = DrgQueryStrName(pditem->hstrContainerName, 260, buf)) &&
                DrgQueryStrName(pditem->hstrSourceName, 260 - cb, buf + cb) &&
            // controlla attributi file
             !DosQueryPathInfo(buf, FIL_STANDARD, &fs, sizeof(FILESTATUS3))) {
               // se directory appende *.*
               if (fs.attrFile & FILE_DIRECTORY) strcat(buf, "\\*.*");
               // se file non Š gi… in lista lo aggiunge
               if (LIT_NONE == (SHORT)pfnwplbox(hwnd, LM_SEARCHSTRING,
                                            MPFROM2SHORT(0, LIT_FIRST), buf)) {
                  pfnwplbox(hwnd, LM_INSERTITEM,
                            (MPARAM)LIT_SORTASCENDING, buf);
                  enableBtns(WinQueryWindow(hwnd, QW_PARENT));
               } /* endif */
            } /* endwhile */
         } /* endif */
         DrgFreeDraginfo((PDRAGINFO)mp1);
      } /* endif */
      if (*buf) {
         PSZ psz = ioFNameFromPath(buf);
         *psz = 0;
         strcpy(pg->achpath, buf);
      } /* endif */
      return (MRESULT)FALSE;
   } /* endif */
   return pfnwplbox(hwnd, msg, mp1, mp2);
}
