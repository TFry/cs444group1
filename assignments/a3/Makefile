obj-m := ramdisk.o
KDIR := ../../../linux-yocto-3.14
PWD := $(shell pwd)


all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
clean:
	rm -f *.pdf *.ps *.dvi *.out *.log *.aux *.bbl *.blg *.pyg *-blx.bib *.run.xml
	rm -f *.o *.ko *.ko.unsigned modules.order Module.symvers ebd.mod.c .*.cmd
	rm -rf .tmp_versions

.PHONY: all show clean ps pdf showps debug test
