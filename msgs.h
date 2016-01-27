// msgs.h: define testo usato nei dialoghi

#ifndef MSGS_H_
   #define MSGS_H_

// DON'T MODIFY THESE LINES ***********************************************
#define SZ_NULL             ""
#define SZ_DEFAULT          "default"
// END OF DON'T MODIFY THESE LINES ****************************************

//#define LANGUAGE 39

#if (defined(ITALIAN)) // ------------------------------------------------

// INIT
#define SZERR_REGCTLS       "Errore registrazione controlli avanzati!"
#define SZ_LOADDLGFAILURE   "Errore apertura finestra principale!"

// finestra principale
#define SZ_MAINTITLE        "Batch Editor v. 0.46 beta"
#define SZ_FILE             "~File:"
#define SZ_FIND             "~Trova..."
#define SZ_ADD              "~Aggiungi"
#define SZ_REMOVE           "~Rimuovi"
#define SZ_REMOVEALL        "Rimuovi t~utto"
#define SZ_RECUR            "~Ricorsivamente nelle subdirectory"
#define SZ_STRINGS          "~Stringhe:"
#define SZ_F_IND            "~Trova:"
#define SZ_REPLACE          "~Sostituisci:"
#define SZ_NOCASE           "Ignora ~Maiuscolo/minuscolo"
#define SZ_ESCAPE           "Sequenza esca~pe"
#define SZ_LOAD             "~Carica"
#define SZ_SAVE             "Sa~lva"
#define SZ_START            "Ese~gui"
#define SZ_EXIT             "~Esci"
// file dialog
#define SZ_ADDTOLIST        "Aggiungi alla lista di ricerca..."
#define SZ_ADDEXT           "*.*"
// altri errori
#define SZERR_ALLOCMAIN     "Errore allocazione struttura dati applicazione"
#define SZERR_ALLOC         "Errore allocazione!"
#define SZERR_PROCESSING    "Errore elaborazione!"
#define SZERR_INVALIDFILE   "Formato file non valido!"
#define SZERR_SAVEPRF       "Impossibile salvare il profilo di editazione corrente!"
#define SZERR_LBXREADSTR    "Impossibile leggere il testo dalla lista!"
#define SZERR_INVESCSEQ     "Sequenza(e) di escape non valida!"
// dialogo progresso
#define SZ_SUBSTIT          "Sostituzione stringhe..."
#define SZ_PROGRESS         "Progresso: "
#define SZ_ELAPSED          "Tempo trascorso: "
#define SZ_STOP             "~Interrompi"
// caricamento profilo editazione
#define SZ_BEPEXT           "*.bep"
#define SZ_BEPTITLE         "Carica profilo editazione"
#define SZ_BEPBTN           "~Carica"
// dialogo salvataggio profilo editazione
#define SZ_SVTITLE          "Salva profilo editazione"
#define SZ_SVBTN            "~Salva"
#define SZ_ASKOVERWRT       "Sovrascrivi il file preesistente?"

#elif (defined(FRENCH)) // --------------------------------------------------

// INIT
#define SZERR_REGCTLS       "Enregistrement impossible des contrìles Çtendus!"
#define SZ_LOADDLGFAILURE   "Ouverture impossible de la fenàtre principale!"

// finestra principale
#define SZ_MAINTITLE        "Batch Editor v0.46 bàta"
#define SZ_FILE             "~Fichiers :"
#define SZ_FIND             "~Rechercher..."
#define SZ_ADD              "~Ajouter"
#define SZ_REMOVE           "~Enlever"
#define SZ_REMOVEALL        "Enlever to~ut"
#define SZ_RECUR            "~Chercher dans les sous-rÇpertoires"
#define SZ_STRINGS          "~Chaånes :"
#define SZ_F_IND            "~Rechercher :"
#define SZ_REPLACE          "~Remplacer par :"
#define SZ_NOCASE           "~Ignorer la casse"
#define SZ_ESCAPE           "Caractäres d'Çc~happement"
#define SZ_LOAD             "Char~ger"
#define SZ_SAVE             "E~nregistrer"
#define SZ_START            "~Lancer"
#define SZ_EXIT             "~Quitter"
// file dialog
#define SZ_ADDTOLIST        "Ajouter Ö la liste..."
#define SZ_ADDEXT           "*.*"
// altri errori
#define SZERR_ALLOCMAIN     "Erreur d'allocation dans la structure des donnÇes de l'application!"
#define SZERR_ALLOC         "Erreur d'allocation!"
#define SZERR_PROCESSING    "Erreur de traitement!"
#define SZERR_INVALIDFILE   "Format de fichier non valide!"
#define SZERR_SAVEPRF       "êchec de la sauvegarde du profil d'Çdition en cours!"
#define SZERR_LBXREADSTR    "êchec de lecture de la chaåne de texte dans la liste!"
#define SZERR_INVESCSEQ     "SÇquence(s) d'Çchappement non valide!"
// dialogo progresso
#define SZ_SUBSTIT          "Substitution des chaånes..."
#define SZ_PROGRESS         "Progression : "
#define SZ_ELAPSED          "Temps ÇcoulÇ : "
#define SZ_STOP             "~Stop"
// caricamento profilo editazione
#define SZ_BEPEXT           "*.bep"
#define SZ_BEPTITLE         "Charger un profile d'Çdition"
#define SZ_BEPBTN           "~Charger"
// dialogo salvataggio profilo editazione
#define SZ_SVTITLE          "Enregistrer le profile d'Çdition"
#define SZ_SVBTN            "~Enregistrer"
#define SZ_ASKOVERWRT       "Remplacer le fichier existant?"


#else  // international (English) ---------------------------------------


// INIT
#define SZERR_REGCTLS       "Cannot register enhanced controls!"
#define SZ_LOADDLGFAILURE   "Cannot open the main window!"

// main window
#define SZ_MAINTITLE        "Batch Editor v. 0.46 beta"
#define SZ_FILE             "~File:"
#define SZ_FIND             "Fi~nd..."
#define SZ_ADD              "~Add"
#define SZ_REMOVE           "~Remove"
#define SZ_REMOVEALL        "Remove a~ll"
#define SZ_RECUR            "Re~cur subdirectories"
#define SZ_STRINGS          "~Strings:"
#define SZ_F_IND            "F~ind:"
#define SZ_REPLACE          "Re~place:"
#define SZ_NOCASE           "Case ~insensitive"
#define SZ_ESCAPE           "~Escape chars"
#define SZ_LOAD             "~Load"
#define SZ_SAVE             "Sa~ve"
#define SZ_START            "S~tart"
#define SZ_EXIT             "E~xit"
// file dialog
#define SZ_ADDTOLIST        "Add to search list..."
#define SZ_ADDEXT           "*.*"
// error messages
#define SZERR_ALLOCMAIN     "Application data structure allocation error"
#define SZERR_ALLOC         "Allocation error!"
#define SZERR_PROCESSING    "Processing error!"
#define SZERR_INVALIDFILE   "Invalid file format!"
#define SZERR_SAVEPRF       "Failed to save the current edit profile"
#define SZERR_LBXREADSTR    "Failed to read the item text from the listbox"
#define SZERR_INVESCSEQ     "Invalid escape sequence(s)!"
// progress dialog
#define SZ_SUBSTIT          "Substituting strings..."
#define SZ_PROGRESS         "Progress: "
#define SZ_ELAPSED          "Elapsed time: "
#define SZ_STOP             "~Stop"
// edit profile load
#define SZ_BEPEXT           "*.bep"
#define SZ_BEPTITLE         "Load editing profile"
#define SZ_BEPBTN           "~Load"
// edit profile save
#define SZ_SVTITLE          "Save editing profile"
#define SZ_SVBTN            "~Save"
#define SZ_ASKOVERWRT       "Overwrite existing file?"
#endif

#endif
