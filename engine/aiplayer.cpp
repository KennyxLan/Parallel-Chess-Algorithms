#include <cstdlib>
#include <ctime>
#include <list>
#include <vector>
#include "aiplayer.h"
#include "chessboard.h"

#include <pthread.h>  //// for Pthreads programming
#include <cstdio>     //// printf()
#include "fasttime.h" //// gettime(), tdiff()


using namespace std;


//// edit: pack the arguments in an Arguments object
class Arguments{
  public:
	//// constructor
	Arguments(         int threadId_arg,
		     AIPlayer& aiplayer_arg,
		   ChessBoard& board_arg,
		   list<Move>& regulars_arg,
		  vector<Move> candidates_arg,
			   int color_arg,
			   int search_depth_arg,
		           int best_arg,
		           int tmp_arg,
		          bool quiescent_arg,
			   int numOfNodesEvaluated_arg
		)
		//// member initializer list
		:     threadId (threadId_arg),
		      aiplayer (aiplayer_arg),
		         board (board_arg),
		      regulars (regulars_arg),
		    candidates (candidates_arg),
		         color (color_arg),
		  search_depth (search_depth_arg),
		          best (best_arg),
		           tmp (tmp_arg),
		     quiescent (quiescent_arg),
	   numOfNodesEvaluated (numOfNodesEvaluated_arg){}

	//// data members
	         int threadId;
	    AIPlayer aiplayer;
	  ChessBoard board;
	 list<Move>& regulars;
	vector<Move> candidates;
	         int color;
	         int search_depth;
	         int best, tmp;
	        bool quiescent;
		 int numOfNodesEvaluated;
};

//// edit: executed by threads.
void* parallel_search(void* parameters){
	Arguments* args = (Arguments*)parameters;
	//printf("threadId[%d]  %p\n", args->threadId, &(args->board));

	//// using block partitioning
	//// each thread process approximately ( (total moves) / NUM_THREADS ) moves
	//// the remaining moves are spli among the first ( (total moves) % NUM_THREADS ) threads
	int offset = (args->regulars.size()) / NUM_THREADS * (args->threadId);
	int iterations = (args->regulars.size()) / NUM_THREADS;
	int remain = (args->regulars.size()) % NUM_THREADS;
	if(args->threadId <= remain){
		offset += (args->threadId);
		if(args->threadId < remain)
			iterations++;
	}else{
		offset += remain;
	}

	//printf("regulars.size() == %d, threadId == %d, offset == %d, iterations == %d\n", (int)(args->regulars.size()), args->threadId, offset, iterations);
	

	std::list<Move>::iterator it = args->regulars.begin(); // iterator pointing to the first element in args->regulars
	for(int i = 0; i < offset; i++) { it++; }

	for(int j = 0; j < iterations; j++, it++){
		/* start original */
	
		// execute move
		//board.move(*it);
		args->board.move(*it);
	
		// check if own king is vulnerable now
		//if(!board.isVulnerable((this->color ? board.black_king_pos : board.white_king_pos), this->color))
		if(!args->board.isVulnerable(
			((args->color) ? (args->board.black_king_pos) : (args->board.white_king_pos)), args->color)) {
	
			if((*it).capture != EMPTY) {
				//quiescent = true;
				args->quiescent = true;
			}
	
			// recursion
			//tmp = -evalAlphaBeta(board, TOGGLE_COLOR(this->color), this->search_depth - 1, -WIN_VALUE, -best, quiescent);
			args->tmp = -args->aiplayer.evalAlphaBeta(args->board, 
					                           TOGGLE_COLOR(args->color), 
								   args->search_depth - 1, 
								   -WIN_VALUE, 
								   -(args->best), 
								   args->quiescent,
								   args->numOfNodesEvaluated);
	
			//if(tmp > best)
			if((args->tmp) > (args->best)) {
				//best = tmp;
				args->best = args->tmp;
				//candidates.clear();
				args->candidates.clear();
				//candidates.push_back(*it);
				args->candidates.push_back(*it);
			}
			//else if(tmp == best)
			else if((args->tmp) == (args->best)) {
				//candidates.push_back(*it);
				args->candidates.push_back(*it);
			}
		}
	
			// undo move and inc iterator
			args->board.undoMove(*it);
	
		/* end original */
	}


	return (void*)0;
} //// end: parallel_search

AIPlayer::AIPlayer(int color, int search_depth)
 : ChessPlayer(color),
   search_depth(search_depth)
{
	//srand(time(NULL)); // done in playchess.cpp
}

AIPlayer::~AIPlayer()
{}

bool AIPlayer::getMove(ChessBoard & board, Move & move, FILE* filePtr) const
{
	
	list<Move> regulars, nulls;
	vector<Move> candidates;
	bool quiescent = false;
	int best, tmp; //// tmp can be seen as temporary `this.advantage`

	// first assume we are loosing
	best = -KING_VALUE;

	// get all moves
	/* edit: measure time */
	fasttime_t startTime;
	fasttime_t endTime;

	//count the number of nodes evaluated by serial code
	int numberOfNodesEvaluatedBySerial = 0;

	//startTime = gettime();
	board.getMoves(this->color, regulars, regulars, nulls);
	//endTime = gettime();
	//printf("board.getMoves()    took %lf secs\n", tdiff(startTime, endTime));


	// execute maintenance moves  //// clear marks of passant on pawns
	//startTime = gettime();
	for(list<Move>::iterator it = nulls.begin(); it != nulls.end(); ++it)
		board.move(*it);
	//endTime = gettime();
	//printf("execute maintenance took %lf secs\n", tdiff(startTime, endTime));


	//// parallelization starts
	pthread_t dummy;
	AIPlayer dummyAIPlayer(this->color, this->search_depth);
	std::vector<pthread_t> threads(NUM_THREADS, dummy);
	std::vector<Arguments> threadArgs(NUM_THREADS,
					  Arguments(0, //// threadId
						    dummyAIPlayer,
						    board,
						    regulars,
						    candidates,
						    this->color,
						    this->search_depth,
						    best,
						    0, //// tmp
						    quiescent,
						    0) //// number of nodes evaluated
					 );


	startTime = gettime();
	for(int tid = 1; tid < NUM_THREADS; tid++){
		(threadArgs[tid]).threadId = tid; //// assign the correct threadId
		pthread_create(&(threads[tid]), nullptr, &parallel_search, (void*)(&(threadArgs[tid])));
		//endTime = gettime();
		//printf("threads[%d] started at %f secs\n", tid, tdiff(startTime, endTime));
	}

	//// main thread
	//endTime = gettime();
	//printf("threads[0] started just before %f secs\n", tdiff(startTime, endTime));
	parallel_search((void*)(&(threadArgs[0])));
	endTime = gettime();
	//printf("threads[0] ended at %f secs\n", tdiff(startTime, endTime));
	printf("%f,", tdiff(startTime, endTime)); // finish time of thread 0
	fprintf(filePtr, "%f,", tdiff(startTime, endTime)); // finish time of thread 0


	//// start: combine the results of individual threads
	int threadBest = (threadArgs[0]).best;
	std::vector<Move> threadCandidates;
	for(int tid = 1; tid < NUM_THREADS; tid++){
		pthread_join(threads[tid], nullptr);
	        endTime = gettime();
		//printf("threads[%d] ended at %f secs\n", tid, tdiff(startTime, endTime));
		printf("%f,", tdiff(startTime, endTime)); // finish time of threa 1 ~ (NUM_THREADS - 1)
		fprintf(filePtr, "%f,", tdiff(startTime, endTime)); // finish time of threa 1 ~ (NUM_THREADS - 1)
		if(threadArgs[tid].best > threadBest)
			threadBest = threadArgs[tid].best;
	}

	for(int tid = 0; tid < NUM_THREADS; tid++){
		if(threadArgs[tid].best == threadBest){
			//// append the candidates of threadId to the end of `threadCandidates`
			threadCandidates.insert(threadCandidates.end(), 
					        threadArgs[tid].candidates.begin(), 
					        threadArgs[tid].candidates.end()); 
		}
	} //// end: combine the results of individual threads

	endTime = gettime();
	//printf("parallel_search     took %lf secs\n", parallelSearchTime);
	printf("%f,", tdiff(startTime, endTime)); // finish time of whole parallel search
	fprintf(filePtr, "%f,", tdiff(startTime, endTime)); // finish time of whole parallel search
	
	
	// start: print the number of nodes evaluated by each thread
	for(int tid = 0; tid < NUM_THREADS; tid++){
		//printf("threads[%d] evaluated %8d nodes\n", tid, threadArgs[tid].numOfNodesEvaluated);
		printf("%8d,", threadArgs[tid].numOfNodesEvaluated); // number of nodes evaluated by thread 0~NUM_THREADS-1
		fprintf(filePtr, "%8d,", threadArgs[tid].numOfNodesEvaluated); // number of nodes evaluated by thread 0~NUM_THREADS-1
	}// end: print the number of nodes evaluated by each thread

	//// parallelization ends

	// loop over all moves  //// and find the candidate moves
	startTime = gettime();
	for(list<Move>::iterator it = regulars.begin(); it != regulars.end(); ++it)
	{
		// execute move
		board.move(*it);

		// check if own king is vulnerable now
		if(!board.isVulnerable((this->color ? board.black_king_pos : board.white_king_pos), this->color)) {

			if((*it).capture != EMPTY) {
				quiescent = true;
			}

			// recursion
			tmp = -evalAlphaBeta(board, TOGGLE_COLOR(this->color), this->search_depth - 1, -WIN_VALUE, -best, quiescent
					, numberOfNodesEvaluatedBySerial);
			if(tmp > best) {
				best = tmp;
				candidates.clear();
				candidates.push_back(*it);
			}
			else if(tmp == best) {
				candidates.push_back(*it);
			}
		}

		// undo move and inc iterator
		board.undoMove(*it);
	}
	endTime = gettime();
	//float serialSearchTime = tdiff(startTime, endTime);
	//printf("loop over all      took %lf secs\n", serialSearchTime);
	printf("%f,", tdiff(startTime, endTime));
	fprintf(filePtr, "%f,", tdiff(startTime, endTime));
	//printf("serial evaluated %8d nodes\n", numberOfNodesEvaluatedBySerial);
	printf("%8d,", numberOfNodesEvaluatedBySerial);
	fprintf(filePtr, "%8d,", numberOfNodesEvaluatedBySerial);


	if(threadBest == best){
		//printf("threadBest == %d, best == %d ............................ OK\n", threadBest, best);
		printf("OK threadBest == best,");
		fprintf(filePtr, "OK threadBest == best,");
	}
	else{
		//printf("threadBest == %d, best == %d ............................ ERROR ERROR ERROR!!!\n", threadBest, best);
		printf("INCORRECT threadBest =/= best,");
		fprintf(filePtr, "INCORRECT threadBest =/= best,");
	}



	if(threadCandidates == candidates){
		//printf("threadCadidates == candidates ........................... OK\n");
		printf("OK threadCandidates == candidates,");
		fprintf(filePtr, "OK threadCandidates == candidates,");
	}
	else{
		//printf("threadCadidates != candidates ........................... ERROR ERROR ERROR!!!\n");
		printf("INCORRECT threadCandidates == candidates,");
		fprintf(filePtr, "INCORRECT threadCandidates == candidates,");
	}



	// undo maintenance moves
	startTime = gettime();
	for(list<Move>::iterator it = nulls.begin(); it != nulls.end(); ++it)
		board.undoMove(*it);
	endTime = gettime();
	//printf("undo maintenance   took %lf secs\n", tdiff(startTime, endTime));
	
	//printf("\n"); // end of this turn's output


	// loosing the game?
	if(best < -WIN_VALUE) {
		return false;
	}
	else {
		// select random move from candidate moves

		move = candidates[rand() % candidates.size()];
		return true;
	}
}

int AIPlayer::evalAlphaBeta(ChessBoard & board, int color, int search_depth, int alpha, int beta, bool quiescent,
		int& numOfNodesEvaluated /* this is for counting the number of nodes evaluated */
		) const
{    //// return the ADVANTAGE of `color` just before `color` moves
	
	// start: count the number of nodes evaluated
	numOfNodesEvaluated++;
	// end: count the number of nodes evaluated

	list<Move> regulars, nulls;
	int best, tmp;

	if(search_depth <= 0 && !quiescent /* `quiescent` means : `color` is being captured! */) {
		if(color)
			return -evaluateBoard(board);   //// -(white points - black points) == (black points - white points)
		else
			return +evaluateBoard(board);
	}

	// first assume we are losing
	best = -WIN_VALUE;

	// get all moves
	board.getMoves(color, regulars, regulars, nulls);
	
	// execute maintenance moves
	for(list<Move>::iterator it = nulls.begin(); it != nulls.end(); ++it)
		board.move(*it);
	
	// loop over all moves
	for(list<Move>::iterator it = regulars.begin();
		alpha <= beta && it != regulars.end(); ++it)
	{
		// execute move
		board.move(*it);

		// check if own king is vulnerable now
		if(!board.isVulnerable((color ? board.black_king_pos : board.white_king_pos), color)) {

			if((*it).capture == EMPTY)
				quiescent = false;
            else
                quiescent = true;

			// recursion 'n' pruning
			tmp = -evalAlphaBeta(board, TOGGLE_COLOR(color), search_depth - 1, -beta, -alpha, quiescent,
					 numOfNodesEvaluated);
			if(tmp > best) {
				best = tmp;
				if(tmp > alpha) {
					alpha = tmp;
				}
			}
		}

		// undo move and inc iterator
		board.undoMove(*it);
	}
	
	// undo maintenance moves
	for(list<Move>::iterator it = nulls.begin(); it != nulls.end(); ++it)
		board.undoMove(*it);
	
	return best;
}

int AIPlayer::evaluateBoard(const ChessBoard & board) const //// (total points for white) MINUS (total points for black)
{
	int figure, pos, sum = 0, summand;

	for(pos = 0; pos < 64; pos++)
	{
		figure = board.square[pos];
		switch(FIGURE(figure))
		{
			case PAWN:
				summand = PAWN_VALUE;
				break;
			case ROOK:
				summand = ROOK_VALUE;
				break;
			case KNIGHT:
				summand = KNIGHT_VALUE;
				break;
			case BISHOP:
				summand = BISHOP_VALUE;
				break;
			case QUEEN:
				summand = QUEEN_VALUE;
				break;
			case KING:
				summand = KING_VALUE;
				break;
			default:
				summand = 0;
				break;
		}
		
		sum += IS_BLACK(figure) ? -summand : summand;
	}
	
	return sum;
}

