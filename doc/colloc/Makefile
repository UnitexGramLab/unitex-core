all: rap.pdf


rap.pdf: rap.tex
	latex rap.tex
	bibtex rap
	latex rap.tex
	latex rap.tex
	dvipdf rap.dvi

clean:
	-rm *.log *.pdf *.dvi *.aux *.bbl *.blg