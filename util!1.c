//==========================================================================\
// util.c: funzioni utilit… blocco x impostazioni tabwin                |
// BOOL enableBtns(HWND hwnd);
// BOOL resetStrCtls(HWND hwnd, BOOL fl, BOOL fcase, BOOL fesc);
// BOOL LoadBedPrf(PAPPDATA pad);
// BOOL SaveBedPrf(PAPPDATA pad);
// BOOL WsetWaitPtr(VOID);
// PFILEDLG fileDlg(HWND hwnd, PSZ pszFile, PSZ pszext, PSZ pszTitle, PSZ pszBtn,
//                  ULONG fl);
// ULONG cbFndReplList(HWND hwnd, ULONG citems);
// ULONG getFndReplData(HWND hwnd, PAPPDATA pad, PFNDREPLLIST pfrl, ULONG citems);
// VOID getFileListData(HWND hlist, PSZ pfiles, ULONG citems);
// MRESULT EXPENTRY saveFileProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
// BOOL compilebinfr(PSZ psz, PFRIDATA pfd, PFNDREPLITEM pfri);
// BOOL chkEscSeq(HWND hlist, ULONG item);
// PSZ escseq2ch(PSZ pszin, PSZ pszout);
// LONG psz2hexch(PSZ psz);
// VOID ab2psz(PSZ pszout, PFNDREPLITEM pfri, PFRIDATA pfd);
//==========================================================================/

#pragma strings(readonly)

#define INCL_WIN                                                  
#define INCL_DOSMISC
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


//==========================================================================\
// abilita/disabilita bottone start e save secondo stato listbox            |
//==========================================================================/

BOOL enableBtns(HWND hwnd) {
   ULONG c1, c2;
   BOOL b1, b2;
   c1 = DlgLboxQueryItemCount(hwnd, LB_FILE);
   b1 = c1 && (DlgLboxQuerySelectedItem(hwnd, LB_FILE) >= 0);
   c2 = DlgLboxQueryItemCount(hwnd, LB_STRINGS);
   b2 = c2 && (DlgLboxQuerySelectedItem(hwnd, LB_STRINGS) >= 0);
   return (WinEnableControl(hwnd, BTN_REM0, b1) &&
           WinEnableControl(hwnd, BTN_REM1, b2) &&
           WinEnableControl(hwnd, BTN_START, c1 && c2) &&
           WinEnableControl(hwnd, BTN_SAVE, c1 || c2) &&
           WinEnableControl(hwnd, BTN_REMALL, c2));
}


//==========================================================================\
// abilita/disabilita controlli e spunta o meno check box opzioni stringhe  |
// secondo la selezione della stringa find/replace                          |
//==========================================================================/

BOOL resetStrCtls(HWND hwnd, BOOL fl, BOOL fcase, BOOL fesc) {
   BOOL rc = WinEnableControl(hwnd, CHK_NOCASE, fl) &&
             WinEnableControl(hwnd, CHK_ESCAPE, fl) &&
             WinEnableControl(hwnd, BTN_REM1, fl);
   WinCheckButton(hwnd, CHK_NOCASE, (fl && fcase? 1: 0));
   WinCheckButton(hwnd, CHK_ESCAPE, (fl && fesc? 1: 0));
   if (fl) WsetDefBtn(hwnd, BTN_REM1);
   return rc;
}


//==========================================================================\
// - legge il file in memoria
// - svuota liste e entry field del contenuto corrente
// - riempie liste con contenuto profilo editazione
// - se profilo editazione contiene sia specifiche file che elenco
//   sostituzioni setta Start come bottone di default
// - se profilo editazione contiene solo elenco file da il focus a
//   entryfield find, altrimenti a entryfield file
//==========================================================================/

BOOL LoadBedPrf(PAPPDATA pad) {
   PBEDFILE pbf;
   BOOL fl;
   SHORT item;
   PSZ pszerr = NULL;
   BOOL rc = FALSE;
   HWND hlfile = WinWindowFromID(pad->hMain, LB_FILE);
   HWND hlstr = WinWindowFromID(pad->hMain, LB_STRINGS);
   pad->is.working = 1;
   WinEnableWindow(pad->hMain, FALSE);
   if (!(pbf = (PBEDFILE)ioF2psz(pad->achload, NULL)))
      {pszerr = SZERR_ALLOC; goto end;}
   if (*((PULONG)pbf) != BEDPRFID) {pszerr = SZERR_INVALIDFILE; goto end;}
   // resetta controlli listbox e entryfield ------------------------------
   wLboxDelAll(hlfile);
   WinSetDlgItemText(pad->hMain, EF_FILE, SZ_NULL);
   wLboxDelAll(hlstr); 
   WinSetDlgItemText(pad->hMain, EF_STRFIND, SZ_NULL);
   WinSetDlgItemText(pad->hMain, EF_STRREPL, SZ_NULL);
   // processa lista file -------------------------------------------------
   if (pbf->offset) {     // se presente lista file
      PSZ pflist, pcur;
      pflist = (PBYTE)pbf + pbf->offset;
      for (pcur = pflist; *pcur; pcur += strlen(pcur) + 1) {
         fl = *pcur++ == 'R';
         item = wLboxInsItem(hlfile, LIT_SORTASCENDING, pcur);
         if (fl && item >= 0) wLboxSetItemHnd(hlfile, item, 1);
      } /* endfor */
      // aggiorna percorso ricerca file secondo percorso primo file
      strcpy(pad->achpath, pflist + 1);
      pcur = ioFNameFromPath(pad->achpath);
      *pcur = 0;
   } /* endif */
   // processa lista stringhe ---------------------------------------------
   if (pbf->frl.citems) {
      PSZ pszitem = malloc(MAXSTRITEM);
      INT i;
      PFNDREPLITEM pfri;
      FRIDATA fd;
      if (!pszitem) {pszerr = SZERR_ALLOC; goto end;}
      for (i = 0, pfri = pbf->frl.fi; i < pbf->frl.citems; ++i) {
         // se stringa contenente caratteri non stampabili la converte
         // usando le opportune sequenze di escape
         if (pfri->esc) {
            ab2psz(pszitem, pfri, &fd); 
         } else {
            memcpy(pszitem, pfri->ach, pfri->cbfind);
            strcpy(pszitem + pfri->cbfind, " -> ");
            memcpy(pszitem + pfri->cbfind + 4, pfri->ach + pfri->cbfind,
                   pfri->cb - pfri->cbfind - 4);
            *(pszitem + pfri->cb) = 0;
            fd.fri.cbfind = pfri->cbfind;
         } /* endif */
         fd.fri.ins = pfri->ins;
         fd.fri.esc = pfri->esc;
         item = wLboxInsItem(hlstr, LIT_END, pszitem);
         wLboxSetItemHnd(hlstr, item, fd.ul);
         pfri = (PFNDREPLITEM)(((PBYTE)pfri) + pfri->cb);
      } /* endfor */
      free(pszitem);
   } /* endif */
   if (pbf->offset && pbf->frl.citems) {
      WsetDefBtn(pad->hMain, BTN_START);
   } else if (pbf->offset) {
      WinPostMsg(pad->hMain, WM_DEFERFOCUS, (MPARAM)EF_FILE, MPVOID);
   } else {
      WinPostMsg(pad->hMain, WM_DEFERFOCUS, (MPARAM)EF_STRFIND, MPVOID);
   } /* endif */
   rc = TRUE;
end:
   if (pszerr) Wprint(pad->hMain, pszerr, 0);
   if (pbf) {free(pbf);}
   enableBtns(pad->hMain);
   WinEnableWindow(pad->hMain, TRUE);
   pad->is.working = 0;
   return TRUE;
}


//==========================================================================\
// - ricava dimensioni allocazione per scrittura file
// - alloca la memoria necessaria
// - legge il contenuto delle listbox scrivendo la struttura del file
// - apre il file di dialogo per selezionare il nome di salvataggio file
// - scrive il file
//==========================================================================/

BOOL SaveBedPrf(PAPPDATA pad) {
   PBEDFILE pbf;
   ULONG i, cbfndrepl, cbfiles, cstrings, cfiles;
   PSZ pszerr = NULL;
   PSZ pflist;
   HWND hlfile = WinWindowFromID(pad->hMain, LB_FILE);
   HWND hlstr = WinWindowFromID(pad->hMain, LB_STRINGS);
   pad->is.working = 1;
   WinEnableWindow(pad->hMain, FALSE);
   // calcola allocazione approssimativa per la lista di sostituzioni
   cstrings = wLboxQueryItemCount(hlstr);
   cbfndrepl = cbFndReplList(hlstr, cstrings);
   // calcola dimensione lista file (aggiunge 1 x flag ricorsivit… e 1 per term
   cfiles = wLboxQueryItemCount(hlfile);
   for (i = cbfiles = 0; i < cfiles; ++i)
      cbfiles += WLboxQueryItemTextLength(hlfile, i) + 2;
   cbfiles++;  // la lista viene terminata con un doppio zero
   // alloca la memoria necessaria
   if (!(pbf = malloc(cbfndrepl + cbfiles + 8)))
      {pszerr = SZERR_ALLOC; goto end;}
   pbf->sign = BEDPRFID;      // setta ID file
   // inizializza lista di sostituzioni
   if (!(cbfndrepl = getFndReplData(hlstr, pad, &pbf->frl, cstrings)))
      {pszerr = SZERR_ALLOC; goto end;}
   pbf->offset = 8 + cbfndrepl;
   // inizializza lista file
   getFileListData(hlfile, (PSZ)((PBYTE)pbf + pbf->offset), cfiles);
   if (!ioPsz2f(pad->achload, (PSZ)pbf, cbfndrepl + cbfiles + 8))
      {pszerr = SZERR_SAVEPRF; goto end;}
end:
   if (pszerr) Wprint(pad->hMain, pszerr, 0);
   if (pbf) {free(pbf);}
   WinEnableWindow(pad->hMain, TRUE);
   pad->is.working = 0;
   return TRUE;
}


//==========================================================================\
// setta il pointer di attesa quando il mouse si muove sulla finestra       |
//==========================================================================/

BOOL WsetWaitPtr(VOID) {
   HPOINTER hptr = WinQuerySysPointer(HWND_DESKTOP, SPTR_WAIT, FALSE);
   return (hptr? WinSetPointer(HWND_DESKTOP, hptr): FALSE);
}

//==========================================================================\
// apre il file di dialogo restituendo un puntatore al nome del file        |
// selezionato                                                              |
// parametri:                                                               |
// HWND hwnd: handle owner window                                           |
// PSZ pszFile: in/out specifiche file da ricercare/nome file trovato       |
// ULONG pszTitle: titolo dialogo                                           |
// ULONG pszBtn: titolo bottone                                             |
// ULONG fl: 0 = caricamento profilo editazione, 1 = ricerca file da        |
//               editare, 2 = salvataggio profilo editazione                |
// valore restituito:                                                       |
// PFILEDLG struttura file dialog allocata o NULL se errore o si preme cancel
//==========================================================================/

PFILEDLG fileDlg(HWND hwnd, PSZ pszFile, PSZ pszext, PSZ pszTitle, PSZ pszBtn,
                 ULONG fl) {
   PFILEDLG pfdlg = NULL;
   PSZ pszret = NULL;
   ULONG afl[] = {FDS_OPEN_DIALOG,
                  FDS_OPEN_DIALOG | FDS_MULTIPLESEL,
                  FDS_SAVEAS_DIALOG | FDS_ENABLEFILELB};
   if (!(pfdlg = (PFILEDLG)malloc(sizeof(FILEDLG)))) {
      Wprint(hwnd, SZERR_ALLOC, 0);
      return NULL;
   } /* endif */
   memset((PVOID)pfdlg, 0, sizeof(FILEDLG));
   pfdlg->pszTitle = pszTitle;
   pfdlg->pszOKButton = pszBtn;
   pfdlg->cbSize = sizeof(FILEDLG);
   pfdlg->pfnDlgProc = fl == 2? saveFileProc: NULL;
   pfdlg->fl = FDS_CENTER | afl[fl];
   strcpy(pfdlg->szFullFile, pszFile);
   strcpy(ioFNameFromPath(pfdlg->szFullFile), pszext);
   if (WinFileDlg(HWND_DESKTOP, hwnd, pfdlg) &&
       pfdlg->lReturn == DID_OK) {
      strcpy(pszFile, pfdlg->szFullFile);
      if (fl != 1) {   // se non Š file dialog multiplo libera allocazione ora
         free(pfdlg);
         return (PFILEDLG)1;
      } /* endif */
      return pfdlg;
   } // end if
   free(pfdlg);
   return NULL;
}


//==========================================================================\
// calcola quantit… memoria necessaria per lista sostituzioni               |
//==========================================================================/

ULONG cbFndReplList(HWND hwnd, ULONG citems) {
   ULONG i, cb;
   if (!citems) return sizeof(FNDREPLLIST);
   for (i = citems, cb = 0; i;)
      // lunghezza totale oldstring->newstring
      cb += WLboxQueryItemTextLength(hwnd, --i);
   return (cb + 4);
}


//==========================================================================\
// riempie la struttura della lista delle sostituzioni con il contenuto
// della listbox
// Parametri:
// HWND hwnd : handle listbox stringhe
// valore restituito:
// dimensione totale dati find/replace - 0 = errore
//==========================================================================/

ULONG getFndReplData(HWND hwnd, PAPPDATA pad, PFNDREPLLIST pfrl, ULONG citems) {
   ULONG rc = 0;
   PFNDREPLITEM pfri;
   PSZ pszitem;
   INT i, rate;
   ULONG hitem;
   FRIDATA fd;
   rate = pad->rate = 1;
   pfrl->citems = citems;
   if (!(pszitem = malloc(MAXSTRITEM))) return FALSE;
   for (pfri = pfrl->fi, i = 0; i < citems; ++i) {
      fd.ul = wLboxQueryItemHnd(hwnd, i);
      fd.fri.cb = wLboxQueryItemText(hwnd, i, pszitem, MAXSTRITEM);
      // se la stringa find/replace contiene caratteri di escape la converte
      if (fd.fri.esc) {
         if (!compilebinfr(pszitem,  &fd, pfri)) goto end;
      } else {
         pfri->cbfind = fd.fri.cbfind;
         pfri->cb = fd.fri.cb;
         memcpy(pfri->ach, pszitem, pfri->cbfind);
         memcpy(pfri->ach + pfri->cbfind, pszitem + pfri->cbfind + 4,
                pfri->cb - pfri->cbfind - 4);
      } /* endif */
      pfri->ins = fd.fri.ins;
      pfri->esc = fd.fri.esc;
      rate = (pfri->cb - pfri->cbfind - 4) / pfri->cbfind +
             (((pfri->cb - pfri->cbfind - 4) % pfri->cbfind)? 1: 0);
      if (rate > pad->rate) pad->rate = rate;
      pfri = (PFNDREPLITEM)((PBYTE)pfri + pfri->cb); 
   } /* endfor */
   rc = (PBYTE)pfri - (PBYTE)pfrl;
end:
   free(pszitem);
   return rc;
}


//==========================================================================\
// legge il contenuto della listbox file e costruisce una sequenza di       |
// stringhe terminate da 0 e inizianti con uno spazio o con 'R' se il       |
// file ha la flag di recorsivit…. Alla fine della lista aggiunge un        |
// ulteriore 0 per indicare un nome di file vuoto                           |
//==========================================================================/

VOID getFileListData(HWND hlist, PSZ pfiles, ULONG citems) {
   INT i;
   for (i = 0; i < citems; ++i) {
      *pfiles = (wLboxQueryItemHnd(hlist, i))? 'R': ' ';
      pfiles += wLboxQueryItemText(hlist, i, pfiles + 1, MAXPATH) + 2;
   } /* endfor */
   *pfiles = 0;
}


//==========================================================================\
//==========================================================================/

MRESULT EXPENTRY saveFileProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
   if (msg == FDM_VALIDATE) {
      if (ioFExists((PSZ)mp1, NULL))
         return (MPARAM)Wprint(hwnd, SZ_ASKOVERWRT, PMPRNT_QUERY);
   } /* endif */
   return WinDefFileDlgProc(hwnd, msg, mp1, mp2);
}


//==========================================================================\
// converte la stringa find/replace contenente caratteri di escape in
// una valida struttura FNDREPLITEM
// Parametri:
// PSZ pszin: buffer contenente stringa da convertire
// PFRIDATA pfd: dati relativi a contenuto buffer psz 
// PFNDREPLITEM pfri: buffer scrittura dati convertiti
// valore restituito:
// BOOL : TRUE/FALSE = successo/errore (presenza caratteri di escape non validi)
//==========================================================================/

BOOL compilebinfr(PSZ psz, PFRIDATA pfd, PFNDREPLITEM pfri) {
   PSZ pszin;      // puntatore in psz
   PSZ pszout;     // puntatore in pfri->ach
   pszin[pfd->fri.cbfind] = 0;
   // converte stringa find
   for (pszin == psz, pszout = pfri->ach; TRUE; ++pszin, ++pszout) {
      if (!(*pszin || pfri->cbfind)) {  // termine stringa find
         pszin += 4;
         pfri->cbfind = pszout - pfri->ach;
      } else {                          // termine stringa replace
         pfri->cb = pszout - pfri->ach + 4;
         break;
      } /* endif */
      // se carattere di escape
      if (*pszin == '\\') 
         if (!(pszin = escseq2ch(++pszin, pszout))) return FALSE;
      else
         *pszout = *pszin;
   } /* endfor */
   return TRUE;
}


//==========================================================================\
// controlla che le sequenze di escape presenti nella stringa contenuta     |
// nell'item item della listbox hlist siano corrette                        |
//==========================================================================/

BOOL chkEscSeq(HWND hlist, ULONG item) {
   BOOL rc = FALSE;
   PSZ psz = malloc(MAXSTRITEM);
   PSZ pszcur;
   // controlla allocazione per lettura testo item lista stringhe
   if (!psz) {
      Wprint(HWND_DESKTOP, SZERR_ALLOC, PMPRNT_ERROR);
      return FALSE;
   } /* endif */
   // legge testo item corrente
   if (!wLboxQueryItemText(hlist, item, psz, MAXSTRITEM)) {
      Wprint(HWND_DESKTOP, SZERR_LBXREADSTR, PMPRNT_ERROR);
      goto end;
   } /* endif */
   // controlla correttezza sequenze di escape
   for (pszcur = psz; *pszcur; ++pszcur) {
      if (*pszcur == '\\') {
         if (!(pszcur = escseq2ch(++pszcur, NULL))) {
            Wprint(HWND_DESKTOP, SZERR_INVESCSEQ, PMPRNT_ERROR);
            goto end;
         } /* endif */
      } /* endif */
   } /* endfor */
   rc = TRUE;
end:
   free(psz);
   return rc;
}


//==========================================================================\
// converte i(l) caratteri(e) successivo ad un carattere di escape nel
// corrispondente valore
// se i caratteri non sono validi restituisce -1
// parametri:
// PSZ pszin: stringa contenente caratteri esadecimali
// PSZ pszout: puntatore carattere risultato conversione
// valore restituito:                                                       |
// PSZ puntatore successivo carattere in pszin / NULL se errore
//==========================================================================/

PSZ escseq2ch(PSZ pszin, PSZ pszout) {
   LONG hexval;
   switch (*pszin) {
      case 'a': if (pszout) *pszout = '\a'; return pszin;
      case 'b': if (pszout) *pszout = '\b'; return pszin;
      case 'f': if (pszout) *pszout = '\f'; return pszin;
      case 'n': if (pszout) *pszout = '\n'; return pszin;
      case 'r': if (pszout) *pszout = '\r'; return pszin;
      case 't': if (pszout) *pszout = '\t'; return pszin;
      case 'v': if (pszout) *pszout = '\v'; return pszin;
      case '\'': 
      case '\"':
      case '?': 
      case '\\': if (pszout) *pszout = *pszin; return pszin;
      case 'x':
         if ((hexval = psz2hexch(++pszin)) >= 0)
            if (pszout) *pszout = hexval;
         else
            return NULL;
         return ++pszin + 2;
      default: return NULL;
   } /* endswitch */
}


//==========================================================================\
// converte una coppia di caratteri esadecimali in un numero
// se i caratteri non sono validi restituisce -1
// parametri:
// PSZ psz: stringa contenente caratteri esadecimali
// valore restituito:                                                       |
// ULONG valore caratteri esadecimali
//==========================================================================/

LONG psz2hexch(PSZ psz) {
   LONG l = 0;
   INT i;
   for (i = 4; i >= 0; ++psz, i -= 4) {
      if (*psz >= '0' && *psz <= '9') {
         l += (*psz - '0') << i;
      } else if (*psz >= 'A' && *psz <= 'F') {
         l += (*psz - 'A' + 10) << i;
      } else if (*psz >= 'a' && *psz <= 'f') {
         l += (*psz - 'a' + 10) << i;
      } else {
         return -1;
      } /* endif */
   } /* endfor */
   return l;
}


//==========================================================================\
// converte un'array di byte contenente caratteri non stampabili in una
// stringa contenente le opportune sequenze di escape
//==========================================================================/

VOID ab2psz(PSZ pszout, PFNDREPLITEM pfri, PFRIDATA pfd) {
   PSZ pszend = pfri->ach + pfri->cbfind;
   PSZ pszstart = pszout;
   PSZ psz;
   for (psz = pfri->ach, pfd->fri.cbfind = 0; ; ++psz, ++pszout) {
      // se ha raggiunto fine stringa find o fine stringa find/replace
      if (psz == pszend) {
         if (pfd->fri.cbfind) break;
         pfd->fri.cbfind = pszout - pszstart;
         memcpy(pszout, " -> ", 4);
         pszout += 4;
         pszend = pfri->ach + pfri->cb - 4;
      } /* endif */
      if (*psz == '\\') {
         memcpy(pszout, "\\\\", 2);
         ++pszout;
      } else if (*psz >= ' ' && *psz < '\xff') {
         *pszout = *psz;
      } else {
         ULONG tmp;
         switch (*psz) {
            // sequenze di escape classiche
            case '\a': memcpy(pszout, "\\a", 2); ++pszout; break;
            case '\b': memcpy(pszout, "\\b", 2); ++pszout; break;
            case '\f': memcpy(pszout, "\\f", 2); ++pszout; break;
            case '\n': memcpy(pszout, "\\n", 2); ++pszout; break;
            case '\r': memcpy(pszout, "\\r", 2); ++pszout; break;
            case '\t': memcpy(pszout, "\\t", 2); ++pszout; break;
            case '\v': memcpy(pszout, "\\v", 2); ++pszout; break;
            // sequenze di escape espresse da stringa esadecimale
            default:
               *pszout++ = '\\';
               *pszout++ = 'x';
               tmp = *psz >> 4;
               *pszout++ = tmp < 10? tmp + '0': tmp + 'a' - 10;
               tmp = *psz & 0xf;
               *pszout = tmp < 10? tmp + '0': tmp + 'a' - 10;
               break;
         } /* endswitch */
      } /* endif */
   } /* endfor */
   *pszout = 0;    // termina la stringa
}
