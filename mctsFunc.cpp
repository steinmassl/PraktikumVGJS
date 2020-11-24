#include <iostream>
#include <math.h>
#include <vector>

namespace mctsFunc {

	//Tic Tac Toe game to test MCTS
	struct Game {
		static constexpr int DEFAULT_BOARD_SIZE = 10;
		static constexpr int IN_PROGRESS = -1;
		static constexpr int DRAW = 0;
		static constexpr int P1 = 1;
		static constexpr int P2 = 2;

		short game_values[DEFAULT_BOARD_SIZE][DEFAULT_BOARD_SIZE] = {};		//initialize the game board with zeroes

		//put player number into selected field marking it for this player
		void performMove(int player, std::pair<int, int> pos) {
			game_values[pos.first][pos.second] = player;
		}

		// get a vector of coordinates with positions on the game board that have not yet been claimed by a player
		std::vector<std::pair<int, int>> getEmptyPositions() {
			std::vector<std::pair<int, int>> empty_positions;
			for (int i = 0; i < DEFAULT_BOARD_SIZE; i++) {
				for (int j = 0; j < DEFAULT_BOARD_SIZE; j++) {
					if (game_values[i][j] == 0)
						empty_positions.push_back(std::make_pair(i, j));
				}
			}
			return empty_positions;
		}

		//check if there is an alignment that would win the game
		int checkForWin(int* row) {
			bool is_equal = true;
			int previous = row[0];
			for (int i = 0; i < DEFAULT_BOARD_SIZE; i++) {
				if (previous != row[i]) {
					is_equal = false;
					break;
				}
				previous = row[i];
			}
			if (is_equal && (previous > 0))
				return previous;
			else
				return 0;
		}

		//check whether a player has won the game or a draw occured
		int checkStatus() {
			int max_index = DEFAULT_BOARD_SIZE - 1;
			int diag_1[DEFAULT_BOARD_SIZE] = {};
			int diag_2[DEFAULT_BOARD_SIZE] = {};

			for (int i = 0; i < DEFAULT_BOARD_SIZE; i++) {
				int row[DEFAULT_BOARD_SIZE] = {};
				for (int j = 0; j < DEFAULT_BOARD_SIZE; j++) {
					row[j] = game_values[i][j];
				}
				int col[DEFAULT_BOARD_SIZE] = {};
				for (int j = 0; j < DEFAULT_BOARD_SIZE; j++) {
					col[j] = game_values[j][i];
				}

				int row_win = checkForWin(row);						//check rows
				if (row_win != 0)
					return row_win;

				int col_win = checkForWin(col);						//check columns
				if (col_win!= 0)
					return col_win;

				diag_1[i] = game_values[i][i];
				diag_2[i] = game_values[max_index - i][i];
			}

			int diag_1_win = checkForWin(diag_1);					//check both diagonals
			if (diag_1_win != 0)
				return diag_1_win;

			int diag_2_win = checkForWin(diag_2);
			if (diag_2_win != 0)
				return diag_2_win;

			if (getEmptyPositions().size() > 0)
				return IN_PROGRESS;									//no win - game continues
			else
				return DRAW;										//no win and nothing to do -> Draw
		}

		//print the current status of the board
		void print() {
			for (int i = 0; i < DEFAULT_BOARD_SIZE; i++) {
				for (int j = 0; j < DEFAULT_BOARD_SIZE; j++) {
					std::cout << game_values[i][j] << " ";
				}
				std::cout << std::endl;
			}
		}
	};

	//a struct carrying information relevant to the MCTS algorithm - state of the game at a specific point
	struct State {
		Game game;
		unsigned int player = 0;		//the player who made the move that resulted in this state
		unsigned int num_visits = 0;
		double score = 0;				//shows how relevant this path is for the algorithm

		State() {}

		State(Game game) : game(game) {}

		//performs a random move from all available moves
		void randomPlay() {
			std::vector<std::pair<int, int>> available_positions = game.getEmptyPositions();
			game.performMove(player, available_positions.at(rand() % available_positions.size()));
		}

		//get a list of all possible states that can be achieved from the current state
		std::vector<State> getAllPossibleStates() {
			std::vector<std::pair<int, int>> available_positions = game.getEmptyPositions();
			std::vector<State> possible_states;
			for (auto& pos : available_positions) {
				State new_state = State(game);
				new_state.player = 3 - player;
				new_state.game.performMove(new_state.player, pos);
				possible_states.push_back(new_state);
			}
			return possible_states;
		}

		//switches P1 and P2
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

		
		~Node() {
			for (int i = 0; i < children.size(); i++) {
				delete children[i];
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
			double largest_UCT = std::numeric_limits<double>::lowest();
			Node* best_node = nullptr;
			for (auto child : children) {
				double child_UCT = child->calculateUCT();
				if (child_UCT > largest_UCT) {
					largest_UCT = child_UCT;
					best_node = child;
				}
			}
			return best_node;
		}

		//Select the child node with the highest overall score 
		Node* getChildWithMaxScore() {
			double largest_score = std::numeric_limits<double>::lowest();
			Node* best_node = nullptr;
			for (auto child : children) {
				if (child->state.score > largest_score) {
					largest_score = child->state.score;
					best_node = child;
				}
			}
			return best_node;
		}
	};

	//tree struct that provides a starting node
	struct Tree {
		Node root;

		Tree(Game game) {
			root = Node(game);
		}
	};












	

	

	//use this class/function to test algorithm
	class MonteCarloTreeSearch {
		static constexpr int WIN_SCORE = 10;
		static constexpr int DRAW_SCORE = 5;
		static constexpr int MAX_ITERATIONS = 1000;						//number of iterations of algorithm
	
		//Selection Phase - find suitable node to expand
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
			for (State& state : possibleStates) {
				Node* new_node = new Node(state, node);
				node->children.push_back(new_node);			//initialize children vector with amount of possible states
			}
		}

		//Rollout Phase - simulate rest of the game from a specific node
		int rollout(Node* node, int& opponent) {
			Node* current_node = node;
			State current_state = current_node->state;
			int game_status = current_state.game.checkStatus();
			if (game_status == opponent) {
				current_node->parent->state.score = 0;
				return game_status;
			}
			while (game_status == Game::IN_PROGRESS) {
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
				if (current_node->state.player == rollout_result)
					current_node->state.score += WIN_SCORE;
				else if (rollout_result == current_node->state.game.DRAW)
					current_node->state.score += DRAW_SCORE;
				current_node = current_node->parent;
			}
		}
	
	public:
		Game findNextMove(Game game, int player) {

			int opponent = 3 - player;		//opponent of current player
			Tree tree = Tree(game);
			Node* root = &tree.root;
			root->state.player = opponent;	//opponent made last move
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
			winner_node->state.game.print();						//show board
			return winner_node->state.game;							//return game / board
		}
	};




	//test MCTS
	void test() {

		std::cout << "Starting MCTS Test" << std::endl;
		Game game = Game();
		MonteCarloTreeSearch mcts;
		int player = Game::P1;
		int total_moves = Game::DEFAULT_BOARD_SIZE * Game::DEFAULT_BOARD_SIZE;

		for (int i = 0; i < total_moves; i++) {
			game = mcts.findNextMove(game, player);
			std::cout << "Number of moves: " << i+1 << std::endl;
			if (game.checkStatus() != -1) {
				break;
			}
			player = 3 - player;
		}
		int win_status = game.checkStatus();
		std::cout << "Status: " << win_status << std::endl;
		std::cout << "Ending MCTS Test" << std::endl;
	}
}