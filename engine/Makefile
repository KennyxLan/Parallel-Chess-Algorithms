target := chess
source := aiplayer.cpp aiplayer.h chessboard.cpp chessboard.h chessplayer.h humanplayer.cpp humanplayer.h playchess.cpp fasttime.h

${target} : ${source}
	g++ -O2 -std=c++17 -pthread -o chess *.cpp

# g++ -O2 -std=c++17 -lpthread -o chess *.cpp
# g++ -O2 -o chess *.cpp

edit :
	vim aiplayer.cpp

zip : 
	zip packed ${source}

clean:
	rm chess
