APP             = Tess
ICONTEXT        = "Tess"
APPID           = Tess
RCP             = $(APP).rcp
PRC             = $(APP).prc
SRC             = $(APP).c 
HDR		= TessRsc.h 

CC              = m68k-palmos-gcc
PILRC           = pilrc
OBJRES          = m68k-palmos-obj-res
BUILDPRC        = build-prc


# Uncomment this if you want to build a GDB-debuggable version
# mdebug-labels allows the log files in the palmOS debugger to put names
# on it's stack traces.  Super useful. (p67 Using Palm OS Emulator PDF)
CFLAGS = -O0 -Wall -g -mdebug-labels
#CFLAGS = -O0 -g -mdebug-labels
#CFLAGS = -O2

all: $(PRC)

$(PRC): grc.stamp bin.stamp;
	$(BUILDPRC) $(PRC) $(ICONTEXT) $(APPID) *.grc *.bin

grc.stamp: $(APP)
	$(OBJRES) $(APP)
	touch $@

$(APP): $(SRC:.c=.o);
	$(CC) $(CFLAGS) $^ -o $@

bin.stamp: $(RCP)
	$(PILRC) $^ $(BINDIR)
	touch $@

%.o: %.c Makefile $(HDR)
	$(CC) $(CFLAGS) -c $< -o $@

Makefile:
	touch $@

$(HDR):
	touch $@

#               touch $<
# Enable the previous line if you want to compile EVERY time.

depend dep:
	$(CC) -M $(SRC) > .dependencies

clean:
	rm -rf *.o $(APP) *.bin *.grc *.stamp *~

veryclean: clean
	rm -rf *.prc *.bak
