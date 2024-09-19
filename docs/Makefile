all: saturn.info saturn.dvi saturn.ps saturn.pdf

saturn.info: saturn.texi
	makeinfo saturn.texi

saturn.dvi: saturn.texi
	texi2dvi saturn.texi

saturn.ps: saturn.dvi
	dvips -o saturn.ps saturn.dvi

saturn.pdf: saturn.dvi
	dvips -o saturn.pdf saturn.dvi

clean:
	rm -f saturn.aux saturn.cp saturn.cps saturn.fn saturn.fns saturn.log saturn.toc saturn.tp saturn.tps

mrproper: clean
	rm -f saturn.info saturn.dvi saturn.ps saturn.pdf
