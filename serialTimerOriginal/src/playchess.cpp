#include <cstdlib>
#include <cstdio>
#include <list>
#include "chessboard.h"
#include "humanplayer.h"
#include "aiplayer.h"
#include "CycleTimer.h"

using namespace std;

const int TIE_THRESHOLD = 50;

void startAIAdversarialChess(int level, FILE* fp, int* nodeCount) {
	ChessBoard board;
	list<Move> regulars, nulls;
	int turn = WHITE;
	Move move;
	bool found;
	
	double start, decisionTime;

	// Initialize players
	AIPlayer black(BLACK, level);
	AIPlayer white(WHITE, level);

	// setup board
	board.initDefaultSetup();
	char mesg[30];
	int turnCount = 0;
	int blackRepeatNum = 0;
	Move blackLastMove;
	int whiteRepeatNum = 0;
	Move whiteLastMove;
	
	for(;;) {
		// show board
		//board.print();
		*nodeCount = 0;

		// query player's choice
		if(turn) {
			fprintf(fp, "%d,black,", turnCount);
			
			start = CycleTimer::currentSeconds();
			found = black.getMove(board, move, nodeCount);
			decisionTime = CycleTimer::currentSeconds() - start;
			fprintf(fp, "%d,%.20f,", *nodeCount, decisionTime);
			
			if(turnCount == 1)
				blackLastMove = move;
			else {
				if(FIGURE(move.figure) == FIGURE(blackLastMove.figure))
					blackRepeatNum++;
				else
					blackRepeatNum = 0;
				
				blackLastMove = move;
			}
		}
		else {
			fprintf(fp, "%d,white,", turnCount);
			start = CycleTimer::currentSeconds();
			found = white.getMove(board, move, nodeCount);
			decisionTime = CycleTimer::currentSeconds() - start;
			fprintf(fp, "%d,%.20f,", *nodeCount, decisionTime);
			
			if(turnCount == 0)
				whiteLastMove = move;
			else {
				if(FIGURE(move.figure) == FIGURE(whiteLastMove.figure))
					whiteRepeatNum++;
				else 
					whiteRepeatNum = 0;
				
				whiteLastMove = move;
			}
		}
		
		if(found) {
			move.print(mesg);
			fprintf(fp, "%s,\n", mesg);
		}
		
		if(whiteRepeatNum > TIE_THRESHOLD || blackRepeatNum > TIE_THRESHOLD) {
			printf("Repeat move the same piece %d times./nThis game ended in a tie.\n", TIE_THRESHOLD);
			break;
		}

		if(!found)
			break;

		// if player has a move get all moves
		regulars.clear();
		nulls.clear();
		board.getMoves(turn, regulars, regulars, nulls);

		// execute maintenance moves
		for(list<Move>::iterator it = nulls.begin(); it != nulls.end(); ++it)
			board.move(*it);

		// execute move
		board.move(move);
		//move.print(mesg);

		// opponents turn
		turn = TOGGLE_COLOR(turn);
		turnCount++;
	}
	
	ChessPlayer::Status status = board.getPlayerStatus(turn);

	switch(status)
	{
		case ChessPlayer::Checkmate:
			printf("Checkmate\n");
			fprintf(fp, "Checkmate,\n");
			break;
		case ChessPlayer::Stalemate:
			printf("Stalemate\n");
			fprintf(fp, "Stalemate,\n");
			break;
	}
	
	board.print();
}

int main() {
	int gameLV = 2;
	int gameLVMAX = 4;
	int gameNum = 10;
	FILE* fp;
	FILE* nodefp;
	char optName[30];
	int nodeCount;
	
	for(int j = gameLV; j < gameLVMAX + 1; ++j) {
		for(int i = 0; i < gameNum; ++i) {
			sprintf(optName, "Game%d_LV%d_Record.csv", i + 1, j);
			
			if(!(fp = fopen(optName, "w"))) {
				printf("%s open fail!\n", optName);
				continue;
			}
			
			fprintf(fp, "turn number,turn,node number,decition time,move,\n");
			
			double start, game_time;
			start = CycleTimer::currentSeconds();
			startAIAdversarialChess(j, fp, &nodeCount);
			game_time = CycleTimer::currentSeconds() - start;
			printf("Game%d time: %fs\n", i + 1, game_time);
			fclose(fp);
		}
	}
	
	if(!system("PAUSE"))
		return -1;
	return 0;
}
