all: efp
	@echo "BUILT SUCCESSFULLY!"

CPLEXDIR      = /opt/ibm/ILOG/CPLEX_Studio1263/cplex
CONCERTDIR    = /opt/ibm/ILOG/CPLEX_Studio1263/concert



CCLNFLAGS = -L$(CPLEXDIR)/lib/x86-64_linux/static_pic -DIL_STD -lilocplex -lcplex -L$(CONCERTDIR)/lib/x86-64_linux/static_pic -lconcert -m64 -lm -pthread
CCINCFLAG = -I$(CPLEXDIR)/include -I$(CONCERTDIR)/include 

efp:
	g++ ./mainGreen.cpp ./SPOOLStructSmartStorage.cpp ./OptFrame/Scanner++/Scanner.cpp $(CCLNFLAGS) $(CCINCFLAG) --std=c++11 -O3 -o main

	
clean:
	#make clean -C ./Examples/
	rm -f mainOptFrame
