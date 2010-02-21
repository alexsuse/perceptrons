LFLAGS = -lgsl -lgslcblas -ligraph -lm -Wall -O2 -m64
MFLAGS = -L/opt/local/lib -I/opt/local/include
CC = gcc

linux: sociedade.c
	$(CC) -o sociedade sociedade.c $(LFLAGS)
	
mac: sociedade.c
	$(CC) -o sociedade sociedade.c $(MFLAGS) $(LFLAGS)
