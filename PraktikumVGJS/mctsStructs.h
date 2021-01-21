#pragma once

#include "mctsGame.h"

//A struct carrying information relevant to the MCTS algorithm - state of the game at a specific point
struct State {
	Game game;
	uint32_t player = 0;		//the player who made the move that resulted in this state
	uint32_t num_visits = 0;
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