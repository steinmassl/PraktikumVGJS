#ifndef GAME_H
#define GAME_H

#include <iostream>
#include <vector>
#include <cstring>

	//Tic Tac Toe game to test MCTS
	class Game {
	public:
		static constexpr int DEFAULT_BOARD_SIZE = 5;			//Board Dimensions
		static constexpr int IN_PROGRESS = -1;
		static constexpr int DRAW = 0;
		static constexpr int P1 = 1;
		static constexpr int P2 = 2;

		short game_values[DEFAULT_BOARD_SIZE][DEFAULT_BOARD_SIZE] = {};			//initialize the game board with zeroes
		short num_empty_positions = DEFAULT_BOARD_SIZE * DEFAULT_BOARD_SIZE;

		//Board Position
		struct Pos {
			short first = 0;
			short second = 0;

			Pos() {}

			Pos(short first, short second) : first(first), second(second) {}
		};

		//Compare two games - necessary for std::unordered_map
		bool operator==(const Game& other) const {
			return !std::memcmp(game_values, other.game_values, sizeof(short) * DEFAULT_BOARD_SIZE * DEFAULT_BOARD_SIZE);
		}

		//Put player number into selected field marking it for this player
		void performMove(int player, Pos pos) {
			game_values[pos.first][pos.second] = player;
			num_empty_positions--;
		}

		//Get a vector of coordinates with positions on the game board that have not yet been claimed by a player
		std::vector<Pos> getEmptyPositions() {
			std::vector<Pos> empty_positions;
			empty_positions.reserve(num_empty_positions);
			for (int i = 0; i < DEFAULT_BOARD_SIZE; i++) {
				for (int j = 0; j < DEFAULT_BOARD_SIZE; j++) {
					if (game_values[i][j] == 0)
						empty_positions.emplace_back(Pos(i, j));
				}
			}
			return empty_positions;
		}

		//Check if there is an alignment that would win the game
		int checkForWin(int* row) {
			bool aligned = true;
			int current = row[0];
			for (int i = 1; i < DEFAULT_BOARD_SIZE; i++) {
				if (current != row[i] || current == 0) {
					aligned = false;
					break;
				}
				current = row[i];
			}
			if (aligned)
				return current;
			else
				return 0;
		}

		//Check whether a player has won the game or a draw occured
		int checkStatus() {
			int max_index = DEFAULT_BOARD_SIZE - 1;
			int diag_1[DEFAULT_BOARD_SIZE];
			int diag_2[DEFAULT_BOARD_SIZE];

			for (int i = 0; i < DEFAULT_BOARD_SIZE; i++) {
				int line[DEFAULT_BOARD_SIZE];
				for (int j = 0; j < DEFAULT_BOARD_SIZE; j++) {
					line[j] = game_values[i][j];
				}

				int row_win = checkForWin(line);						//check rows
				if (row_win != 0)
					return row_win;

				for (int j = 0; j < DEFAULT_BOARD_SIZE; j++) {
					line[j] = game_values[j][i];
				}

				int col_win = checkForWin(line);						//check columns
				if (col_win != 0)
					return col_win;

				diag_1[i] = game_values[i][i];
				diag_2[i] = game_values[max_index - i][i];
			}

			int diag_1_win = checkForWin(diag_1);						//check both diagonals
			if (diag_1_win != 0)
				return diag_1_win;

			int diag_2_win = checkForWin(diag_2);
			if (diag_2_win != 0)
				return diag_2_win;

			if (num_empty_positions > 0)
				return IN_PROGRESS;									//no win - game continues
			else
				return DRAW;										//no win and nothing to do -> Draw
		}

		//Print the current status of the board
		void print() {
			for (int i = 0; i < DEFAULT_BOARD_SIZE; i++) {
				for (int j = 0; j < DEFAULT_BOARD_SIZE; j++) {
					std::cout << game_values[i][j] << " ";
				}
				std::cout << std::endl;
			}
		}

		//Hash the values of the game/board - necessary for std::unordered_map
		struct GameHasher {
			std::size_t operator()(const Game& game) const {
				using std::size_t;
				using std::hash;

				size_t res = 0;
				for (int i = 0; i < DEFAULT_BOARD_SIZE; i++) {
					for (int j = 0; j < DEFAULT_BOARD_SIZE; j++) {
						res = res * 314159 + game.game_values[i][j];
					}
				}
				return res;
			}
		};
	};

#endif