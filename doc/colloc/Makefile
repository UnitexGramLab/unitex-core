all: rap.pdf rap.ps

rap.aux: rap.tex rap.bib difint.tex
	latex rap.tex
	bibtex rap
	latex rap.tex
	latex rap.tex

rap.pdf: rap.aux
	dvipdf rap.dvi


rap.ps: rap.aux
	dvips rap.dvi -orap.ps

clean:
	-rm *.log *.pdf *.dvi *.aux *.bbl *.blg *.ps