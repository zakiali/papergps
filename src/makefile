BINDIR = /home/obs/bin
OPT=    -O -Wall
LIBPGPLOT=      -L/usr/local/pgplot -lcpgplot -lpgplot -L/usr/X11R6/lib -lX11

# ---------------------------------------------------------------------------------------------------
# to show GPS STATUS directly fom shared memory
# ---------------------------------------------------------------------------------------------------
statgps:	statgps.c
	cc $(OPT) -c statgps.c initshrmemgps.c
	cc -o statgps $(OPT) statgps.o initshrmemgps.o dateheure.c jdgregjdc.c -lncurses -lm
	mv statgps $(BINDIR)/.

# ---------------------------------------------------------------------------------------------------
# to receive and process GPS STATUS from Serial line and put it in shared memory
# ---------------------------------------------------------------------------------------------------
daemongps:	daemongps.c
	cc $(OPT) -c daemongps.c initshrmemgps.c tsip_rpt.c tsip_ifc.c dateheure.c jdgregjdc.c
	cc -o daemongps $(OPT) daemongps.o initshrmemgps.o tsip_rpt.o tsip_ifc.o dateheure.o jdgregjdc.o -lm
	mv daemongps $(BINDIR)/.

# ---------------------------------------------------------------------------------------------------
# to send command to the GPS through the Serial line
# ---------------------------------------------------------------------------------------------------
send2gps:	send2gps.c
	cc $(OPT) -c send2gps.c initshrmemgps.c
	cc -o send2gps $(OPT) send2gps.o initshrmemgps.o dateheure.c jdgregjdc.c -lm
	mv send2gps $(BINDIR)/.

# ---------------------------------------------------------------------------------------------------
# CLEANING
# ---------------------------------------------------------------------------------------------------
clean:	
	\rm *.o
