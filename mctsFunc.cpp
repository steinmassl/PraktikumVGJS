#include <iostream>
#include <math.h>
#include <vector>

namespace mctsFunc {

	static constexpr int MAX_ITERATIONS = 1000;


	struct Tree {
		Node* root = nullptr;
	};


	struct Game {
		//game specific code here
		int** game_values;
		static constexpr int DEFAULT_BOARD_SIZE = 3;
		static constexpr int IN_PROGRESS = -1;
		static constexpr int DRAW = 0;
		static constexpr int P1 = 1;
		static constexpr int P2 = 2;

		void performMove(int player, std::pair<int, int> pos) {
			game_values[pos.first][pos.second] = player;
		}

		int checkStatus() {
			//todo
		}
	};

	struct Node {

		State state;
		Node* parent = nullptr;
		std::vector<Node*> children;

		Node(State state, Node* parent = nullptr) : state(state), parent(parent) {};

		//Calculate UCT value of current Node
		double calculateUCT() {
			if (state.num_visits == 0)			
				return INFINITY;		//nodes that haven't been visited have initial UCT of infinity
			return (state.score / state.num_visits) + 1.41 * sqrt(log(parent->state.num_visits) / state.num_visits);		//UCT calculation
		}

		//Find the child node of current node with the largest UCT value
		Node* findBestChildNode() {
			double largest_score = 0;
			Node* best_node;
			for (auto child : children) {
				double child_UCT = child->calculateUCT();
				if (child_UCT > largest_score) {
					largest_score = child_UCT;
					best_node = child;
				}
			}
			return best_node;
		}

		//Select a random child node from all children of this node
		Node* getRandomChildNode() {
			return children.at(rand() % children.size());
		}

		//Select the child with the highest score from the current node
		Node* getChildWithMaxScore() {
			//todo
		}


	};



	struct State {
		Game* game = nullptr;
		unsigned int player = 0;
		unsigned int num_visits = 0;
		double score = 0;


		void randomPlay() {

		}

		std::vector<State> getAllPossibleStates() {

		}

		void togglePlayer() {

		}
	};











	

	

	//use this class/function to test algorithm
	class MonteCarloTreeSearch {
		static constexpr int WIN_SCORE = 10;
		int opponent;

		Game* findNextMove(Game* game, int player) {

			opponent = 3 - player;
			Tree tree;
			Node* root = tree.root;
			root->state.game = game;
			int num_iterations = 0;

			while (num_iterations <= MAX_ITERATIONS) {
				//MCTS Phases
				Node* promising_node = selectNode(root);
				if (promising_node->state.game->checkStatus() == Game::IN_PROGRESS)
					expandNode(promising_node);
				Node* node_to_explore = promising_node;
				if (promising_node->children.size() > 0) {
					node_to_explore = promising_node->getRandomChildNode();
				}
				int rollout_result = rollout(node_to_explore);
				backPropagation(node_to_explore, rollout_result);
			}
			Node* winner_node = root->getChildWithMaxScore();
			tree.root = winner_node;
			return winner_node->state.game;
		}


	

		//Selection Phase - find suitable node to expand
		Node* selectNode(Node* root) {
			Node* current = root;
			while (current->children.size() != 0)
				current = current->findBestChildNode();
			return current;
		}

		//Expansion Phase - expand node adding a child for every possible action
		void expandNode(Node* node) {
			std::vector<State> possibleStates = node->state.getAllPossibleStates();
			for (State state : possibleStates) {
				Node* new_node = new Node(state, node);
				node->children.push_back(new_node);			//initialize children vector with amount of possible states
			}
		}

		//Rollout Phase - simulate rest of the game from a specific node
		int rollout(Node* node) {
			Node current_node = *node;
			State current_state = current_node.state;
			int game_status = current_state.game->checkStatus();
			if (game_status == opponent) {
				current_node.parent->state.score = 0;
				return game_status;
			}
			while (game_status == Game::IN_PROGRESS) {
				current_state.togglePlayer();
				current_state.randomPlay();
				game_status = current_state.game->checkStatus();
			}
			return game_status;
		}

		//Backpropagation Phase - update nodes according to rollout result
		void backPropagation(Node* nodeToExplore, int player) {
			Node* current_node = nodeToExplore;
			while (current_node != nullptr) {
				current_node->state.num_visits++;
				if (current_node->state.player == player)
					current_node->state.score += WIN_SCORE;
				current_node = current_node->parent;
			}
		}


	};













}