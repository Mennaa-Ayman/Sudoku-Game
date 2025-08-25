#include "../Include/SudokuBoard.hpp"
// --------------------------------------------Constructors-------------------------------------------- //

// Default constructor initializes a 9x9 board with zeros
SudokuBoard::SudokuBoard():board(9,std::vector<int>(9,0)){}

// Constructor to load board from file
SudokuBoard::SudokuBoard(const std::string& fname):board(9,std::vector<int>(9,0))
{
    std::ifstream infile(fname);
    std::string line;
    if(infile.is_open())
    {
        int row_num = 0, col_num = 0;
        // Read each line from the file
        while(std::getline(infile,line))
        {
            std::istringstream ss(line);
            std::string num;
            while (ss >> num){ 
                int number = stoi(num);
                board[row_num][col_num++] = number;
            }
            row_num++;
            col_num= 0;
        }
        infile.close();
        puzzleLoaded=true;
    }
     else    { throw WrongFileName("file failed to open"); }
}

// ------------------------------------------- Set & Get Board ------------------------------------------------ //
const std::vector<std::vector<int>>& SudokuBoard::getBoard() const{
    return board;
}

void  SudokuBoard::setBoard(const std::vector<std::vector<int>>& other){
    board = other;
}

// -------------------------------------------- Validation Methods -------------------------------------------- //

std::vector<int> SudokuBoard::getBox(int box_row_num , int box_col_num) const{ 
    std::vector<int> box;
    for(int i=0; i<3 ; i++){
        for(int j=0; j<3; j++)
            box.push_back(board[box_row_num*3+i][box_col_num*3+j]);
    }
    return box;
}
bool SudokuBoard::validBox(int row_num, int col_num, int num){
    int box_row_num = row_num/3;
    int box_col_num = col_num/3;
    std::vector<int> box = getBox(box_row_num, box_col_num);
    for(int i=0; i<box.size(); i++){
        if(box[i]==num) return false;
    }
    return true;
}

bool SudokuBoard::validRow(int row_num, int num){
    for(int j=0; j<9; j++){
        if(board[row_num][j]==num) return false;
    }
    return true;
}
bool SudokuBoard::validColumn (int col_num, int num){
    for(int i=0; i<9; i++){
        if(board[i][col_num]==num) return false;
    }
    return true;
}
// -------------------------------------------- Set Value -------------------------------------------- //
bool  SudokuBoard::setValue(int row_num, int col_num, int num, bool backtrace)
{
    row_num = row_num - 1;
    col_num = col_num - 1;
    if(board[row_num][col_num]!=0 && !backtrace) return false;
    if(num==0 || (validBox(row_num,col_num,num) && validColumn(col_num,num) && validRow(row_num,num)))
    {
        board[row_num][col_num] = num;
        return true;
    }
    return false;
}

int SudokuBoard:: emptyCount() const{
    int count=0;
    for (auto row : board){
        for(auto value: row){
            if(value==0) count++;
        }
    }
    return count;
}

// -------------------------------------------- Print Board -------------------------------------------- //
void SudokuBoard::printBoard() const{
    for(int i=0; i<9; i++){
        if(i%3==0 && i!=0) std::cout<<"---------------------\n";
        for(int j=0; j<9; j++){
            if(j%3==0 && j!=0) std::cout<<"| ";
            std::cout<<board[i][j]<<" ";
        }
        std::cout<<"\n";
    }
    std::cout<<"\n";
}

// -------------------------------------- Save ---------------------------------------------------------//
void SudokuBoard::save(const std::string& fname) const{
    std::ofstream outfile(fname);
    std::string line;
    if(outfile.is_open())
    {
        for(auto row : board)
        {
            for(auto val : row)
            {
                outfile << val << " ";
            }
            outfile << std::endl;
        }
        outfile.close();
    }
    else    { std::cout<<"file failed to open"<<std::endl; }
} 







