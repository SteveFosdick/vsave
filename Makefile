all: vsave.ssd vlist

CFLAGS =-O2 -Wall

vsave.ssd: blank.ssd check parsfn setup vload vlodsrc vlodlst vsave vsavsrc vsavlst
	cp blank.ssd vsave.ssd
	afscp check parsfn setup vload vlodsrc vlodlst vsave vsavsrc vsavlst vsave.ssd:

vsave: vsavsrc parsfn
	laxasm -o vsave -l vsavlst vsavsrc

vload: vlodsrc parsfn
	laxasm -o vload -l vlodlst vlodsrc

vlist: vlist.c
	$(CC) $(CFLAGS) -o vlist vlist.c -lm
