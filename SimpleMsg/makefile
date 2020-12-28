!include <ntwin32.mak>

INTDIR=.
OUTDIR=.
HDIR=.

NAME=SimpleMsg

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

all: clear $(OUTDIR)\$(NAME).dll

clear: 
	del *.bin
	del *.res
	del *.h
	del *.rc

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"
    
$(OUTDIR)\$(NAME).dll: $(INTDIR)\$(NAME).res
  $(link) $(lflags) -base:0x60000000 -machine:$(CPU) -dll -noentry \
    -out:$*.dll $(INTDIR)\$(NAME).res
    
$(HDIR)\$(NAME).h: $(NAME).mc
  del $(OUTDIR)\$(NAME).rc

#WARNING: using the -c option can cause problems with replaceable parameter
#strings. Do not compile message files that contain replaceable parameter
#strings with -c.

$(OUTDIR)\$(NAME).rc: $(NAME).mc
  if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"
  mc -w $(NAME).mc -r $(OUTDIR) -h $(HDIR)
   
.rc.res:
    rc -r -fo $*.res $*.rc
