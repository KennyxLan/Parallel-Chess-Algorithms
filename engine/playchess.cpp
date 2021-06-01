//#include <mcheck.h>
#include <cstdlib>
#include <cstdio>
#include <list>
#include "chessboard.h"
#include "humanplayer.h"
#include "aiplayer.h"
#include <ctime>

using namespace std;

int main(void) {

	ChessBoard board;
	list<Move> regulars, nulls;
	int turn = WHITE;
	Move move;
	bool found;

	// Initialize players
	////AIPlayer black(BLACK, 3); original search_depth is 3
	AIPlayer black(BLACK, SEARCH_DEPTH);
	//HumanPlayer white(WHITE);
	AIPlayer white(WHITE, SEARCH_DEPTH);


	//// start: seeding the pseudo random number generator
	int seed; 
	//seed = time(nullptr);
	//seed = 1234567890; //// fix the seed for debugging
	seed = 8888; //// fix the seed for debugging
	srand(seed);
	//// end: seeding the pseudo random number generator


	// create a csv file
	FILE* filePtr = nullptr;
	char fileName[50];
	sprintf(fileName, "ava_seed_%d_thread_%d_depth_%d.csv", seed, NUM_THREADS, SEARCH_DEPTH); 
	filePtr = fopen(fileName, "w");
	if(filePtr == nullptr){
		printf(">>>>>> fopen() failed !!! <<<<<<\n");
		return -2;
	}


	// fprint the header
	printf("seed == %d, NUM_THREADS == %d, SEARCH_DEPTH == %d,\n", seed, NUM_THREADS, SEARCH_DEPTH);
	fprintf(filePtr, "seed == %d, NUM_THREADS == %d, SEARCH_DEPTH == %d,\n", seed, NUM_THREADS, SEARCH_DEPTH);
	for(int tid = 0; tid < NUM_THREADS; tid++){
		fprintf(filePtr, "thread%d_time,", tid);
	}
	fprintf(filePtr, "parallel_time,");
	for(int tid = 0; tid < NUM_THREADS; tid++){
		fprintf(filePtr, "thread%d_nodes,", tid);
	}
	fprintf(filePtr, "serial_time,serial_nodes,best_same,candidates_same,move\n");



	// setup board
	board.initDefaultSetup();

	for(;;) {
	//for(int i = 0; i < 16; i++) {
		// show board
		//board.print();

		// query player's choice
		if(turn)
			found = black.getMove(board, move, filePtr);
		else
			found = white.getMove(board, move, filePtr);

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
		//move.print();
		move.print(filePtr);

		// opponents turn
		turn = TOGGLE_COLOR(turn);
	}

	ChessPlayer::Status status = board.getPlayerStatus(turn);

	switch(status)
	{
		case ChessPlayer::Checkmate:
			printf("Checkmate\n");
			fprintf(filePtr, "Checkmate\n");
			break;
		case ChessPlayer::Stalemate:
			printf("Stalemate\n");
			fprintf(filePtr, "Stalemate\n");
			break;
	}

	fclose(filePtr);
}
