CC=cc
CFLAGS=-dynamic -qopenmp
LIBS= -lnuma
EXE=thrd_aff.x
OBJS=thread_affinities.cc

all:
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $(EXE)

clean:
	rm -rf $(EXE) *.optrpt *.s
