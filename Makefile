all: vsave vload vlist

CFLAGS =-O2 -Wall

vsave: vsavsrc parsfn
	laxasm -o vsave -l vsavlst vsavsrc

vload: vlodsrc parsfn
	laxasm -o vload -l vlodlst vlodsrc

vlist: vlist.c
	$(CC) $(CFLAGS) -o vlist vlist.c -lm
