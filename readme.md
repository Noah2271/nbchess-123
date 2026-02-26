## Noah Billedo CMPM 123 - Windows PC

## Chess Implementation

### Chess.cpp
- Contains implementation of the 8x8 game board and basic movement without legality checks. Currently uses iterative generation for moves instead of bitboards. The board indexing is organized from the bottom left (0,0) to the top right (7,7). Movement is implemented via generatePiece() functions that utilize piece specific offsets that are passed into a a function template in the header file that then processes the moves by applying the move logic, mostly for pawns, and checking if the space is either empty or occupied by an enemy piece for capture. These moves are then stored into a vectorwhich is generated in the canBitMoveFromTo() function, and players are able to take the move if there is a move with a from-to index that aligns with the players desired move in the vector. There is no win/loss implementation yet.

### Chess.h
- Contains function definitions and the function template for calculateMoves() which takes an equation that applies offsets and calculates moves. This function is the function that adds the move to the vector and also contains the logic that allows pawns to perform their diagonal captures.