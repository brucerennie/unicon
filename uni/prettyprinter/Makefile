BASE=../..
include $(BASE)/Makedefs.uni

IYACC=$(UNI)/iyacc/iyacc

.PHONY: all clean deps

UFILES=parser.u prettyprinter.u \
	unigram.u unigram.action.u unigram.prefix.u unigram.suffix.u \
	node.u links.u grammar.u

PROGS=showtree$(EXE)

all: $(UFILES) $(PROGS)

clean:
	$(RM) $(PROGS) unigram.icn *.u uniclass.dir uniclass.pag uniclass.db

deps:
	$(UNIDEP) *.icn -f deps.out -nb

unigram.icn : unigram.y ytab_h.icn
	$(IYACC) -i unigram.y

unigram.u: unigram.icn grammar.u

unigram.action.u: unigram.action.icn

unigram.prefix.u: unigram.prefix.icn

unigram.suffix.u: unigram.suffix.icn

links.u: links.icn unigram.u unilex.u ../lib/preproce.u

node.u: node.icn

grammar.u: grammar.icn node.u unigram.action.u unigram.prefix.u unigram.suffix.u

parser.u: parser.icn links.u

prettyprinter.u: prettyprinter.icn

showtree.u : showtree.icn parser.u prettyprinter.u

test: clean all

