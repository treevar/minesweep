/*
    Simple terminal based minesweeper game - main file
    Copyright (C) 2023  treevar

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <ctime>
#include "minefield.h"
#include <string>
#include <chrono>
//#include <ncurses.h>

std::string promptForStr(const std::string &prompt){
    std::cout << prompt << "\n > ";
    std::string str{};
    std::getline(std::cin, str);
    return str;
}

int promptForInt(const std::string &prompt){
    std::string str = promptForStr(prompt);
    while(1){
        try{
            return std::stoi(str);
        }catch(...){
            str = promptForStr(prompt);
        }
    }
}

int getX(){
    return promptForInt("Enter X Coord");
}

int getY(){
    return promptForInt("Enter Y Coord");
}

void printTime(std::chrono::time_point<std::chrono::system_clock> &start){
    std::cout << std::fixed << std::setprecision(3) << std::chrono::duration<double>(std::chrono::system_clock::now() - start).count()  << "s";
}

void clearScreen(){ 
    #if (defined (_WIN32) || defined (_WIN64))
        system("cls");
    #else
        system("clear"); 
    #endif
}

int main(int argc, char **argv){
    int w = 10, h = 10, m = 5;
    clearScreen();
    if(argc == 4){
        w = atoi(argv[1]);
        h = atoi(argv[2]);
        m = atoi(argv[3]);
    }else{
        w = std::abs(promptForInt("Enter the width"));
        h = std::abs(promptForInt("Enter the height"));
        m = std::abs(promptForInt("Enter the number of mines"));
    }
    srand(time(nullptr));
    MineField mf(w, h, m);
    mf.randomizeMineLocations();
    bool run = 1, gameOver = 0;
    std::string str{};
    unsigned int x = 0;
    unsigned int y = 0;
    std::chrono::time_point<std::chrono::system_clock> startTime = std::chrono::system_clock::now();
    while(run){
        mf.print();
        str = promptForStr("(f)lag, (u)nflag, (r)eveal, (q)uit");
        if(str[0] == 'q'){ run = 0; }
        else{
            x = getX();
            y = getY();
            if(str[0] == 'f'){
                mf.flagTile(x, y);
                clearScreen();
            }
            else if(str[0] == 'u'){
                mf.unFlagTile(x, y);
                clearScreen();
            }
            else if(str[0] == 'r'){
                if(mf.isRevealed(x, y)){
                    mf.autoReveal(x, y);
                    //if(mf.mineHit()){ run = 0; }
                }
                else if(!mf.isFlagged(x, y)){ mf.revealTile(x, y); }
                if(mf.mineHit()){
                    str = "\tLOSER ";
                    gameOver = 1;
                }
                else if(mf.numHiddenTiles() == mf.numMines()){
                    str = "\tWINNER ";
                    gameOver = 1;
                }
                if(gameOver){
                    clearScreen();
                    std::cout << str;
                    printTime(startTime);
                    std::cout << '\n';
                    mf.revealAll();
                    mf.print();
                    mf.randomizeMineLocations();
                    startTime = std::chrono::system_clock::now();
                    gameOver = 0;
                }
                else{
                    clearScreen();
                }
            }
        }
    }
    return 0;
}
