
.PHONY: all clean

all: report.pdf

report.pdf: report.tex
	platex -shell-escape report.tex
	platex -shell-escape report.tex
	dvipdfmx report

clean:
	rm -rf *.dvi *.aux *.log *.pdf _minted-*
