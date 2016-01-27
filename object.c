//==========================================================================\
// object.c: crea thread secondario con object window                       |
// funzioni: -------------------------------------------------------------- |
// BOOL StartObjectThread(PAPPDATA pad);
// VOID _System ObjThreadProc(ULONG ulArg);
// MRESULT EXPENTRY ObjWinProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
// VOID StartProcessing(PAPPDATA pad, HWND hw1, HWND hw2);
// VOID TerminateObjProc(PAPPDATA pad);
// VOID OWProgress(HWND hwnd, HWND hProgress);
// LONG countBytes(PPROCESSTREEDATA pptd);
// LONG FindNReplace(PPROCESSTREEDATA pptd);
//==========================================================================/

#pragma strings(readonly)

#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSNLS
#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include <winutil.h>
#include <string.h>
#include <stdlib.h>
#include <afcbsu00.h>
#include <afcpmu00.h>
#include "msgs.h"
#include "main.h"
#include "api.h"


#define THREADSTACKSZ     8192L
#define SEM_TIMEOUT       1000L
#define OBJWINCLASS       "MainObjWin"


// prototipi funzioni:
//BOOL StartObjectThread(PAPPDATA pad);
VOID _System ObjThreadProc(ULONG ulArg);
MRESULT EXPENTRY ObjWinProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID StartProcessing(PAPPDATA pad, HWND hw1, HWND hw2);
VOID TerminateObjProc(PAPPDATA pad);
VOID OWProgress(HWND hwnd, HWND hProgress);
LONG countBytes(PPROCESSTREEDATA pptd);
LONG FindNReplace(PPROCESSTREEDATA pptd);
BOOL compare(PVOID p1, PVOID p2, ULONG cb, BOOL fl);


//==========================================================================\
// Inizia il thread secondario in cui verr… creata la object window         |
// parametri:                                                               |
// PAPPDATA pad : struttura dati applicazione                               |
// valore restituito:                                                       |
// BOOL:    TRUE  = errore                                                  |
//          FALSE = successo                                                |
//==========================================================================/

BOOL StartObjectThread(PAPPDATA pad) {
   APIRET rc;
   DosCreateEventSem((PSZ)NULL, &pad->hevObj, 0L, FALSE);
   rc = DosCreateThread(&pad->tidObj, (PFNTHREAD)ObjThreadProc, (ULONG)pad,
                        CREATE_READY | STACK_SPARSE, THREADSTACKSZ);
   if (WinWaitEventSem(pad->hevObj, SEM_TIMEOUT)) rc = TRUE;
   DosCloseEventSem(pad->hevObj);
   return(BOOL)rc;
}


//==========================================================================\
// Thread secondario                                                        |
// parametri:                                                               |
//  |
// valore restituito:                                                       |
//==========================================================================/

VOID _System ObjThreadProc(ULONG ulArg) {
   QMSG qmsg;
   ((PAPPDATA)ulArg)->hmqObj =
         WinCreateMsgQueue((((PAPPDATA)ulArg)->habObj = WinInitialize(0)), 0);
   // solo il thread principale riceve WM_QUIT dovuti allo shutdown
   WinCancelShutdown(((PAPPDATA)ulArg)->hmqObj, TRUE);
   // registra classe "TabWinTimeManagement"
   if (!WinRegisterClass(((PAPPDATA)ulArg)->habObj, OBJWINCLASS,
                         (PFNWP)ObjWinProc, 0L, 4L) ||
      (NULLHANDLE ==
       (((PAPPDATA)ulArg)->hObj = WinCreateWindow(HWND_OBJECT, OBJWINCLASS,
                                    NULL, 0, 0, 0, 0, 0, NULLHANDLE,
                                    HWND_BOTTOM, ID_OBJWIN,
                                    (PVOID)((PAPPDATA)ulArg), NULL))))
      TerminateObjProc(((PAPPDATA)ulArg));
   ((PAPPDATA)ulArg)->is.objOK = TRUE;
   DosPostEventSem(((PAPPDATA)ulArg)->hevObj);
   // message loop
   while (WinGetMsg(((PAPPDATA)ulArg)->habObj, &qmsg, NULLHANDLE, 0, 0) &&
                    !((PAPPDATA)ulArg)->is.closing)
      WinDispatchMsg(((PAPPDATA)ulArg)->habObj, &qmsg);
   ((PAPPDATA)ulArg)->is.objOK = 0;
   TerminateObjProc(((PAPPDATA)ulArg));
}


//==========================================================================\
// procedura object window                                                  |
// messaggi:                                                                |
// WM_CREATE                                                                |
//
//==========================================================================/

MRESULT EXPENTRY ObjWinProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
   switch (msg) {
      case WM_CREATE:
         WinSetWindowPtr(hwnd, 0L, (PVOID)mp1);
         break;
      case OBJM_START:
         StartProcessing(GetWinData(hwnd), (HWND)mp1, (HWND)mp2);
         break;
      case OBJM_LOAD:
         LoadBedPrf((PAPPDATA)mp2);
         break;
      case OBJM_SAVE:
         SaveBedPrf((PAPPDATA)mp2);
         break;
      case WM_PRGRSREADY:   // incremento barra progresso
         OWProgress(hwnd, (HWND)mp1);
         break;
      default:
         return WinDefWindowProc(hwnd, msg, mp1, mp2);
   } // end switch
   return (MRESULT)FALSE;
}


//==========================================================================\
// Inizia procedura find&replace calcolando totale bytes che saranno        |
// processati e spedendo msg start indietro alla finestra principale        |
// che creer… il dialogo di progresso e dar… inizio alla sostituzione       |
// delle stringhe                                                           |
// parametri:                                                               |
//  |
// valore restituito:                                                       |
//==========================================================================/

VOID StartProcessing(PAPPDATA pad, HWND hw1, HWND hw2) {
   INT c;
   PPROCESSTREEDATA pptd;
   PSZ pfname;
   HWND hlfile = WinWindowFromID(pad->hMain, LB_FILE);
   if (!(hw1 == pad->hMain && hw2 == pad->hMain)) return;
   if (!(pptd = malloc(sizeof(PROCESSTREEDATA)))) {
      Wprint(hw1, SZERR_ALLOC, 0);
      return;
   } /* endif */
   memset(pptd, 0, sizeof(PROCESSTREEDATA));
   pptd->flfind = FILE_ARCHIVED | FILE_SYSTEM | FILE_HIDDEN | FILE_READONLY;
   pptd->pdata = pad;
   pptd->pffile = countBytes;
   pad->cbtot = 0;
   pad->is.error = 0;
   pad->cbdone = 0;
   pad->is.working = 1;
   WinEnableWindow(pad->hMain, FALSE);
   // calcola totale bytes da processare
   for (c = wLboxQueryItemCount(hlfile); c;) {
      wLboxQueryItemText(hlfile, --c, pptd->achPath, MAXPATH);
      // se ricorsivo
      if (wLboxQueryItemHnd(hlfile, c)) {
         pfname = ioFNameFromPath(pptd->achPath);
         strcpy(pptd->achMask, pfname);
         strcpy(pfname, "*");
         pptd->flfind |= FILE_DIRECTORY;
      } else {
         *pptd->achMask = 0;
         pptd->flfind &= ~FILE_DIRECTORY;
      } /* endif */
      if (ioProcessTree(pptd) < 0) {
         pad->is.error = 1;
         break;
      } // end if
   } /* endfor */
   pad->step =  (pad->cbtot) / 1000;
   pad->incr = pad->step;
   free(pptd);
   WinPostMsg(pad->hMain, (pad->is.error? WM_OBJWERROR: WM_STARTPROCESSING),
              (MPARAM)pad->hObj, (MPARAM)pad->hObj);
}


//==========================================================================\
// termina il thread secondario liberando le risorse e notificando il       |
// thread principale tramite DosPostEventSem() se la terminazione Š dovuta  |
// al verificarsi di un errore                                              |
// parametri:                                                               |
// PAPPDATA pad: indirizzo struttura dati applicazione                      |
// valore restituito: VOID                                                  |
//==========================================================================/

VOID TerminateObjProc(PAPPDATA pad) {
   if (pad->hObj) WinDestroyWindow(pad->hObj);
   if (pad->hmqObj) WinDestroyMsgQueue(pad->hmqObj);
   if (pad->habObj) WinTerminate(pad->habObj);
   if (!pad->is.objOK) DosPostEventSem(pad->hevObj);
   pad->is.objOK = 0;
   DosExit(EXIT_THREAD, 0L);
}


//==========================================================================\
//  |
// parametri:                                                               |
// HWND hwnd : handle object window                                         |
// HWND hProgress : handle progress dialog                                  |
// valore restituito:                                                       |
//==========================================================================/

VOID OWProgress(HWND hwnd, HWND hProgress) {
   PAPPDATA pad = GetWinData(hwnd);
   INT i;
   PFNDREPLITEM pfri;
   ULONG cb, cstrings;
   PPROCESSTREEDATA pptd;
   PSZ pfname;
   HWND hlfile = WinWindowFromID(pad->hMain, LB_FILE);
   HWND hlstr = WinWindowFromID(pad->hMain, LB_STRINGS);
   // calcola massima dimensione nuovo file come massimo rapporto tra
   // vecchia e nuova stringa
   pad->hProgress = hProgress;
   cstrings = wLboxQueryItemCount(hlstr);
   // calcola quantit… memoria necessaria per lista sostituzioni
   cb = cbFndReplList(hlstr, cstrings);
   if (!(pad->plist = malloc(cb))) GOTOEND;
   memset(pad->plist, 0, cb);
   // riempie la struttura plist
   if (!getFndReplData(hlstr, pad, pad->plist, cstrings)) GOTOEND;
   // alloca memoria x dati file, mask e PROCESSTREEDATA struct
   pptd = malloc(sizeof(PROCESSTREEDATA));
   memset(pptd, 0, sizeof(PROCESSTREEDATA));
   pptd->flfind = FILE_ARCHIVED | FILE_SYSTEM | FILE_HIDDEN | FILE_READONLY;
   pptd->pdata = pad;
   pptd->pffile = FindNReplace;
   // loopa attraverso tutti i file in lista
   for (i = wLboxQueryItemCount(hlfile); i;) {
      wLboxQueryItemText(hlfile, --i, pptd->achPath, MAXPATH);
      // se ricorsivo
      if (wLboxQueryItemHnd(hlfile, i)) {
         pfname = ioFNameFromPath(pptd->achPath);
         strcpy(pptd->achMask, pfname);
         strcpy(pfname, "*");
         pptd->flfind |= FILE_DIRECTORY;
      } else {
         *pptd->achMask = 0;
         pptd->flfind &= ~FILE_DIRECTORY;
      } /* endif */
      if (ioProcessTree(pptd) < 0) {
         pad->is.error = 1;
         break;
      } // end if
   } /* endfor */
end:
   if (pad->is.error) {
      Wprint(hwnd, SZERR_PROCESSING, 0);
      if (pad->plist) {free(pad->plist);}
      pad->is.error = 0;
   } /* endif */
   if (pptd) {free(pptd);}
   _heapmin();
   WinPostMsg(hProgress, PRGSM_END, MPVOID, MPVOID);
   pad->is.working = 0;
   WinEnableWindow(pad->hMain, TRUE);
}


//==========================================================================\
// Calcola totale bytes da processare                                       |
//==========================================================================/

LONG countBytes(PPROCESSTREEDATA pptd) {
   ((PAPPDATA)pptd->pdata)->cbtot += pptd->fBuf.cbFile;
   return 1;
}


//==========================================================================\
// sostituisce le stringhe x tutti i file trovati                           |
// *** migliorare controllo spazio allocazione
//     controllo spazio libero x scrivere nuovi file
//==========================================================================/

LONG FindNReplace(PPROCESSTREEDATA pptd) {
   ULONG cb;
   INT i;
   PFNDREPLITEM pfri;
   PSZ pszfile = NULL;
   PSZ pfin = NULL;
   PSZ pin = NULL;
   PSZ pfout = NULL;
   PSZ pout = NULL;
   PAPPDATA pad = (PAPPDATA)pptd->pdata;
   PFILESTATUS3 pfs = NULL;
   BOOL modified = FALSE;
   // se il file ha dimensione 0 passa al successivo
   if (!(cb = (ULONG)(pad->rate * pptd->fBuf.cbFile))) goto end;
   if (!(pfs = malloc(sizeof(FILESTATUS3)))) GOTOEND;
   memset(pfs, 0, sizeof(FILESTATUS3));
   if (!(pfout = malloc(cb))) GOTOEND;
   if (!(pszfile = malloc(MAXPATH))) GOTOEND;
   strcpy(pszfile, pptd->achPath);
   strcpy(ioFNameFromPath(pszfile), pptd->fBuf.achName);
   WinSetWindowText(pad->hProgress, pptd->fBuf.achName);
   if (pptd->fBuf.attrFile & (FILE_SYSTEM | FILE_HIDDEN | FILE_READONLY)) {
      memcpy((PVOID)pfs, (PVOID)(((PSZ)&pptd->fBuf) + 4), sizeof(FILESTATUS3));
      pfs->attrFile = 0;
      if (DosSetPathInfo(pszfile, FIL_STANDARD, (PVOID)pfs,
                         sizeof(FILESTATUS3), 0)) GOTOEND;
      pad->is.ResetAttr = 1;
   } /* endif */
   if (!(pfin = ioF2psz(pszfile, NULL))) GOTOEND;
   pin = pfin;    // cursore in
   pout = pfout;   // cursore out
   cb = pptd->fBuf.cbFile;
   while (cb) {
      // cerca tra le coppie oldstring->newstring
      for (i = 0, pfri = pad->plist->fi; i < pad->plist->citems; ++i) {
         // trovata corrispondenza con stringa da sostituire
         if (!compare(pin, pfri->ach, pfri->cbfind, pfri->ins)) {
            pin += pfri->cbfind;
            memcpy(pout, pfri->ach + pfri->cbfind, pfri->cb - pfri->cbfind - 4);
            pout += pfri->cb - pfri->cbfind - 4;
            cb -= pfri->cbfind;
            pad->cbdone += pfri->cbfind;
            modified = TRUE;
            break;
         } else {
            // prova con stringa successiva
            pfri = (PFNDREPLITEM)(((PBYTE)pfri) + pfri->cb);
         } /* endif */
      } /* endfor */
      // corrispondenza non trovata
      if (i == pad->plist->citems) {
         *pout++ = *pin++;
         --cb;
         ++pad->cbdone;
      } // end if
      // eventualmente icrementa progressbar
      if (pad->cbdone > pad->incr) {
         WinPostMsg(pad->hProgress, PRGSM_INCREMENT,
                    (MPARAM)((ULONG)((pad->cbdone / pad->cbtot) * 1000)),
                    MPVOID);
         pad->incr += pad->step;
      } /* endif */
   } /* endwhile */
   if (modified) ioPsz2f(pszfile, pfout, pout - pfout);

end:
   if (pad->is.ResetAttr) {
      pad->is.ResetAttr = 0;
      if (pfs) {
         memcpy((PVOID)pfs, (PVOID)(((PSZ)&pptd->fBuf) + 4), sizeof(FILESTATUS3));
         DosSetPathInfo(pszfile, FIL_STANDARD, (PVOID)pfs,
                        sizeof(FILESTATUS3), 0);
      } /* endif */
   } /* endif */
   if (pfin) {free(pfin);}
   if (pfout) {free(pfout);}
   if (pszfile) {free(pszfile);}
   if (pfs) {free(pfs);}
   return (pad->is.error? -1: 1);
}


BOOL compare(PVOID p1, PVOID p2, ULONG cb, BOOL fl) {
   if (fl) return (BOOL)wpsznicmp(p1, p2, cb);
   return memcmp(p1, p2, cb);
}
