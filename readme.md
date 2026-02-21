## Noah Billedo CMPM 123 - Windows PC

## Chess Implementation

### Chess.cpp
- Contains implementation of the 8x8 game board and pawn moves. Currently uses iterative generation for moves instead of bitboards. Will likely change everything to bitboards after understanding how everything works in the iteration way first.
- Note that the state string reads from the bottom left of the board and FEN is translated and pieces placed on the board starting from the top left.