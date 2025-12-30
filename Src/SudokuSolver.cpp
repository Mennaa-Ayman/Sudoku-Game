#include "../Include/SudokuSolver.hpp"

bool SudokuSolver::solve() {
    for (int i = 1 ; i <= 9 ; i++) {
        for (int j = 1 ; j <= 9 ; j++) {
            if (board.getValue(i,j) == 0) {
                for (int num = 1; num <= 9; num++) {
                    if (board.setValue(i,j,num)) {
                        if (solve()) return true;
                        board.setValue(i,j,0,1); // backtrack
                    }
                }
                return false; // only if no valid number was found for this empty cell
            }
        }
    }
    return true; // board is fully solved
}

void SudokuSolver::setBoard(const std::vector<std::vector<int>>& boardObject){
   board.setBoard(boardObject);
}
const SudokuBoard& SudokuSolver:: getBoard() const { return board; }

