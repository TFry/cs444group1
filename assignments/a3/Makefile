obj-m := ramdisk.o
TARGET=writeup
KDIR := ../../../fresh-dir
PWD := $(shell pwd)


all: pdf mod

default: pdf

mod:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

all: pdf

dvi: ${TARGET}.tex 
	latex ${TARGET}.tex
	latex $(TARGET).tex

ps: dvi
	dvips -R -Poutline -t letter ${TARGET}.dvi -o ${TARGET}.ps

pdf: ps
	ps2pdf ${TARGET}.ps

clean:
	rm -f *.aux
	rm -f *.log
	rm -f *.dvi
	rm -f *.out
	rm -rf .tmp_versions
	rm -f ${TARGET}.pdf
	rm -f *.ps
	rm -f *.o 
	rm -f *.ko 
	rm -f *.ko.unsigned 
	rm -f modules.order 
	rm -f Module.symvers ramdisk.mod.c 
	rm -f .*.cmd

