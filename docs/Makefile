#
# TexInfo Documentation
#
MAKEINFO= makeinfo
TEXI2DVI= texi2dvi
DVIPS= dvips

doc:
	$(MAKEINFO) saturn.texi
	$(TEXI2DVI) saturn.texi && $(DVIPS) -o saturn.ps saturn.dvi
	@echo "*** Install saturn.info manually, please"

clean:
	-rm saturn.aux saturn.cp saturn.cps saturn.dvi saturn.info saturn.log saturn.ps saturn.toc
