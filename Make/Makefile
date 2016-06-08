CC = g++ -std=c++0x -lboost_program_options
CLASS = -c
FLAGS = -Wall -Wextra #-Werror #-Wfatal-errors
ROOT = `root-config --libs --cflags --ldflags`
ROOFIT = -lRooFit -lRooFitCore -lRooStats -lHistFactory

EXE = likelihoodProfile.exe DrawProfileBatch.exe EditWS.exe


all : ${EXE}
	echo "Up to date" 

likelihoodProfile.exe : likelihoodProfile.o RobustMinimize.o
	$(CC) $^ -o $@ $(ROOT) $(ROOFIT)

DrawProfileBatch.exe : DrawProfileBatch.o Style_Christophe.o
	$(CC) $^ -o $@ $(ROOT) $(ROOFIT)

EditWS.exe : EditWS.o CouplingDef.o
	$(CC) $^ -o $@ $(ROOT) $(ROOFIT)


%.o : %.cpp
	$(CC) $(CLASS) $(FLAGS)  $(ROOT) $(ROOFIT) $<