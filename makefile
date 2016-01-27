# To add new extensions to the inference rules uncomment and redefine this:
#.SUFFIXES:
#
#.SUFFIXES: \
#    .C .obj .rc .res

# compiler, linker, resource compiler, resource binder MACRO

CC = icc.exe
CL = ilink.exe
RC = rc.exe
RB = rc.exe

# compiler and linker flags

# Debug version
!ifdef DEBUG

CFLAGS = /Ss /Ti /Rn /G5 /C
LFLAGS = /DE /E:2 /PM:PM /PACKD /A:4 /OPTF /NOL /M /L

!else
# RELEASE version

#CFLAGS = /Ss /O /Oc /Ol /Rn /G5 /C
CFLAGS = /Ss /O /Oc /Rn /G5 /C
LFLAGS = /E:2 /PM:PM /PACKD /A:4 /OPTF /NOL /M /L

!endif

RCFLAGS = -r
RBFLAGS = -x2
# language dependent compilation
!ifdef FRENCH
CFLAGS = $(CFLAGS) /DFRENCH
RCFLAGS = $(RCFLAGS) -D FRENCH
!endif

!ifdef ITALIAN
CFLAGS = $(CFLAGS) /DITALIAN
RCFLAGS = $(RCFLAGS) -D ITALIAN
!endif


.rc.res:
   $(RC) $(RCFLAGS) $<

.C.obj:
   $(CC) $(CFLAGS) $<

all: batched.exe

batched.exe: \
   util.obj \
   _api.obj \
   main.obj \
   object.obj \
   main.res \
   {$(LIB)}afcbsu00.lib \
   {$(LIB)}afcpmu00.lib
   $(CL) @<<
      $(LFLAGS)
      /O:batched.exe
      afcbsu00.lib
      afcpmu00.lib
      util.obj
      _api.obj
      main.obj
      object.obj
    <<
    $(RB) $(RBFLAGS) main.res batched.exe

main.res: \
    main.rc \
    main.dlg \
    be.ico \
    msgs.h \
    main.h

util.obj: \
    util.c \
    msgs.h \
    main.h \
    api.h

object.obj: \
    object.c \
    msgs.h \
    main.h \
    api.h

main.obj: \
    main.c \
    msgs.h \
    main.h \
    api.h

_api.obj: \
    _api.c


