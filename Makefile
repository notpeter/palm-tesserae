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
PRCFLAGS	= -t appl -o $(PRC) -n $(ICONTEXT) -c $(APPID) 

# Uncomment this if you want to build a GDB-debuggable version
# mdebug-labels allows the log files in the palmOS debugger to put names
# on it's stack traces.  Super useful. (p67 Using Palm OS Emulator PDF)
#CFLAGS = -O0 -Wall -g -mdebug-labels
CFLAGS = -O0 -g -mdebug-labels
#CFLAGS = -O2

.PHONY: depend dep clean veryclean dd

all: $(PRC)

$(PRC): $(APP) $(APP).ro
	$(BUILDPRC) $(PRCFLAGS) $(APP).ro $(APP)
#	$(BUILDPRC) $(PRC) $(ICONTEXT) $(APPID) *.grc *.bin

$(APP): $(SRC:.c=.o);
	@echo Linking...
	@$(CC) $(CFLAGS) $^ -o $@

$(APP).ro: $(RCP)
	@echo Compiling Resources...
	$(PILRC) -q -ro $^ $(APP).ro
#	touch $@

%.o: %.c $(HDR) Makefile
	@echo Compiling Code...
	@$(CC) $(CFLAGS) -c $< -o $@

#               touch $<
# Enable the previous line if you want to compile EVERY time.

depend dep:
	$(CC) -M $(SRC) > .dependencies

clean:
	rm -rf *.o $(APP) *.ro *.bin *.grc *~

veryclean: clean
	rm -rf *.prc *.bak
