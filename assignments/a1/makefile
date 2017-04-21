TARGET=writeup
HTML=main_html

default: pdf producer

all: pdf producer

producer:
	gcc producer.c -pthread -o prod

dvi: ${TARGET}.tex 
	latex ${TARGET}.tex
	latex $(TARGET).tex

ps: dvi
	dvips -R -Poutline -t letter ${TARGET}.dvi -o ${TARGET}.ps

pdf: ps
	ps2pdf ${TARGET}.ps

clean-dvi:
	rm -f ${TARGET}.aux
	rm -f ${TARGET}.log
	rm -f ${TARGET}.dvi
	rm -f ${TARGET}.out
	rm -f prod
	rm -f *.o

clean-ps: clean-dvi
	rm -f ${TARGET}.ps

clean-pdf: clean-ps
	rm -f ${TARGET}.pdf

clean: clean-pdf

