BASE=../..
include $(BASE)/Makedefs.uni

export PATH:=$(BIN):$(PATH)

U= unicon.u unigram.u unilex.u tree.u preproce.u idol.u unix.u tokens.u yyerror.u main.u cfy.u ca.u node.u grammar.u links.u
UG= unigram.action.u unigram.prefix.u unigram.suffix.u

# Don't delete unigram.u and idol.u, needed for bootstrapping
UDEL= unicon.u unilex.u tree.u preproce.u unix.u tokens.u yyerror.u main.u cfy.u ca.u links.u node.u grammar.u unigram.action.u unigram.prefix.u unigram.suffix.u

UCFILES= unicon.icn unigram.icn unilex.icn tree.icn preproce.icn idol.u unix.icn tokens.icn yyerror.icn main.icn cfy.icn ca.icn unigram.action.icn unigram.prefix.icn unigram.suffix.icn

# Files from this dir that (for now) we copy into lib/ for use by external
# programs.  Long term, need to reconcile with or replace by stuff in parser/
LIBFILES= ../lib/unilex.u ../lib/tree.u ../lib/preproce.u

all: unicon $(WUNICONTARGET)
unicon $(UNICONEXE): $(U) $(LIBFILES) $(UG)
	$(ICONT) -o unicon main.icn
	$(CP) unicon$(EXE) $(BIN)

# A windows-specific build option
wunicon $(WUNICONEXE): $(U) $(UG)
	$(ICONT) -c $(UG)
	$(ICONT) -G -o wunicon$(EXE) $(U)
	$(CP) wunicon$(EXE) $(BIN)

%.u: %.icn

unicon.u : unicon.icn links.u
	$(ICONT) $(UFLAGS) -c unicon

../lib/unilex.u: unilex.u
	$(CP) unilex.u ../lib

unilex.u : unilex.icn ytab_h.icn
	$(ICONT) $(UFLAGS) -c unilex

main.u: main.icn
	$(ICONT) $(UFLAGS) -c main

../lib/tree.u: tree.u
	$(CP) tree.u ../lib

tree.u : tree.icn node.u
	$(ICONT) $(UFLAGS) -c tree

tokens.u : tokens.icn ytab_h.icn
	$(ICONT) $(UFLAGS) -c tokens

../lib/preproce.u: preproce.u
	$(CP) preproce.u ../lib

preproce.u : preproce.icn
	$(ICONT) $(UFLAGS) -c preproce

cfy.u : cfy.icn
	$(ICONT) $(UFLAGS) -c cfy

ca.u : ca.icn
	$(ICONT) $(UFLAGS) -c ca

node.u : node.icn
	$(ICONT) $(UFLAGS) -c node

links.u : links.icn
	$(ICONT) $(UFLAGS) -c links

grammar.u : grammar.icn node.u $(UG)
	$(ICONT) $(UFLAGS) -c grammar

unigram.action.u: unigram.action.icn
	$(ICONT) -c unigram.action.icn

unigram.prefix.u: unigram.prefix.icn
	$(ICONT) -c unigram.prefix.icn

unigram.suffix.u: unigram.suffix.icn
	$(ICONT) -c unigram.suffix.icn


# Commented out to avoid bootstrap problem.
# Uncomment if you modify unigram.y/unigram.icn
unigram.u: unigram.icn $(UG) node.u
	$(ICONT) -c unigram

# build iyacc, and uncomment these lines, if you change the language grammar (or unigram.y)
unigram.icn : unigram.y ytab_h.icn
	$(IYACC) -i unigram.y

#ytab_h.icn: unigram.y
#	$(IYACC) -i -d unigram.y
#	mv unigram_tab.icn ytab_h.icn

# these lines were used when Idol was involved in the build process
#unigram.icn : unigram.y ytab_h.icn
#	$(IYACC) -i unigram.y
#	mv unigram.icn unigram.iol
#	idol -c unigram.iol

#
# If you have other unicon binaries on your path (!) you might need this to say "./merr -u ./unicon".
# If you need it to run on Windows, you might have to use .\ instead of ./
#
#yyerror.icn: unigram.icn
#	./merr -u unicon

yyerror.u: yyerror.icn
	$(ICONT) -c yyerror

# Uncomment if you modify idol.icn
#idol.u: idol.icn
#	$(UNICON) -c idol

unix.u: unix.icn
	$(ICONT) $(UFLAGS) -c unix

#
# Don't clean everything, we need some .u files for bootstrapping.
#

clean Clean:
	$(RM) unicon$(EXE) wunicon$(EXE) uniclass.dir uniclass.pag uniclass.db $(UDEL)
