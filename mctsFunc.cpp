#include "mctsFunc.h"
#include "mctsStructs.h"

namespace mctsFunc {

	// Monte Carlo Tree Search algorithm
	class MonteCarloTreeSearch {

		// Configure MCTS here
		static constexpr int WIN_SCORE = 10;							// Score awarded to nodes leading to win
		static constexpr int DRAW_SCORE = 5;							// Score awarded to nodes leading to draw
		static constexpr int MAX_ITERATIONS = 500;						// Number of iterations of algorithm
		static constexpr int NUM_TREES = 4;								// Number of parallel trees to create when using root parallelization
		Game results[NUM_TREES];										// Results from individual trees when using parallelization
		Game current_game;												// Save result of algorithm and access in the following round

		// Selection Phase - traverse tree and find suitable node to expand
		Node* selectNode(Node* root) {
			Node* current = root;
			while (current->children.size() != 0) {
				current = current->findBestChildNodeWithUTC();
			}
			return current;
		}

		// Expansion Phase - expand node adding a child for every possible action
		void expandNode(Node* node) {
			std::vector<State> possibleStates = node->state.getAllPossibleStates();
			node->children.reserve(possibleStates.size());
			for (State& state : possibleStates) {
				node->children.emplace_back(new Node(state,node));			// Initialize children vector with amount of possible states
			}
		}

		// Rollout Phase - simulate rest of the game from a specific node
		int rollout(Node* node, int opponent) {
			Node* current_node = node;
			State current_state = current_node->state;
			int game_status = current_state.game.checkStatus();
			if (game_status == opponent) {							// Opponent won
				current_node->parent->state.score = 0;
				return game_status;
			}
			while (game_status == Game::IN_PROGRESS) {				// Play random moves until the game ends
				current_state.togglePlayer();
				current_state.randomPlay();
				game_status = current_state.game.checkStatus();
			}
			return game_status;
		}

		// Backpropagation Phase - update nodes according to rollout result
		void backPropagation(Node* node_to_explore, int rollout_result) {
			Node* current_node = node_to_explore;
			while (current_node != nullptr) {
				current_node->state.num_visits++;
				if (current_node->state.player == rollout_result)			// Path won
					current_node->state.score += WIN_SCORE;
				else if (rollout_result == current_node->state.game.DRAW)	// Path led to draw
					current_node->state.score += DRAW_SCORE;
				current_node = current_node->parent;
			}
		}
	
	public:
		MonteCarloTreeSearch(Game game) : current_game(game) {}

		// MCTS - find the best move from the current state of the game
		void findNextMove(int player, int tree_num) {

			int opponent = 3 - player;		// Opponent of current player
			Tree tree = Tree(current_game);
			Node* root = &tree.root;
			root->state.player = opponent;	// Opponent made previous move
			int num_iterations = 0;

			// Do this for a number of iterations / amount of time
			while (num_iterations < MAX_ITERATIONS) {
																					// MCTS Phases

				Node* promising_node = selectNode(root);							// Selection

				// If promising node doesn't end game -> expand
				if (promising_node->state.game.checkStatus() == Game::IN_PROGRESS)
					expandNode(promising_node);										// Expansion

				Node* node_to_explore = promising_node;
				if (promising_node->children.size() > 0) {
					node_to_explore = promising_node->getRandomChildNode();
				}

				int rollout_result = rollout(node_to_explore, opponent);			// Rollout					

				backPropagation(node_to_explore, rollout_result);					// Backpropagation

				num_iterations++;
			}

			Node* winner_node = root->getChildWithMaxScore();		// Pick best move for this round
			results[tree_num] = winner_node->state.game;			// Save game / board
		}

		// Find best move by creating multiple trees in parallel and merging all results
		void findNextMoveWithRootParallelization(int player) {
			for (int i = 0; i < NUM_TREES; i++) {
				schedule([=, this]() {findNextMove(player, i); });
			}
			continuation([=, this]() {similarityVote();});
		}

		// Use simplified similarity vote to choose next move after parallelized search
		void similarityVote() {
			//std::cout << "Similarity: ";
			std::unordered_map<Game, int, Game::GameHasher> hash;			// Hash into map to efficiently find most frequent element
			for (int i = 0; i < NUM_TREES; i++) {
				hash[results[i]]++;											// Add votes from trees to their respective chosen moves 
			}
			int max_count = 0;
			Game res;
			for (auto& ele : hash) {
				if (ele.second > max_count) {
					res = ele.first;
					max_count = ele.second;
				}
			}
			//std::cout << max_count << " trees voted for this move" << std::endl;
			current_game = res;
		}

		Game getCurrentGame() {
			return current_game;
		}
	};








	Game tic_tac_toe_game = Game();
	MonteCarloTreeSearch mcts(tic_tac_toe_game);
	int player = Game::P1;
	int total_moves = Game::DEFAULT_BOARD_SIZE * Game::DEFAULT_BOARD_SIZE;

	void loopForEachRound(int n) {
		//std::cout << "Player " << player << std::endl;
		schedule([]() {mcts.findNextMoveWithRootParallelization(player); });			// Find move using MCTS with root parallelization

		continuation([=]() {
			//mcts.getCurrentGame().print();		// Show Board
			//std::cout << "Number of moves: " << n + 1 << std::endl << std::endl;
			if (mcts.getCurrentGame().checkStatus() == -1 && n < total_moves) {			// Game is not over
				player = 3 - player;													// Toggle player
				loopForEachRound(n + 1);												// Next round
			}
		});	
	}

	// Test MCTS
	void test() {

		//std::cout << "Starting MCTS Test" << std::endl;	

		auto start = std::chrono::high_resolution_clock::now();
		schedule([]() {loopForEachRound(0); });
										
		continuation([=]() {
			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> elapsed_milliseconds = end - start;
			std::cout << std::endl << 	"   Test: mctsFunc" << std::endl;
			std::cout << 				"   Execution time:   " << elapsed_milliseconds.count() << " ms" << std::endl;
			
			
			//int win_status = mcts.getCurrentGame().checkStatus();
			//std::cout << "Status: " << win_status << std::endl;
			//std::cout << "Ending MCTS Test" << std::endl;
		});
	}
}