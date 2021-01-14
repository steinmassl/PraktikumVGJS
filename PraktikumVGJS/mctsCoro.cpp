#include "tests.h"
#include "mctsStructs.h"

namespace mctsCoro {

	// Monte Carlo Tree Search algorithm
	class MonteCarloTreeSearch {

		// Configure MCTS here
		static constexpr uint32_t WIN_SCORE = 10;							// Score awarded to nodes leading to win
		static constexpr uint32_t DRAW_SCORE = 5;							// Score awarded to nodes leading to draw
		static constexpr uint32_t MAX_ITERATIONS = 500;						// Number of iterations of algorithm
		static constexpr uint32_t NUM_TREES = 4;								// Number of parallel trees to create when using root parallelization
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
				node->children.emplace_back(new Node(state, node));			// Initialize children vector with amount of possible states
			}
		}

		// Rollout Phase - simulate rest of the game from a specific node
		uint32_t rollout(Node* node, uint32_t opponent) {
			Node* current_node = node;
			State current_state = current_node->state;
			int32_t game_status = current_state.game.checkStatus();
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
		void backPropagation(Node* node_to_explore, uint32_t rollout_result) {
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
		Coro<> findNextMove(uint32_t player, uint32_t tree_num) {

			uint32_t opponent = 3 - player;		// Opponent of current player
			Tree tree = Tree(current_game);
			Node* root = &tree.root;
			root->state.player = opponent;	// Opponent made previous move
			uint32_t num_iterations = 0;

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

				uint32_t rollout_result = rollout(node_to_explore, opponent);			// Rollout					

				backPropagation(node_to_explore, rollout_result);					// Backpropagation

				num_iterations++;
			}

			Node* winner_node = root->getChildWithMaxScore();		// Pick best move for this round
			results[tree_num] = winner_node->state.game;			// Save game / board
			co_return;
		}

		// Find best move by creating multiple trees in parallel and merging all results
		Coro<> findNextMoveWithRootParallelization(uint32_t player) {
			n_pmr::vector<Coro<>> trees;
			trees.reserve(NUM_TREES);
			for (uint32_t i = 0; i < NUM_TREES; i++) {
				trees.emplace_back(findNextMove(player, i));
			}
			co_await trees;
			co_await similarityVote();
			co_return;
		}

		// Use simplified similarity vote to choose next move after parallelized search
		Coro<> similarityVote() {
			//std::cout << "Similarity: ";
			std::unordered_map<Game, uint32_t, Game::GameHasher> hash;			// Hash into map to efficiently find most frequent element
			for (uint32_t i = 0; i < NUM_TREES; i++) {
				hash[results[i]]++;											// Add votes from trees to their respective chosen moves 
			}
			uint32_t max_count = 0;
			Game res;
			for (auto& ele : hash) {
				if (ele.second > max_count) {
					res = ele.first;
					max_count = ele.second;
				}
			}
			//std::cout << max_count << " trees voted for this move" << std::endl;
			current_game = res;
			co_return;
		}

		Game getCurrentGame() {
			return current_game;
		}
	};








	Game tic_tac_toe_game = Game();
	MonteCarloTreeSearch mcts(tic_tac_toe_game);
	uint32_t player = Game::P1;
	uint32_t total_moves = Game::DEFAULT_BOARD_SIZE * Game::DEFAULT_BOARD_SIZE;

	Coro<> loopForEachRound(uint32_t n) {
		//std::cout << "Player " << player << std::endl;
		co_await mcts.findNextMoveWithRootParallelization(player);			// Find move using MCTS with root parallelization

		//mcts.getCurrentGame().print();		// Show Board
		//std::cout << "Number of moves: " << n + 1 << std::endl << std::endl;
		if (mcts.getCurrentGame().checkStatus() == -1 && n < total_moves) {			// Game is not over
			player = 3 - player;													// Toggle player
			co_await loopForEachRound(n + 1);										// Next round
		}
		co_return;
	}

	//Test MCTS
	Coro<> test() {

		//std::cout << "Starting MCTS Test" << std::endl;
		auto start = std::chrono::high_resolution_clock::now();
		co_await loopForEachRound(0);
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> elapsed_milliseconds = end - start;
		std::cout << std::endl << 	"   Test: mctsCoro" << std::endl;
		std::cout << 				"   Execution time:   " << elapsed_milliseconds.count() << " ms" << std::endl;
		
		
		//int win_status = mcts.getCurrentGame().checkStatus();
		//std::cout << "Status: " << win_status << std::endl;
		//std::cout << "Ending MCTS Test" << std::endl;
		co_return;
	}
}