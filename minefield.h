/*
    Simple terminal based minesweeper game - MineField class file
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

#ifndef MINE_FIELD_H
#define MINE_FIELD_H
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <iomanip>

//each tile is one byte
struct Tile{
    uint8_t adjacentMines : 4;
    uint8_t isMine : 1;
    uint8_t isRevealed : 1;
    uint8_t isFlagged : 1;
    uint8_t reserved : 1;
};

class MineField{
    public:
        MineField(unsigned int w, unsigned int h, unsigned int mines) : _width(w), _height(h), _mines(mines), _fieldSize(w*h), _numHiddenTiles(_fieldSize){
            _field = new Tile[_fieldSize];
            resetField();
            //generate label for x axis with indexes
            int val = 10, i = 0;
            for(; i < 5; i++){//find out the max power of 10 we have
                if((_width-1) / val){ val *= 10; }
                else{
                    if((_width-1) % (val) != 0){//if the max index is a multiple of 10, the current max power will be one too high
                        val /= 10;
                    }
                    break;
                }
            }
            for(; i > 0; --i){//splits the index up by powers of 10 and puts them each on a different line
                _xLabel += ' ';
                for(int j = 0; j < _width; ++j){
                    char c = (j/val);
                    _xLabel += c + 48;// +48 to turn num into a char
                    _xLabel += ' ';
                }  
                _xLabel += '\n';
                val /= 10;
            }
            _xLabel += ' ';
            for(int j = 0; j < _width; ++j){//needed for ones row
                char c = j % 10;
                _xLabel += c + 48;// +48 to turn num into a char
                _xLabel += '_';
            } 
            _xLabel.back() = '\n';//currently has an '_' at end
        }
        //returns whether the coordinate is in bounds
        bool coordInField(unsigned int x, unsigned int y) const { return (x < _width && y < _height); }
        void randomizeMineLocations(){
            resetField();
            for(unsigned int i = 0; i < _mines; ++i){
                unsigned int xi = 0;
                unsigned int yi = 0;
                do{
                    xi = rand() % _width;
                    yi = rand() % _height;
                }while(_getTile(xi, yi)->isMine);
                _getTile(xi, yi)->isMine = 1;
                //add 1 to neighbours' adjacentMines field
                _forNeighbours(xi, yi, [](Tile *t){ if(!t->isMine){ t->adjacentMines += 1; } });
            }
        }
        void flagTile(unsigned int x, unsigned int y){
            if(coordInField(x, y)){
                if(!_getTile(x, y)->isRevealed){ _getTile(x, y)->isFlagged = 1; }
            }
        }
        void unFlagTile(unsigned int x, unsigned int y){
            if(coordInField(x, y)){
                _getTile(x, y)->isFlagged = 0;
            }
        }
        bool isFlagged(unsigned int x, unsigned int y) const {
            if(coordInField(x, y)){
                return _getTile(x, y)->isFlagged;
            }
            return false;
        }
        void revealTile(unsigned int x, unsigned int y){
            if(coordInField(x, y)){
                Tile *t = _getTile(x, y);
                if(t->isRevealed){ return; }
                t->isRevealed = 1;
                --_numHiddenTiles;
                if(t->isMine){ _revealedMine = 1; }
                //if theres's no mines around then we reveal all the neighbours, 
                //and if the neighbour is 0 all of its neighbours get revealed and so on
                else if(t->adjacentMines == 0){ 
                    _forNeighbours(x, y, [&](Tile *t, unsigned int x, unsigned int y){
                        revealTile(x, y);
                    });
                }
            }
        }
        bool isRevealed(unsigned int x, unsigned int y) const {
            if(coordInField(x, y)){
                return _getTile(x, y)->isRevealed;
            }
            return false;
        }
        bool mineHit() const { return _revealedMine; }
        uint8_t numFlaggedNeighbours(unsigned int x, unsigned int y){
            uint8_t c = 0;
            _forNeighbours(x, y, [&](Tile *t){ if(t->isFlagged){ ++c; } });
            return c;
        }
        unsigned int numHiddenTiles() const { return _numHiddenTiles; }
        unsigned int numMines() const { return _mines; }
        //if tile(x, y) is already revealed and it has the same number of adjacent mines as it does flags 
        //then it reveals all the other neighbours that are not flagged
        void autoReveal(unsigned int x, unsigned int y){
            if(coordInField(x, y)){
                Tile *t = _getTile(x, y);
                if(numFlaggedNeighbours(x, y) == t->adjacentMines){
                    _forNeighbours(x, y, [&](Tile *ti, unsigned int xi, unsigned int yi){ 
                        if(!ti->isFlagged){ 
                            revealTile(xi, yi);
                        }
                    });
                }
            }
        }
        void print() const {
            //std::cout << _numHiddenTiles << '\n'; 
            std::cout << _xLabel;
            //std::cout << ' ' << std::setfill('_') << std::setw(_width*2) << "\n";
            for(int i = 0; i < _height; ++i){
                std::cout << '|';
                for(int j = 0; j < _width; ++j){
                    Tile *t = _getTile(j, i);
                    if(t->isFlagged){ std::cout << '.'; }
                    else if(!t->isRevealed){ std::cout << '#'; }
                    else if(t->isMine){ std::cout << (char)169; }
                    else if(t->adjacentMines == 0){ std::cout << ' '; }
                    else { std::cout << (int)t->adjacentMines; }
                    std::cout << ' ';
                }
                std::cout << "\b| " << i << '\n';
            }
            std::cout << ' ' << std::setfill('^') << std::setw(_width*2) << "\n" << std::flush;
        }

        void revealAll(){
            for(int i = 0; i < _fieldSize; ++i){
                _field[i].isRevealed = 1;
                if(!_field[i].isMine){ _field[i].isFlagged = 0; }
            }
            _revealedMine = 1;
        }

        void resetField(){ 
            memset(_field, 0, _fieldSize); 
            _numHiddenTiles = _fieldSize;
            _revealedMine = 0;
        }
        ~MineField(){
            if(_field != nullptr){ delete[] _field; }
        }
    private:
        bool _revealedMine = 0;//whether a mine has been revealed
        unsigned int _width, _height, _mines, _fieldSize, _numHiddenTiles;
        std::string _xLabel{};
        Tile *_field = nullptr;
        Tile *_getTile(unsigned int x, unsigned int y) const {
            x += y * _width;
            return _field + x; 
        }
        Tile *_getTileWithCheck(unsigned int x, unsigned int y) const {
            if(!coordInField(x, y)){ return nullptr; }
            return _getTile(x, y);
        }
        //passes every neighbour of (x,y) to the func
        void _forNeighbours(unsigned int x, unsigned int y, std::function<void(Tile*, unsigned int x, unsigned int y)> func){
            unsigned int tx = x-1, ty = y-1;
            Tile *t = nullptr;
            //top and bottom row
            for(int j = 0; j < 3; j += 2){
                for(int k = 0; k < 3; ++k){
                    t = _getTileWithCheck(tx+k, ty+j);
                    if(t != nullptr){ func(t, tx+k, ty+j); }
                }
            }
            //left
            t = _getTileWithCheck(x-1, y);
            if(t != nullptr){ func(t, x-1, y); }  
            //right
            t = _getTileWithCheck(x+1, y);
            if(t != nullptr){ func(t, x+1, y); }  
        }
        void _forNeighbours(unsigned int x, unsigned int y, std::function<void(Tile*)> func){
            _forNeighbours(x, y, [&](Tile *t, unsigned int x, unsigned int y){ func(t); });
        }
};

#endif //MINE_FIELD_H