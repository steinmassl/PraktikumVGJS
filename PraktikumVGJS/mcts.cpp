#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <functional>
#include <string>
#include <algorithm>
#include <chrono>
#include <numeric>

#include "tests.h"
#include "mctsStructs.h"

using namespace std::chrono;

namespace mcts {

	using namespace vgjs;

	auto				g_global_mem = n_pmr::synchronized_pool_resource({ .max_blocks_per_chunk = 10000, .largest_required_pool_block = 1 << 10 }, n_pmr::new_delete_resource());

	auto				g_global_mem_f = n_pmr::synchronized_pool_resource({ .max_blocks_per_chunk = 10000, .largest_required_pool_block = 1 << 10 }, n_pmr::new_delete_resource());
	thread_local auto	g_local_mem_f = n_pmr::unsynchronized_pool_resource({ .max_blocks_per_chunk = 10000, .largest_required_pool_block = 1 << 10 }, n_pmr::new_delete_resource());

	auto				g_global_mem_c = n_pmr::synchronized_pool_resource({ .max_blocks_per_chunk = 10000, .largest_required_pool_block = 1 << 10 }, n_pmr::new_delete_resource());
	thread_local auto	g_local_mem_c = n_pmr::unsynchronized_pool_resource({ .max_blocks_per_chunk = 10000, .largest_required_pool_block = 1 << 10 }, n_pmr::new_delete_resource());


	// Monte Carlo Tree Search algorithm
	class MonteCarloTreeSearch {
	public:
		// Configure MCTS here
		static constexpr uint32_t WIN_SCORE = 10;							// Score awarded to nodes leading to win
		static constexpr uint32_t DRAW_SCORE = 5;							// Score awarded to nodes leading to draw
		static constexpr uint32_t MAX_ITERATIONS = 500;						// Number of iterations of algorithm
		static constexpr uint32_t NUM_TREES = 256;								// Number of parallel trees to create when using root parallelization
	private:
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
		void find_next_move_function(uint32_t player, uint32_t tree_num) {
			//auto start = high_resolution_clock::now();
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

				uint32_t rollout_result = rollout(node_to_explore, opponent);		// Rollout					

				backPropagation(node_to_explore, rollout_result);					// Backpropagation

				num_iterations++;
			}

			Node* winner_node = root->getChildWithMaxScore();		// Pick best move for this round
			results[tree_num] = winner_node->state.game;			// Save game / board
			//auto duration = duration_cast<microseconds>(high_resolution_clock::now() - start);
			//std::cout << duration.count() << "\n";
		}

		Coro<> find_next_move_coro(uint32_t player, uint32_t tree_num) {
			find_next_move_function(player, tree_num);
			co_return;
		}

		// Use simplified majority vote to choose next move after parallelized search
		void majority_vote_function(uint32_t num_trees) {
			//std::cout << "Majority: ";
			std::unordered_map<Game, uint32_t, Game::GameHasher> hash;			// Hash into map to efficiently find most frequent element
			for (uint32_t i = 0; i < num_trees; i++) {
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
		}
		/*
		Coro<> majority_vote_coro(uint32_t num_trees) {
			majority_vote_function(num_trees);
			co_return;
		}
		*/
		// Find best move by creating multiple trees in parallel and merging all results
		void find_next_move_root_parallel_function(uint32_t player, uint32_t num_trees) {
			for (uint32_t i = 0; i < num_trees; i++) {
				schedule([=, this]() {find_next_move_function(player, i); });
			}
			//continuation([=, this]() {majority_vote_function(num_trees); });
		}

		Coro<> find_next_move_root_parallel_coro(uint32_t player, uint32_t num_trees) {
			n_pmr::vector<Coro<>> trees;
			trees.reserve(num_trees);
			for (uint32_t i = 0; i < num_trees; i++) {
				trees.emplace_back(find_next_move_coro(player, i));
			}
			co_await trees;
			//co_await majority_vote_coro(num_trees);
			co_return;
		}
		
		Game getCurrentGame() {
			return current_game;
		}

		void setCurrentGame(Game game) {
			current_game = game;
		}
	};

	// Execute sequentially but with multiple trees
	void mcts(uint32_t num_trees) {
		Game tic_tac_toe_game = Game();
		uint32_t player = Game::P1;
		uint32_t num_moves = 0;
		uint32_t total_moves = Game::DEFAULT_BOARD_SIZE * Game::DEFAULT_BOARD_SIZE;
		MonteCarloTreeSearch mcts(tic_tac_toe_game);

		do {
			for (uint32_t i = 0; i < num_trees; i++) {
				mcts.find_next_move_function(player, i);
				//std::cout << "Finished sequential tree " << i << "\n";
			}
			mcts.majority_vote_function(num_trees);
			player = 3 - player;
			num_moves++;
			//mcts.getCurrentGame().print();		// Show Board
			//std::cout << "Number of moves: " << num_moves << std::endl << std::endl;
		} while (mcts.getCurrentGame().checkStatus() == -1 && num_moves < total_moves);
	}


	template<bool WITHALLOCATE = false, typename FT1 = Function, typename FT2 = std::function<void(void)>>
	Coro<std::tuple<double, double>> performance_function(bool print = true, bool wrtfunc = true, unsigned int num = 1000, unsigned int num_trees = 128, std::pmr::memory_resource* mr = std::pmr::new_delete_resource()) {
		auto& js = JobSystem::instance();

		double duration0_sum = 0;
		double duration2_sum = 0;

		// calculate mcts multiple times
		for (uint32_t i = 0; i < num / num_trees; i++) {

			// no JS
			auto start0 = high_resolution_clock::now();
			mcts(num_trees);
			auto duration0 = duration_cast<microseconds>(high_resolution_clock::now() - start0);
			//std::cout << "duration0: " << duration0.count() <<"\n";
			duration0_sum += duration0.count();

			Game tic_tac_toe = Game();
			MonteCarloTreeSearch mcts(tic_tac_toe);
			MonteCarloTreeSearch* mcts_ptr = &mcts;
			uint32_t player = Game::P1;
			uint32_t total_moves = Game::DEFAULT_BOARD_SIZE * Game::DEFAULT_BOARD_SIZE;
			uint32_t num_moves = 0;

			// multithreaded
			auto start2 = high_resolution_clock::now();
			if constexpr (std::is_same_v<FT1, Function>) {
				do {
					co_await([=]() {mcts_ptr->find_next_move_root_parallel_function(player, num_trees); });
					mcts.majority_vote_function(num_trees);
					player = 3 - player;
					num_moves++;
					//mcts.getCurrentGame().print();		// Show Board
					//std::cout << "Number of moves: " << num_moves << std::endl << std::endl;
				} while (mcts.getCurrentGame().checkStatus() == -1 && num_moves < total_moves);
			}
			else {
				do {
					co_await mcts.find_next_move_root_parallel_coro(player, num_trees);
					mcts.majority_vote_function(num_trees);
					player = 3 - player;
					num_moves++;
					//mcts.getCurrentGame().print();		// Show Board
					//std::cout << "Number of moves: " << num_moves << std::endl << std::endl;
				} while (mcts.getCurrentGame().checkStatus() == -1 && num_moves < total_moves);
			}
			auto duration2 = duration_cast<microseconds>(high_resolution_clock::now() - start2);
			//std::cout << "duration2: " << duration2.count() << "\n";
			duration2_sum += duration2.count();
		}

		// calculate + output
		double speedup0 = duration0_sum / duration2_sum;
		double efficiency0 = speedup0 / js.get_thread_count().value;
		if (wrtfunc) {
			if (print) {
				std::cout << "Wrt function calls: MCTS " << std::right << std::setw(3) << num_trees << " trees" << " Speedup " << std::left << std::setw(8) << speedup0 << " Efficiency " << std::setw(8) << efficiency0 << std::endl;
			}
			co_return std::make_tuple(speedup0, efficiency0);
		}
	}

	template<bool WITHALLOCATE = false, typename FT1, typename FT2>
	Coro<> performance_driver(std::string text, std::pmr::memory_resource* mr = std::pmr::new_delete_resource(), int runtime = 256) {
		int num = runtime;
		const int st = 8;
		const int mt = MonteCarloTreeSearch::NUM_TREES;
		const int dt1 = 1;
		const int dt2 = 10;
		const int dt3 = 100;
		const int dt4 = 10;
		int mdt = dt1;
		bool wrt_function = true; //speedup wrt to sequential function calls w/o JS

		auto& js = JobSystem::instance();

		std::cout << "\nPerformance for " << text << " on " << js.get_thread_count().value << " threads\n\n";
		co_await performance_function<WITHALLOCATE, Function, std::function<void(void)>>(false, wrt_function, mt, mt); //heat up, allocate enough jobs
		for (int num_trees = st; num_trees <= mt; num_trees = num_trees * 2) {
			auto [speedup, eff] = co_await performance_function<WITHALLOCATE, FT1, FT2>(true, wrt_function, num, num_trees);
		}
		co_return;
	}

	Coro<> test() {

		co_await performance_driver<false, Function, std::function<void(void)>>("std::function calls (w / o allocate)");
		//co_await performance_driver<true, Function, std::function<void(void)>>("std::function calls (with allocate new/delete)", std::pmr::new_delete_resource());
		//co_await performance_driver<true, Function, std::function<void(void)>>("std::function calls (with allocate synchronized)", &g_global_mem_f);
		//co_await performance_driver<true, Function, std::function<void(void)>>("std::function calls (with allocate unsynchronized)", &g_local_mem_f);

		co_await performance_driver<false, Coro<>, Coro<>>("Coro<> calls (w / o allocate)");
		//co_await performance_driver<true, Coro<>, Coro<>>("Coro<> calls (with allocate new/delete)", std::pmr::new_delete_resource());
		//co_await performance_driver<true, Coro<>, Coro<>>("Coro<> calls (with allocate synchronized)", &g_global_mem_f);
		//co_await performance_driver<true, Coro<>, Coro<>>("Coro<> calls (with allocate unsynchronized)", &g_local_mem_f);

		//int win_status = mcts.getCurrentGame().checkStatus();
		//std::cout << "Status: " << win_status << std::endl;
		//std::cout << "Ending MCTS Test" << std::endl;

		co_return;
	}


	/*
	void loopForEachRound(uint32_t n) {
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
	*/
}