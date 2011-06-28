BINDIR = ../bin/
# If you change PREFIX, be sure to change path in etc/init.d/daemongps!!!
PREFIX = /usr/local
OPT=    -O -Wall

all: $(BINDIR)/statgps $(BINDIR)/daemongps

install: all
	mkdir -p $(PREFIX)/bin
	mkdir -p $(PREFIX)/sbin
	cp $(BINDIR)/statgps $(PREFIX)/bin/statgps
	cp $(BINDIR)/daemongps $(PREFIX)/sbin/daemongps

install-etc: install
	cp ../etc/init.d/daemongps /etc/init.d/.
	ln -s ../init.d/daemongps /etc/rc2.d/S99daemongps

# ---------------------------------------------------------------------------------------------------
# to show GPS STATUS directly fom shared memory
# ---------------------------------------------------------------------------------------------------
$(BINDIR)/statgps:	statgps.c $(BINDIR)
	cc $(OPT) -c statgps.c initshrmemgps.c
	cc -o statgps $(OPT) statgps.o initshrmemgps.o dateheure.c jdgregjdc.c -lncurses -lm
	mv statgps $(BINDIR)/.

# ---------------------------------------------------------------------------------------------------
# to receive and process GPS STATUS from Serial line and put it in shared memory
# ---------------------------------------------------------------------------------------------------
$(BINDIR)/daemongps:	daemongps.c $(BINDIR)
	cc $(OPT) -c daemongps.c initshrmemgps.c tsip_rpt.c tsip_ifc.c dateheure.c jdgregjdc.c
	cc -o daemongps $(OPT) daemongps.o initshrmemgps.o tsip_rpt.o tsip_ifc.o dateheure.o jdgregjdc.o -lm
	mv daemongps $(BINDIR)/.

# ---------------------------------------------------------------------------------------------------
# to send command to the GPS through the Serial line
# ---------------------------------------------------------------------------------------------------
$(BINDIR)/send2gps:	send2gps.c $(BINDIR)
	cc $(OPT) -c send2gps.c initshrmemgps.c
	cc -o send2gps $(OPT) send2gps.o initshrmemgps.o dateheure.c jdgregjdc.c -lm
	mv send2gps $(BINDIR)/.

$(BINDIR):
	mkdir -p $@

# ---------------------------------------------------------------------------------------------------
# CLEANING
# ---------------------------------------------------------------------------------------------------
clean:	
	\rm *.o
