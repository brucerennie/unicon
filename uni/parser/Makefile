BASE=../..
include $(BASE)/Makedefs.uni

IYACC=$(UNI)/iyacc/iyacc

.PHONY: all clean deps

UFILES=classinfo.u databaseinfo.u packageinfo.u \
	parsedclass.u parsedfunction.u parsedinitiallymethod.u \
	parsedmethod.u parsedobject.u parsedprocedure.u parsedprogram.u \
	parsedrecord.u parser.u \
	unigram.u unigram.action.u unigram.prefix.u unigram.suffix.u\
	node.u links.u info.u grammar.u parsed.u

PROGS=showtree$(EXE) showdb$(EXE)

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

links.u: links.icn unigram.u info.u parsed.u ../lib/tree.u ../lib/unilex.u ../lib/preproce.u

info.u: info.icn classinfo.u databaseinfo.u packageinfo.u

node.u: node.icn

grammar.u: grammar.icn node.u unigram.action.u unigram.prefix.u unigram.suffix.u

parsed.u: parsed.icn parsedclass.u parsedfunction.u parsedinitiallymethod.u parsedmethod.u \
	parsedobject.u parsedprocedure.u parsedprogram.u parsedrecord.u

parser.u: parser.icn links.u

classinfo.u : classinfo.icn databaseinfo.u

databaseinfo.u : databaseinfo.icn

packageinfo.u : packageinfo.icn classinfo.u databaseinfo.u

parsedclass.u : parsedclass.icn parsedinitiallymethod.u parsedmethod.u parsedobject.u

parsedfunction.u : parsedfunction.icn parsedobject.u

parsedinitiallymethod.u : parsedinitiallymethod.icn parsedfunction.u

parsedmethod.u : parsedmethod.icn parsedfunction.u

parsedobject.u : parsedobject.icn

parsedprocedure.u : parsedprocedure.icn parsedfunction.u

parsedprogram.u : parsedprogram.icn parsedclass.u parsedobject.u parsedprocedure.u parsedrecord.u

parsedrecord.u : parsedrecord.icn parsedobject.u

showdb.u : showdb.icn classinfo.u packageinfo.u

showtree.u : showtree.icn parsedprogram.u parser.u

test: clean all
	showtree$(EXE) classinfo.icn &>examples/classinfo.output
	showtree$(EXE) databaseinfo.icn &>examples/databaseinfo.output
	showtree$(EXE) grammar.icn &>examples/grammar.output
	showtree$(EXE) info.icn &>examples/info.output
	showtree$(EXE) links.icn &>examples/links.output
	showtree$(EXE) node.icn &>examples/node.output
	showtree$(EXE) packageinfo.icn &>examples/packageinfo.output
	showtree$(EXE) parsedclass.icn &>examples/parsedclass.output
	showtree$(EXE) parsedinitiallymethod.icn &>examples/parsedinitiallymethod.output
	showtree$(EXE) parsedmethod.icn &>examples/parsedmethod.output
	showtree$(EXE) parsedprogram.icn &>examples/parsedprogram.output
	showtree$(EXE) parsedrecord.icn &>examples/parsedrecord.output
	showtree$(EXE) parsedmethod.icn &>examples/parsedmethod.output
	showtree$(EXE) parsedprocedure.icn &>examples/parsedprocedure.output
	showtree$(EXE) parsedobject.icn &>examples/parsedobject.output
	showtree$(EXE) parsedfunction.icn &>examples/parsedfunction.output
	showtree$(EXE) parser.icn &>examples/parser.output
	showtree$(EXE) unigram.action.icn &>examples/unigram.action.output
	showtree$(EXE) unigram.prefix.icn &>examples/unigram.prefix.output
	showtree$(EXE) unigram.suffix.icn &>examples/unigram.suffix.output

