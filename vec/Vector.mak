#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
#* Project: VECTOR FOR THE PSION 3a          *  Written by: Nick Matthews  *
#*  Module: PSION SIBOSDK/TOPSPEED MAKEFILE  *  Date Started: 27 Aug 1996  *
#*    File: VECTOR.MAK      Type: MAKEFILE   *  Date Revised: 13 Oct 1998  *
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *


INC=e:\sibosdk\include
PRJ=vector

TARGET = PSION_3A
FINALZIP = vecp120
BETAZIP  = vecp12b5
BACKZIP  = vecp120s

!ifdef MDEBUG
CD_FLAG = /v2 /dDEBUG /d$(TARGET)
AD_FLAG = /DDEBUG=1 /D$(TARGET)=1
!else
CD_FLAG = /v0 /d$(TARGET)
AD_FLAG = /DDEBUG=0 /D$(TARGET)=1
!endif

.c.obj:
  tsc $*.c /fp$(PRJ).pr /zq $(CD_FLAG)

# .cat.obj: use specific intructions

.asm.obj:
  ml /c /Cp /Fl$*.asl /Sa $(AD_FLAG) $*.asm

.cat.g:
  ctran $*.cat -e$(INC)\ -g -s -v
  copy $*.g $(INC)

.rss.rsc:
  del $*.rsc
  rcomp $*.rss -i$(INC)\ -o$*.rsc -v

.rss.rsg:
  del $*.rsg
  rcomp $*.rss -i$(INC)\ -g$*.rsg -v

.re.rg:
  rgprep $*.re $*.rex
  lprep -i$(INC)\ -o$*.i $*.rex
  del $*.rex
  rgcomp $*.i $*.rg
  del $*.i
  copy $*.rg $(INC)

.rsc.rzc:
  rchuf hwim -v -i$*.rsc -o$*.rzc

.rht.rhi:
  phcomp $*.rht -v

.ms.shd:
  makeshd $*.ms

.pcx.pic:
  wspcx -p $*.pcx

############################################################################

all : vector.app vecqcd.vdl vecdxf.vdl

vector.app : vector.img
  #del vector.exe
  copy vector.img vector.app
  #edump vector

vector.img : vector.obj vecmain.obj veccom.obj vecdrg.obj vecfile.obj \
             vecdlg.obj vecdwin.obj vecband.obj vecdata.obj veclocs.obj \
             veciwin.obj vecswin.obj drawline.obj hypot.obj trig.obj \
             process.obj circle.obj vecldlg.obj regread.obj vecabout.obj \
             vecmisc.obj drawgrid.obj vecundo.obj vecdraw.obj clear.obj \
             vectext.obj vectdlg.obj veccalc.obj vecsadlg.obj vecprint.obj \
             vecplot.obj vecpage.obj vecudlg.obj vecscale.obj vecdim.obj \
             vector.afl vector48.pic vector.rzc vector.shd default.vft \
             vector.dfl vecsym.dyl vecbld.dyl \
             vector.pr
  del vector.img
  tsc /L vector.pr $(CD_FLAG)

#  del vector.exe
#  copy vector.img vector.app

vector.afl : vector.mak
  echo vector48.pic>  vector.afl
  echo vector.rzc>>   vector.afl
  echo vector.shd>>   vector.afl
  echo default.vft>>  vector.afl

vector.ms : vector.mak
  echo Vector.VEC>  vector.ms
  echo \VEC\>>      vector.ms
  echo 1003>>       vector.ms

# NOTE: the order of dyl's is critical, see vector.h
vector.dfl : vector.mak
  echo vecsym.dyl>  vector.dfl
  echo vecbld.dyl>> vector.dfl

#vector.plk : vector.mak
#  echo vector24.pic>  vector.plk
#  echo vector48.pic>> vector.plk

#vector.pic : vector.plk vector24.pic vector48.pic
#  wspcx -L vector.plk

#vector24.pic : vector24.pcx
vector48.pic : vector48.pcx

vector.obj : vector.cat vector.g
  ctran $*.cat -e$(INC)\ -c -s -v
  tsc $*.c /fp$(PRJ).pr /zq $(CD_FLAG)
  ecobj $*.obj

vector.g : vector.cat vecfile.h
  ctran $*.cat -e$(INC)\ -g -s -v
  copy $*.g $(INC)
  copy vecfile.h $(INC)

vector.rg : vector.re vector.g

vechelp.rhi : vechelp.rht vhpstart.rht vhpmove.rht vhpsel.rht vhpinfo.rht \
              vhpkeys.rht vhpreg.rht vhplic.rht vhpsup.rht vhpsym.rht

VECRSS = vector.rss vector.rg vecbld.rsi vecbld.rg \
         vecsym.rsi vecsym.rg popup.rh vechelp.rhi \
         vecqcd.rsi vecdxf.rsi

vector.rsg : $(VECRSS)

vector.rsc : $(VECRSS)

vector.rzc : vector.rsc

vector.shd : vector.ms

drawline.obj : drawline.asm kcmacros.inc vector.inc
circle.obj   : circle.asm kcmacros.inc vector.inc
clear.obj    : clear.asm kcmacros.inc vector.inc
hypot.obj    : hypot.asm kcmacros.inc
trig.obj     : trig.asm kcmacros.inc
process.obj  : process.asm kcmacros.inc vector.inc
drawgrid.obj : drawgrid.asm kcmacros.inc vector.inc

VECBLD = vecbld.h vecbld.g
VEC_G = vector.g vecfile.h

regread.obj  : regread.c regread.h

vecmain.obj  : vecmain.c $(VEC_G) $(VECBLD) vector.rsg vector.h
veccom.obj   : veccom.c $(VEC_G) $(VECBLD) vector.rsg vector.h vecxch.h
vecfile.obj  : vecfile.c $(VEC_G) vector.rsg vector.h
vecundo.obj  : vecundo.c $(VEC_G) vector.rsg vector.h
vecdrg.obj   : vecdrg.c $(VEC_G) $(VECBLD) vector.rsg vector.h vecsym.g vecdebug.h
vecdlg.obj   : vecdlg.c $(VEC_G) vector.rsg vector.h
vecldlg.obj  : vecldlg.c $(VEC_G) vector.rsg vector.h
vectdlg.obj  : vectdlg.c $(VEC_G) $(VECBLD) vector.rsg vector.h
vecdwin.obj  : vecdwin.c $(VEC_G) $(VECBLD) vector.rsg vector.h
vecband.obj  : vecband.c $(VEC_G) $(VECBLD) vector.rsg vector.h
vecdata.obj  : vecdata.c $(VEC_G) vector.rsg vector.h
veciwin.obj  : veciwin.c $(VEC_G) vector.rsg vector.h
vecswin.obj  : vecswin.c $(VEC_G) vector.rsg vector.h
vecmisc.obj  : vecmisc.c $(VEC_G) vector.rsg vector.h
vecdraw.obj  : vecdraw.c $(VEC_G) vector.rsg vector.h
vectext.obj  : vectext.c $(VEC_G) vecsym.g vector.rsg vector.h
veccalc.obj  : veccalc.c $(VEC_G) vector.h
veclocs.obj  : veclocs.c $(VEC_G) vector.h
vecabout.obj : vecabout.c $(VEC_G) vector.rsg vector.h regread.h
vecsadlg.obj : vecsadlg.c $(VEC_G) vector.rsg vector.h
vecprint.obj : vecprint.c $(VEC_G) vector.rsg vector.h
vecplot.obj  : vecplot.c $(VEC_G) vector.h
vecpage.obj  : vecpage.c $(VEC_G) vector.rsg vector.h
vecudlg.obj  : vecudlg.c $(VEC_G) vector.rsg vector.h
vecscale.obj : vecscale.c $(VEC_G) vector.h
vecdim.obj   : vecdim.c $(VEC_G) vector.h

#------------------------------[ VecSym.dyl ]------------------------------#

vecsym.dyl : vecsym.obj vsymwin.obj \
             vector.pr
  tsc /L vecsym.pr $(CD_FLAG)

vecsym.obj : vecsym.cat vecsym.g
  ctran $*.cat -e$(INC)\ -c -s -v
  tsc $*.c /fp$(PRJ).pr /zq $(CD_FLAG)
  ecobj $*.obj

vecsym.g : vecsym.cat $(VEC_G)

vecsym.rg : vecsym.re vecsym.g

vsymwin.obj : vsymwin.c vector.h $(VECBLD) vector.rsg vecsym.g $(VEC_G) vecdebug.h

#------------------------------[ VecBld.dyl ]------------------------------#

vecbld.dyl : vecbld.obj vecbuild.obj vecsel.obj vecaggr.obj vecbreak.obj \
             bldsym.obj vecprop.obj vecpdlg.obj \
             vecjump.obj vecexp.obj \
             vecbld.pr
  tsc /L vecbld.pr $(CD_FLAG)

vecbld.obj : vecbld.cat $(VECBLD)
  ctran $*.cat -e$(INC)\ -c -s -v
  tsc $*.c /fp$(PRJ).pr /zq $(CD_FLAG)
  ecobj $*.obj

vecbld.g : vecbld.cat $(VEC_G)

vecbld.rg : vecbld.re $(VECBLD)

vecbuild.obj : vecbuild.c $(VEC_G) $(VECBLD)  vector.rsg vector.h
bldsym.obj   : bldsym.c $(VEC_G) $(VECBLD) vector.rsg vector.h vecsym.g
vecsel.obj   : vecsel.c $(VEC_G) $(VECBLD) vector.rsg vector.h
vecbreak.obj : vecbreak.c $(VEC_G) $(VECBLD) vector.rsg vector.h
vecaggr.obj  : vecaggr.c $(VEC_G) $(VECBLD) vector.rsg vector.h
vecprop.obj  : vecprop.c $(VEC_G) $(VECBLD) vector.rsg vector.h
vecpdlg.obj  : vecpdlg.c $(VEC_G) $(VECBLD) vector.rsg vector.h
vecjump.obj  : vecjump.c $(VEC_G) $(VECBLD) vector.rsg vector.h
vecexp.obj   : vecexp.c $(VEC_G) $(VECBLD) vector.rsg vector.h vecpcx.h

#------------------------------[ VecDxf.vdl ]------------------------------#

vecdxf.vdl : vecdxf.obj vecdxfob.obj vecdxfim.obj vecdxfex.obj \
             vecdxf.pr
  tsc /L vecdxf.pr $(CD_FLAG)
  copy vecdxf.dyl vecdxf.vdl

vecdxf.obj : vecdxf.cat vecdxf.g vec.h
  ctran $*.cat -e$(INC)\ -c -s -v
  tsc $*.c /fp$(PRJ).pr /zq $(CD_FLAG)
  ecobj $*.obj

vecdxf.g : vecdxf.cat $(VEC_G)

vecdxfob.obj : vecdxfob.c vecdxf.g $(VEC_G) vector.rsg
vecdxfim.obj : vecdxfim.c vecdxf.g $(VEC_G) vector.rsg
vecdxfex.obj : vecdxfex.c vecdxf.g $(VEC_G) vector.rsg

#------------------------------[ VecQcd.vdl ]------------------------------#

vecqcd.vdl : vecqcd.obj vecqcdim.obj vecqcdex.obj \
             vecqcd.pr
  tsc /L vecqcd.pr $(CD_FLAG)
  copy vecqcd.dyl vecqcd.vdl

vecqcd.obj : vecqcd.cat vecqcd.g vec.h
  ctran $*.cat -e$(INC)\ -c -s -v
  tsc $*.c /fp$(PRJ).pr /zq $(CD_FLAG)
  ecobj $*.obj

vecqcd.g : vecqcd.cat $(VEC_G)

vecqcdim.obj : vecqcdim.c vecqcd.g $(VEC_G) vector.rsg
vecqcdex.obj : vecqcdex.c vecqcd.g $(VEC_G) vector.rsg

#-------------------------------[ Final zip ]------------------------------#

makezip : $(FINALZIP).zip

$(FINALZIP).zip : vector.app readme.txt vpmanual.wrd shapes.vsl ascii.vsl \
              logic.vsl electric.vsl traction.vec jkff.vec
  call makezip $(FINALZIP)

#-------------------------------[ Beta zip ]-------------------------------#

makebeta : $(BETAZIP).zip

$(BETAZIP).zip : vector.app vecdxf.vdl readme_.txt
  del $(BETAZIP).zip
  zip $(BETAZIP).zip vector.app vecdxf.vdl readme_.txt

#------------------------------[ Backup zip ]------------------------------#


backup : vectorbu.rsp
  pkzip g:\p\vector\backup\$(BACKZIP).zip -u @vectorbu.rsp

vectorbu.rsp : vector.mak
  echo files.txt    >  vectorbu.rsp
  echo vecspec.txt  >> vectorbu.rsp
  echo vectodo.txt  >> vectorbu.rsp
  echo vecmain.c    >> vectorbu.rsp
  echo vecmisc.c    >> vectorbu.rsp
  echo veccalc.c    >> vectorbu.rsp
  echo vecdraw.c    >> vectorbu.rsp
  echo vectext.c    >> vectorbu.rsp
  echo vecdwin.c    >> vectorbu.rsp
  echo veciwin.c    >> vectorbu.rsp
  echo vecswin.c    >> vectorbu.rsp
  echo veccom.c     >> vectorbu.rsp
  echo vecdata.c    >> vectorbu.rsp
  echo vecdrg.c     >> vectorbu.rsp
  echo vecband.c    >> vectorbu.rsp
  echo vecfile.c    >> vectorbu.rsp
  echo vecundo.c    >> vectorbu.rsp
  echo vecdlg.c     >> vectorbu.rsp
  echo vecldlg.c    >> vectorbu.rsp
  echo vectdlg.c    >> vectorbu.rsp
  echo vecjump.c    >> vectorbu.rsp
  echo veclocs.c    >> vectorbu.rsp
  echo vsymwin.c    >> vectorbu.rsp
  echo vecbuild.c   >> vectorbu.rsp
  echo bldsym.c     >> vectorbu.rsp
  echo vecsel.c     >> vectorbu.rsp
  echo vecbreak.c   >> vectorbu.rsp
  echo vecaggr.c    >> vectorbu.rsp
  echo vecprop.c    >> vectorbu.rsp
  echo vecpdlg.c    >> vectorbu.rsp
  echo vector.h     >> vectorbu.rsp
  echo vecfile.h    >> vectorbu.rsp
  echo vecbld.h     >> vectorbu.rsp
  echo drawline.asm >> vectorbu.rsp
  echo circle.asm   >> vectorbu.rsp
  echo drawgrid.asm >> vectorbu.rsp
  echo hypot.asm    >> vectorbu.rsp
  echo trig.asm     >> vectorbu.rsp
  echo process.asm  >> vectorbu.rsp
  echo kcmacros.inc >> vectorbu.rsp
  echo vector.inc   >> vectorbu.rsp
  echo vector.mak   >> vectorbu.rsp
  echo vector.cat   >> vectorbu.rsp
  echo vecsym.cat   >> vectorbu.rsp
  echo vecbld.cat   >> vectorbu.rsp
  echo vector48.pcx >> vectorbu.rsp
  echo vector.pr    >> vectorbu.rsp
  echo vecsym.pr    >> vectorbu.rsp
  echo vecbld.pr    >> vectorbu.rsp
  echo popup.rh     >> vectorbu.rsp
  echo vector.re    >> vectorbu.rsp
  echo vecsym.re    >> vectorbu.rsp
  echo vecbld.re    >> vectorbu.rsp
  echo vector.rss   >> vectorbu.rsp
  echo vecsym.rsi   >> vectorbu.rsp
  echo vecbld.rsi   >> vectorbu.rsp
  echo vechelp.rht  >> vectorbu.rsp
  echo vhpstart.rht >> vectorbu.rsp
  echo vhpmove.rht  >> vectorbu.rsp
  echo vhpsel.rht   >> vectorbu.rsp
  echo jkff.vec     >> vectorbu.rsp
  echo logic.vsl    >> vectorbu.rsp
  echo electric.vsl >> vectorbu.rsp
  echo default.vft  >> vectorbu.rsp
  echo readme.txt   >> vectorbu.rsp
  echo regread.h    >> vectorbu.rsp
  echo regread.c    >> vectorbu.rsp
  echo vecabout.c   >> vectorbu.rsp
  echo vecdebug.h   >> vectorbu.rsp
  echo clear.asm    >> vectorbu.rsp
  echo vecsadlg.c   >> vectorbu.rsp
  echo vecexp.c     >> vectorbu.rsp
  echo vecpcx.h     >> vectorbu.rsp
  echo vhpinfo.rht  >> vectorbu.rsp
  echo vhpkeys.rht  >> vectorbu.rsp
  echo vpmanual.wrd >> vectorbu.rsp
  echo vhpreg.rht   >> vectorbu.rsp
  echo vhplic.rht   >> vectorbu.rsp
  echo ascii.vsl    >> vectorbu.rsp
  echo shapes.vsl   >> vectorbu.rsp
  echo traction.vec >> vectorbu.rsp
  echo vhpsup.rht   >> vectorbu.rsp
  echo vecprint.c   >> vectorbu.rsp
  echo vecplot.c    >> vectorbu.rsp
  echo vecpage.c    >> vectorbu.rsp
  echo readme_.txt  >> vectorbu.rsp
  echo history.txt  >> vectorbu.rsp
  echo history_.txt >> vectorbu.rsp
  echo vecqcd.cat   >> vectorbu.rsp
  echo vecqcd.pr    >> vectorbu.rsp
  echo vecqcdim.c   >> vectorbu.rsp
  echo vecxch.h     >> vectorbu.rsp
  echo vec.h        >> vectorbu.rsp
  echo vecqcdex.c   >> vectorbu.rsp
  echo vecqcd.rsi   >> vectorbu.rsp
  echo vecdxf.cat   >> vectorbu.rsp
  echo vecdxf.pr    >> vectorbu.rsp
  echo vecdxfim.c   >> vectorbu.rsp
  echo vecdxfex.c   >> vectorbu.rsp
  echo vecdxf.rsi   >> vectorbu.rsp
  echo vecdxfob.c   >> vectorbu.rsp
  echo vecfile.txt  >> vectorbu.rsp
  echo vecudlg.c    >> vectorbu.rsp
  echo vecscale.c   >> vectorbu.rsp
  echo vecdim.c     >> vectorbu.rsp

# End of VECTOR.MAK file
