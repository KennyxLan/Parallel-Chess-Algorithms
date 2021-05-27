SRCDIR = ./src
OBJSDIR = ./objs
OBJS = $(OBJSDIR)/playchess.o $(OBJSDIR)/chessboard.o $(OBJSDIR)/humanplayer.o \
       $(OBJSDIR)/aiplayer.o
	   
CFLAGS = -O2 -o

default: chess

.PHONY: clean dirs
	
chess: dirs $(OBJS)
	g++ -static -static-libgcc -static-libstdc++ $(CFLAGS) chess $(OBJS)
	
$(OBJSDIR)/playchess.o: $(SRCDIR)/playchess.cpp $(SRCDIR)/chessboard.h \
	$(SRCDIR)/humanplayer.h $(SRCDIR)/aiplayer.h
	g++ $(CFLAGS) $(OBJSDIR)/playchess.o -c $(SRCDIR)/playchess.cpp
	
$(OBJSDIR)/chessboard.o: $(SRCDIR)/chessboard.cpp $(SRCDIR)/chessboard.h \
	$(SRCDIR)/chessplayer.h
	g++ $(CFLAGS) $(OBJSDIR)/chessboard.o -c $(SRCDIR)/chessboard.cpp
	
$(OBJSDIR)/humanplayer.o: $(SRCDIR)/humanplayer.cpp $(SRCDIR)/humanplayer.h \
	$(SRCDIR)/chessboard.h $(SRCDIR)/chessplayer.h
	g++ $(CFLAGS) $(OBJSDIR)/humanplayer.o -c $(SRCDIR)/humanplayer.cpp
	
$(OBJSDIR)/aiplayer.o: $(SRCDIR)/aiplayer.cpp $(SRCDIR)/chessboard.h \
	$(SRCDIR)/chessplayer.h
	g++ $(CFLAGS) $(OBJSDIR)/aiplayer.o -c $(SRCDIR)/aiplayer.cpp

clean:
	rm -rf $(OBJSDIR) *.o chess.exe chess

dirs:
	mkdir -p $(OBJSDIR)
