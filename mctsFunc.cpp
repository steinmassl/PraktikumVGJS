#include <iostream>
#include <math.h>
#include <vector>
#include "VEGameJobSystem.h"
#include "Game.h"

using namespace vgjs;

namespace mctsFunc {

	//A struct carrying information relevant to the MCTS algorithm - state of the game at a specific point
	struct State {
		Game game;
		unsigned int player = 0;		//the player who made the move that resulted in this state
		unsigned int num_visits = 0;
		double score = 0;				//shows how relevant this path is for the algorithm

		State() {}

		State(Game game) : game(game) {}

		//Performs a random move from all available moves
		void randomPlay() {
			std::vector<Game::Pos> available_positions(game.getEmptyPositions());
			game.performMove(player, available_positions.at(rand() % available_positions.size()));
		}

		//Get a list of all possible states that can be achieved from the current state
		std::vector<State> getAllPossibleStates() {
			std::vector<Game::Pos> available_positions(game.getEmptyPositions());
			std::vector<State> possible_states;
			possible_states.reserve(game.num_empty_positions);
			for (auto& pos : available_positions) {
				State new_state = State(game);
				new_state.player = 3 - player;
				new_state.game.performMove(new_state.player, pos);
				possible_states.push_back(new_state);
			}
			return possible_states;
		}

		//Toggle between two active players
		void togglePlayer() {
			player = 3 - player;
		}
	};

	//Node struct which contains state object and links to parent/child nodes
	struct Node {

		State state;
		Node* parent = nullptr;
		std::vector<Node*> children;

		Node() {}

		Node(Game game) : state(State(game)) {}

		Node(State state, Node* parent = nullptr) : state(state), parent(parent) {}

		//Destructor to make sure all child nodes in the tree are destroyed properly
		~Node() {
			for (auto& child : children) {
				delete child;
			}
		}
		

		//Calculate UCT value of current Node
		double calculateUCT() {
			if (state.num_visits == 0)			
				return std::numeric_limits<double>::max();		//nodes that haven't been visited have initial UCT of infinity
			return (state.score / state.num_visits) + 1.41 * sqrt(log(parent->state.num_visits) / state.num_visits);		//UCT calculation
		}

		//Select a random child node from all children of this node
		Node* getRandomChildNode() {
			return children.at(rand() % children.size());
		}

		//Find the child node of current node with the largest UCT value
		Node* findBestChildNodeWithUTC() {
			double largest_UCT = std::numeric_limits<double>::lowest();				//initialize with lowest value
			Node* best_node = nullptr;
			for (auto& child : children) {
				double child_UCT = child->calculateUCT();
				if (child_UCT > largest_UCT) {
					largest_UCT = child_UCT;
					best_node = child;
				}
			}
			return best_node;
		}

		//Select the child node with the highest overall score / the best move to play
		Node* getChildWithMaxScore() {
			double largest_score = std::numeric_limits<double>::lowest();			//initialize with lowest value
			Node* best_node = nullptr;
			for (auto& child : children) {
				if (child->state.score > largest_score) {
					largest_score = child->state.score;
					best_node = child;
				}
			}
			return best_node;
		}
	};



	//Tree struct that provides a starting node
	struct Tree {
		Node root;

		Tree(Game game) {
			root = Node(game);
		}
	};












	

	

	//Monte Carlo Tree Search algorithm
	class MonteCarloTreeSearch {
		static constexpr int WIN_SCORE = 10;							//score awarded to nodes leading to win
		static constexpr int DRAW_SCORE = 5;							//score awarded to nodes leading to draw
		static constexpr int MAX_ITERATIONS = 500;						//number of iterations of algorithm
		static constexpr int NUM_TREES = 16;							//number of parallel trees to create when using root parallelization
		Game results[NUM_TREES];										//results from individual trees when using parallelization
		Game current_game;													//save result of algorithm and access in the following round

		//Selection Phase - traverse tree and find suitable node to expand
		Node* selectNode(Node* root) {
			Node* current = root;
			while (current->children.size() != 0) {
				current = current->findBestChildNodeWithUTC();
			}
			return current;
		}

		//Expansion Phase - expand node adding a child for every possible action
		void expandNode(Node* node) {
			std::vector<State> possibleStates = node->state.getAllPossibleStates();
			node->children.reserve(possibleStates.size());
			for (State& state : possibleStates) {
				node->children.emplace_back(new Node(state,node));			//initialize children vector with amount of possible states
			}
		}

		//Rollout Phase - simulate rest of the game from a specific node
		int rollout(Node* node, int opponent) {
			Node* current_node = node;
			State current_state = current_node->state;
			int game_status = current_state.game.checkStatus();
			if (game_status == opponent) {							//opponent won
				current_node->parent->state.score = 0;
				return game_status;
			}
			while (game_status == Game::IN_PROGRESS) {				//play random moves until the game ends
				current_state.togglePlayer();
				current_state.randomPlay();
				game_status = current_state.game.checkStatus();
			}
			return game_status;
		}

		//Backpropagation Phase - update nodes according to rollout result
		void backPropagation(Node* node_to_explore, int rollout_result) {
			Node* current_node = node_to_explore;
			while (current_node != nullptr) {
				current_node->state.num_visits++;
				if (current_node->state.player == rollout_result)			//path won
					current_node->state.score += WIN_SCORE;
				else if (rollout_result == current_node->state.game.DRAW)	//path led to draw
					current_node->state.score += DRAW_SCORE;
				current_node = current_node->parent;
			}
		}
	
	public:

		MonteCarloTreeSearch(Game game) : current_game(game) {}

		//MCTS - find the best move from the current state of the game
		void findNextMove(int player, int tree_num) {

			int opponent = 3 - player;		//opponent of current player
			Tree tree = Tree(current_game);
			Node* root = &tree.root;
			root->state.player = opponent;	//opponent made previous move
			int num_iterations = 0;

			//do this for a number of iterations / amount of time
			while (num_iterations < MAX_ITERATIONS) {
																					//MCTS Phases

				Node* promising_node = selectNode(root);							//Selection

				//if promising node doesn't end game -> expand
				if (promising_node->state.game.checkStatus() == Game::IN_PROGRESS)
					expandNode(promising_node);										//Expansion

				Node* node_to_explore = promising_node;
				if (promising_node->children.size() > 0) {
					node_to_explore = promising_node->getRandomChildNode();
				}

				int rollout_result = rollout(node_to_explore, opponent);			//Rollout					

				backPropagation(node_to_explore, rollout_result);					//Backpropagation

				num_iterations++;
			}

			Node* winner_node = root->getChildWithMaxScore();		//pick best move for this round
			//winner_node->state.game.print();						//show board
			results[tree_num] = winner_node->state.game;			//save game / board
		}

		//Find best move by creating multiple trees in parallel and merging all results
		void findNextMoveWithRootParallelization(int player) {
			for (int i = 0; i < NUM_TREES; i++) {
				schedule([=]() {findNextMove(player, i); });
			}
			continuation([=]() {similarityVote();});
		}

		//Use simplified similarity vote to choose next move after parallelized search
		void similarityVote() {
			std::cout << "Similarity: ";
			std::unordered_map<Game, int, Game::GameHasher> hash;			//hash into map to efficiently find most frequent element
			for (int i = 0; i < NUM_TREES; i++) {
				hash[results[i]]++;											//add votes from trees to their respective chosen moves 
			}
			int max_count = 0;
			Game res;
			for (auto& ele : hash) {
				if (ele.second > max_count) {
					res = ele.first;
					max_count = ele.second;
				}
			}
			std::cout << max_count << " trees voted for this move" << std::endl;
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
		std::cout << "Player " << player << std::endl;
		schedule([]() {mcts.findNextMoveWithRootParallelization(player); });			//find move using MCTS with root parallelization

		continuation([=]() {
			mcts.getCurrentGame().print();
			std::cout << "Number of moves: " << n + 1 << std::endl << std::endl;
			if (mcts.getCurrentGame().checkStatus() == -1 && n < total_moves) {			//game is not over
				player = 3 - player;													//toggle player
				loopForEachRound(n + 1);												//next round
			}
		});	
	}

	//Test MCTS
	void test() {

		std::cout << "Starting MCTS Test" << std::endl;	

		schedule([]() {loopForEachRound(0); });
										
		continuation([]() {
			int win_status = mcts.getCurrentGame().checkStatus();
			std::cout << "Status: " << win_status << std::endl;
			std::cout << "Ending MCTS Test" << std::endl;
		});
	}
}