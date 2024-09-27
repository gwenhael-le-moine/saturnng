all: chf.info chf.dvi chf.ps chf.pdf

chf.info: chf.texi
	makeinfo chf.texi

chf.dvi: chf.texi
	texi2dvi chf.texi

chf.ps: chf.dvi
	dvips -o chf.ps chf.dvi

chf.pdf: chf.dvi
	dvips -o chf.pdf chf.dvi

clean:
	rm -f chf.aux chf.cp chf.cps chf.fn chf.fns chf.log chf.toc chf.tp chf.tps

mrproper: clean
	rm -f chf.info chf.dvi chf.ps chf.pdf
