#ifndef MAIN_H_
   #define MAIN_H_

// interfaccia
#define DLG_MAIN                 32
#define TXT_FILE                 100
#define LB_FILE                  101
#define EF_FILE                  102
#define BTN_FIND                 103
#define BTN_ADD0                 104
#define BTN_REM0                 105
#define BTN_REMALL               106
#define CHK_RECUR                107
#define TXT_STRINGS              108
#define LB_STRINGS               109
#define BTN_FINDCLP              110
#define EF_STRFIND               111
#define BTN_REPLACECLP           112
#define EF_STRREPL               113
#define BTN_ADD1                 114
#define BTN_REM1                 115
#define CHK_NOCASE               116
#define CHK_ESCAPE               117
#define BTN_LOAD                 118
#define BTN_SAVE                 119
#define BTN_START                120
#define BTN_EXIT                 121
#define BTN_FOCUSREPL            122
#define BTN_MOVEUP               123
#define BTN_MOVEDOWN             124

#define ID_OBJWIN     400      // object window

// messaggi
#define OBJM_START    WM_USER + 765
#define WM_STARTPROCESSING OBJM_START
#define WM_OBJWERROR  WM_STARTPROCESSING + 1
#define WM_DEFERFOCUS WM_STARTPROCESSING + 2
#define OBJM_LOAD     WM_STARTPROCESSING + 3
#define OBJM_SAVE     WM_STARTPROCESSING + 4

// altri
#define MAXPATH       260
#define MAXSTR        512
#define MAXSTRITEM   1032

#define GOTOEND      {pad->is.error = 1; goto end;}
#define GetWinData(hwnd)      (PAPPDATA)WinQueryWindowPtr((hwnd), 0L)

// VECCHIA struttura stringa sostituzione
typedef struct {
   UINT cb     : 16;    // dimensione totale struttura
   UINT cbfind : 15;    // dimensione stringa find
   UINT ins    :  1;    // case insensitivo
   CHAR ach[1];         // stringa find+replace non terminate
} OLDFRITEM, * POLDFRITEM;


// struttura stringa sostituzione
typedef struct {
   UINT cb     : 15;    // dimensione totale struttura
   UINT ins    :  1;    // case insensitivo
   UINT cbfind : 15;    // dimensione stringa find
   UINT esc    :  1;    // interpreta caratteri escape
   CHAR ach[1];         // stringa find+replace non terminate
} FNDREPLITEM, * PFNDREPLITEM;

typedef union {
   ULONG ul;
   struct {
      UINT cb     : 15;    // dimensione totale struttura
      UINT ins    :  1;    // case insensitivo
      UINT cbfind : 15;    // dimensione stringa find
      UINT esc    :  1;    // interpreta caratteri escape
   } fri;
} FRIDATA, * PFRIDATA;

typedef struct {
   ULONG citems;
   FNDREPLITEM fi[1];
} FNDREPLLIST, * PFNDREPLLIST;

// identificatore file
#define BEDPRFID               0x00504542  // vecchio formato file
#define BEDPRFID1              0x01504542  // nuovo formato versione 0.4
#define BEDPRFID2              0x54504542  // BEPT formato file testo

// struttura file di configurazione profilo di sostituzioni:
typedef struct {
   ULONG sign;      // BEDPRFID
   ULONG offset;    // offset alla lista di file dall'inizio della struttura
   FNDREPLLIST frl; // lista di sostituzioni
   PSZ filelist;    // lista di file
} BEDFILE, * PBEDFILE;

// stato applicazione
typedef struct {
   UINT bever        :  8;          // versione batch editor
   UINT osver        :  4;
   UINT objOK        :  1;          // stato object window
   UINT working      :  1;          // thread find&replace attivo
   UINT ending       :  1;          // chiusura in corso
   UINT closing      :  1;          // programma Š in fase di chiusura
   UINT ResetAttr    :  1;          // resettare attributi file
   UINT error        :  1;          // flag errore
   UINT loadarg      :  1;          // flag che indica presenza profilo da
                                    // caricare durante inizializzazione
} APPSTATUS, * PAPPSTATUS;

typedef struct {
   APPSTATUS is;               // stato applicazione
   HAB hab;
   HMQ hmq;
   // dati thread object window
   TID tidObj;                 // thread ID della object window
   HEV hevObj;                 // handle eventsem avviamento thread
   HAB habObj;                 // hab obj win
   HMQ hmqObj;                 // hmq obj win
   QMSG qmsgObj;               //
   HWND hObj;                  // handle della object window
   HWND hMain;                 // handle finestra principale
   HWND hProgress;             // handle progress dialog
   ULONG cbfile;               // dimensione file
   double cbtot;               // tot bytes da elaborare
   double cbdone;              // tot bytes elaborati
   double step;                // passo incremento progress bar in millesimi
   double incr;                // incremento corrente progress bar (millesimi)
   ULONG rate;                 // rapporto tra vecchio file e max dimens nuovo
   PFNDREPLLIST plist;         // dati sostituzioni
   CHAR achpath[260];          // path caricamento file da editare
   CHAR achload[260];          // path caricamento profili da caricare
} APPDATA, * PAPPDATA;


// funzioni

// _api.c
ULONG sysinfo(ULONG ulid);

// prefmain.c

// object.c
BOOL StartObjectThread(PAPPDATA pad);

// util.c
BOOL enableBtns(HWND hwnd);
BOOL resetStrCtls(HWND hwnd, BOOL fl, BOOL fcase, BOOL fesc);
BOOL LoadBedPrf(PAPPDATA pad);
BOOL SaveBedPrf(PAPPDATA pad);
BOOL WsetWaitPtr(VOID);
PFILEDLG fileDlg(HWND hwnd, PSZ pszFile, PSZ pszext, PSZ pszTitle, PSZ pszBtn,
                 ULONG fl);
ULONG cbFndReplList(HWND hwnd, ULONG citems);
ULONG getFndReplData(HWND hwnd, PAPPDATA pad, PFNDREPLLIST pfrl, ULONG citems);
VOID getFileListData(HWND hlist, PSZ pfiles, ULONG citems);
MRESULT EXPENTRY saveFileProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
BOOL compilebinfr(PSZ psz, PFRIDATA pfd, PFNDREPLITEM pfri);
BOOL chkEscSeq(HWND hlist, ULONG item);
PSZ escseq2ch(PSZ pszin, PSZ pszout);
LONG psz2hexch(PSZ psz);
VOID ab2psz(PSZ pszout, PFNDREPLITEM pfri, PFRIDATA pfd);
VOID moveItem(HWND hwnd, INT offset);
#endif
