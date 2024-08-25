.PHONY: get-roms mrproper

get-roms: sxrom-a sxrom-b sxrom-c sxrom-d sxrom-e sxrom-j gxrom-l gxrom-m gxrom-p gxrom-r rom.49g

mrproper:
	rm -f ./sxrom-a ./sxrom-b ./sxrom-c ./sxrom-d ./sxrom-e ./sxrom-j ./gxrom-l ./gxrom-m ./gxrom-p ./gxrom-r ./rom.49g

# HP 48Sx
sxrom-a:
	curl "https://www.hpcalc.org/hp48/pc/emulators/sxrom-a.zip" --output - | funzip > "sxrom-a"
sxrom-b:
	curl "https://www.hpcalc.org/hp48/pc/emulators/sxrom-b.zip" --output - | funzip > "sxrom-b"
sxrom-c:
	curl "https://www.hpcalc.org/hp48/pc/emulators/sxrom-c.zip" --output - | funzip > "sxrom-c"
sxrom-d:
	curl "https://www.hpcalc.org/hp48/pc/emulators/sxrom-d.zip" --output - | funzip > "sxrom-d"
sxrom-e:
	curl "https://www.hpcalc.org/hp48/pc/emulators/sxrom-e.zip" --output - | funzip > "sxrom-e"
sxrom-j:
	curl "https://www.hpcalc.org/hp48/pc/emulators/sxrom-j.zip" --output - | funzip > "sxrom-j"

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

# HP 39g/40g
rom.40g:
	curl "https://www.hpcalc.org/hp40/pc/rom3940.zip" --output - | funzip > "rom.40g"
