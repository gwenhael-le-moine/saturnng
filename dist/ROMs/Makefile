.PHONY: get-roms mrproper

get-roms: gxrom-l gxrom-m gxrom-p gxrom-r

mrproper:
	rm -f gxrom-l gxrom-m gxrom-p gxrom-r

# HP 48Gx
gxrom-l:
	curl "https://www.hpcalc.org/hp48/pc/emulators/gxrom-l.zip" --output - | funzip > "gxrom-l"
gxrom-m:
	curl "https://www.hpcalc.org/hp48/pc/emulators/gxrom-m.zip" --output - | funzip > "gxrom-m"
gxrom-p:
	curl "https://www.hpcalc.org/hp48/pc/emulators/gxrom-p.zip" --output - | funzip > "gxrom-p"
gxrom-r:
	curl "https://www.hpcalc.org/hp48/pc/emulators/gxrom-r.zip" --output - | funzip > "gxrom-r"

# HP 49
rom.49g:
	curl "https://www.hpcalc.org/hp49/pc/rom/beta1196.zip" --output rom-49g.zip
	unzip rom-49g.zip rom.49g
	rm rom-49g.zip
