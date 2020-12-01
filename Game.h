#include <iostream>
#include <vector>

	//Tic Tac Toe game to test MCTS
	class Game {
	public:
		static constexpr int DEFAULT_BOARD_SIZE = 5;
		static constexpr int IN_PROGRESS = -1;
		static constexpr int DRAW = 0;
		static constexpr int P1 = 1;
		static constexpr int P2 = 2;

		short game_values[DEFAULT_BOARD_SIZE][DEFAULT_BOARD_SIZE] = {};		//initialize the game board with zeroes

		//Compare two games - necessary for std::unordered_map
		bool operator==(const Game& other) const {
			return !memcmp(game_values, other.game_values, sizeof(short) * DEFAULT_BOARD_SIZE * DEFAULT_BOARD_SIZE);
		}

		//Put player number into selected field marking it for this player
		void performMove(int player, std::pair<int, int> pos) {
			game_values[pos.first][pos.second] = player;
		}

		//Get a vector of coordinates with positions on the game board that have not yet been claimed by a player
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

		//Check if there is an alignment that would win the game
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

		//Check whether a player has won the game or a draw occured
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
				if (col_win != 0)
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