#include "../Include/SudokuGame.hpp"
#include <string>
#include <iostream>

// -------------------------------------------- Constructors -------------------------------------------- //
SudokuGame::SudokuGame():SudokuBoard(),solver(getBoard()) {
    int diffChoice = 0; 
    SudokuGenerator generator;
    SudokuGenerator::Difficulity diff;
    std::cout << "Choose Difficulty:" << std::endl;
    std::cout << "1) Easy\n2) Medium\n3) Hard\n" << std::endl;
    std::cout << "choice: ";
    std::cin >> diffChoice;
    switch(diffChoice){
        case 1: diff = SudokuGenerator::EASY; break;
        case 2: diff = SudokuGenerator::MEDIUM; break;
        case 3: diff = SudokuGenerator::HARD; break;
        default: 
            std::cout << "Invalid choice, defaulting to Easy." << std::endl;
             diff = SudokuGenerator::EASY;
            }
    generator.generate(diff);
    setBoard(generator.getBoard());
}

SudokuGame::SudokuGame(const std::string& fname):SudokuBoard(fname),solver(getBoard()){}

SudokuGame::SudokuGame(const std::vector<std::vector<int>>& other):SudokuBoard(other),solver(getBoard()){}

// -------------------------------------------- GAME METHODS -------------------------------------------- //
void SudokuGame::welcomeSudoku(){
    std::cout << "---- Welcome to Sudoku! ----" << std::endl;
    printBoard();
}


int SudokuGame::getChoice() const{
    std::cout << "1) Enter a move" << std::endl;
    std::cout << "2) Solve automatically" << std::endl;
    std::cout << "3) Load puzzle from file" << std::endl;
    std::cout << "4) Save current puzzle to file" << std::endl;
    std::cout<< "5) Exit" << std::endl;
    std::cout << "choice: ";

    int c; 
    std::cin >> c;
    return c;
}

void SudokuGame::startGame(){
    int choice = 0;
    std::string path;
    bool running = true;
    int row = 0, column = 0, value = 0; 
    welcomeSudoku();
    while(running){
        if(emptyCount()==0){
            std::cout<<"Congratulations! You solved the puzzle!" << std::endl;
            break;
        }
        choice = getChoice();
        try{
            switch(choice){
            case 1: //Enter a move 
                std::cout<< "Enter row (1-9), column (1-9), and value (1-9): " << std::endl;
                std::cin>> row>> column >> value;   
            
                if(row<1 || row>9) throw WrongInput("Invalid Input: Row value must be between 1 and 9.");
                if(column<1 || column>9) throw WrongInput("Invalid Input: Column must be between 1 and 9");    
                if(value<0 || value>9) throw WrongInput("Invalid Input: value must be between 1 and 9");
                if (setValue(row,column,value)){
                    std::cout<<"Move Accepted!" <<std::endl; 
                    printBoard();
                    } 
                    else throw SudokuRulesException("Invalid Move: The value breaks the sudoku Rules");
                break; 

            case 2: //Solve Automatically
                solver.setBoard(getBoard());

                if(solver.solve()){    
                    std::cout<<"Puzzle Solved Successfully!"<<std::endl;
                    solver.printBoard();
                    setBoard(solver.getBoard().getBoard());
                }
                else throw SudokuUnsolvable("The puzzle is unsolvable");
                break;

            case 3: //Load puzzle from file
                std::cout<<"Enter the file Path " << std::endl;
                std::cin>>path;
                setBoard(SudokuBoard(path).getBoard());  
                printBoard();        
                break;

            case 4: //Save current puzzle to a file
                std::cout<<"please enter the file path "<<std::endl;
                std::cin.clear();
                std::cin >> path;
                save(path);
                break;
                       
            case 5: // Exit 
                std::cout<<"Game is now closing!!"<<std::endl;
                running = false;
                break;   

            default:
                std::cin.clear();                 // Clear error flags
                std::cin.ignore(10000, '\n');    // Discard the rest of the input line
                throw WrongInput("Wrong input!");
                break;
            } 
        }
        catch(const SudokuException&e){
            std::cerr<<std::endl<<e.what()<<std::endl<<std::endl;
        }
    }
}

