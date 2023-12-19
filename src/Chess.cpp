#include <iostream>
#include <cmath>
#include <vector>
#include <SDL/SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include "Chess.h"
#include "Board.h"
using namespace std;
class Board;

Chess::Chess(int NumPlayers, bool PlayerColor) : 
numPlayers(NumPlayers), playerColor(PlayerColor) {
    board = new Board();
    mouseX = 0;
    mouseY = 0;
    isWhiteTurn = true;
    isPieceSelected = false;
    selectedPiece = 0;
    selectedRow = -1;
    selectedCol = -1;
    togglePerspective = false; 
}

void Chess::presentState(SDL_Renderer* renderer) {
    //clear previous textures and copy board texture
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, textureBoard, NULL, NULL);
    SDL_Rect Rect;

    //copy piece textures onto occupied cells
    for(int row = 0; row < BOARD_SIZE; row++) {
        for(int col = 0; col < BOARD_SIZE; col++) {
            int Row = togglePerspective ? row : (BOARD_SIZE - 1 - row);
            int Col = togglePerspective ? col : (BOARD_SIZE - 1 - col);
            //highlight previous move start & end square
            if(prevStart.first == row && prevStart.second == col ||
               prevEnd.first == row && prevEnd.second == col) {
                SDL_Rect square;
                square.x = Col * SQUARE_SIZE;
                square.y = Row * SQUARE_SIZE;
                square.w = SQUARE_SIZE;
                square.h = SQUARE_SIZE;
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(renderer, 160, 210, 255, 150);
                SDL_RenderFillRect(renderer, &square);
            }
            int p = board->getPiece(row * 8 + col); if(p == 0) continue;
            Rect = {Col * SQUARE_SIZE, Row * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
            if(p > 0) SDL_RenderCopy(renderer, textureWhite[p - 1], NULL, &Rect);
            else SDL_RenderCopy(renderer, textureBlack[abs(p) - 1], NULL, &Rect);
        }
    }
    if(isPieceSelected) {
        U64 moves = board->getValidMoves(selectedRow * 8 + selectedCol); 
        SDL_SetRenderDrawColor(renderer, 40, 200, 190, 50);
        for(int row = 0; row < 8; row++) {
            for(int col = 0; col < 8; col++) {
                U64 mask = 1ULL << row * 8 + col;
                if(moves & mask) {
                    int displayRow = togglePerspective ? row : (BOARD_SIZE - 1 - row);
                    int displayCol = togglePerspective ? col : (BOARD_SIZE - 1 - col);
                    Rect = {displayCol * SQUARE_SIZE, displayRow * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
                    SDL_RenderCopy(renderer, textureLight, NULL, &Rect);
                }
            } 
        }
        int hideRow = togglePerspective ? selectedRow : (BOARD_SIZE - 1 - selectedRow);
        int hideCol = togglePerspective ? selectedCol : (BOARD_SIZE - 1 - selectedCol);
        Rect = {hideCol * SQUARE_SIZE, hideRow * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
        if((hideRow + hideCol) % 2 == 0)
                SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255);
        else SDL_SetRenderDrawColor(renderer, 30, 135, 155, 255);
        SDL_RenderFillRect(renderer, &Rect);
        int displayRow = togglePerspective ? mouseX : SQUARE_SIZE * (BOARD_SIZE) - mouseX;
        int displayCol = togglePerspective ? mouseY : SQUARE_SIZE * (BOARD_SIZE) - mouseY;
        Rect = {displayRow - SQUARE_SIZE/2, displayCol - SQUARE_SIZE/2, SQUARE_SIZE, SQUARE_SIZE};
        if(selectedPiece > 0) SDL_RenderCopy(renderer, textureWhite[selectedPiece - 1], NULL, &Rect);
        else SDL_RenderCopy(renderer, textureBlack[abs(selectedPiece) - 1], NULL, &Rect);
    }
    SDL_RenderPresent(renderer); 
}

bool Chess::isGameOver() {
    return 
    board->isCheckmate(1) || 
    board->isCheckmate(0) || 
    board->isStalemate();
}

void Chess::start() {
    //Adjust display size to monitor
    SDL_DisplayMode displayMode;
    SDL_GetDesktopDisplayMode(0, &displayMode);
    SQUARE_SIZE = displayMode.h / 8 - 30;

    //SDL Setup
    SDL_Window* window = SDL_CreateWindow(
        "Chess", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SQUARE_SIZE * BOARD_SIZE, SQUARE_SIZE * BOARD_SIZE,
        SDL_WINDOW_SHOWN);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED);

    //create chess board texture
    textureBoard = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
        SQUARE_SIZE * BOARD_SIZE, SQUARE_SIZE * BOARD_SIZE);
    SDL_SetRenderTarget(renderer, textureBoard);
    SDL_Rect squares[BOARD_SIZE][BOARD_SIZE];
    for(int row = 0; row < BOARD_SIZE; row++)
    {
        for(int col = 0; col < BOARD_SIZE; col++)
        {
            squares[row][col].x = col * SQUARE_SIZE;
            squares[row][col].y = row * SQUARE_SIZE;
            squares[row][col].w = SQUARE_SIZE;
            squares[row][col].h = SQUARE_SIZE;
        }
    }
    for(int row = 0; row < BOARD_SIZE; row++)
        for(int col = 0; col < BOARD_SIZE; col++) {
            if((row + col) % 2 == 0)
                SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255);
            else SDL_SetRenderDrawColor(renderer, 30, 135, 155, 255);
            SDL_RenderFillRect(renderer, &squares[row][col]);
        }
    SDL_SetRenderTarget(renderer, NULL);

    //load images
    SDL_Surface* image;
    image = IMG_Load("C:/Visual Studio Code/Chess/images/whiteKing.png");
    textureWhite.push_back(SDL_CreateTextureFromSurface(renderer, image));
    image = IMG_Load("C:/Visual Studio Code/Chess/images/whiteQueen.png");
    textureWhite.push_back(SDL_CreateTextureFromSurface(renderer, image));
    image = IMG_Load("C:/Visual Studio Code/Chess/images/whiteRook.png");
    textureWhite.push_back(SDL_CreateTextureFromSurface(renderer, image));
    image = IMG_Load("C:/Visual Studio Code/Chess/images/whiteBishop.png");
    textureWhite.push_back(SDL_CreateTextureFromSurface(renderer, image));
    image = IMG_Load("C:/Visual Studio Code/Chess/images/whiteKnight.png");
    textureWhite.push_back(SDL_CreateTextureFromSurface(renderer, image));
    image = IMG_Load("C:/Visual Studio Code/Chess/images/whitePawn.png");
    textureWhite.push_back(SDL_CreateTextureFromSurface(renderer, image));
    image = IMG_Load("C:/Visual Studio Code/Chess/images/blackKing.png");
    textureBlack.push_back(SDL_CreateTextureFromSurface(renderer, image));
    image = IMG_Load("C:/Visual Studio Code/Chess/images/blackQueen.png");
    textureBlack.push_back(SDL_CreateTextureFromSurface(renderer, image));
    image = IMG_Load("C:/Visual Studio Code/Chess/images/blackRook.png");
    textureBlack.push_back(SDL_CreateTextureFromSurface(renderer, image));
    image = IMG_Load("C:/Visual Studio Code/Chess/images/blackBishop.png");
    textureBlack.push_back(SDL_CreateTextureFromSurface(renderer, image));
    image = IMG_Load("C:/Visual Studio Code/Chess/images/blackKnight.png");
    textureBlack.push_back(SDL_CreateTextureFromSurface(renderer, image));
    image = IMG_Load("C:/Visual Studio Code/Chess/images/blackPawn.png");
    textureBlack.push_back(SDL_CreateTextureFromSurface(renderer, image));
    image = IMG_Load("C:/Visual Studio Code/Chess/images/PureBlack.png");
    textureLight = SDL_CreateTextureFromSurface(renderer, image);
    SDL_SetTextureAlphaMod(textureLight, 35);
    SDL_FreeSurface(image);

    //load SFX
    Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 4096);
    Mix_Chunk* soundPlace = Mix_LoadWAV("C:/Visual Studio Code/Chess/sounds/Place.wav");
    Mix_Chunk* soundCapture = Mix_LoadWAV("C:/Visual Studio Code/Chess/sounds/Capture.wav");
    Mix_Chunk* soundCastle = Mix_LoadWAV("C:/Visual Studio Code/Chess/sounds/Castle.wav");
    Mix_Chunk* soundInvalid = Mix_LoadWAV("C:/Visual Studio Code/Chess/sounds/Invalid.wav");
    Mix_Chunk* soundCheck = Mix_LoadWAV("C:/Visual Studio Code/Chess/sounds/Check.wav");
    Mix_Chunk* soundOver = Mix_LoadWAV("C:/Visual Studio Code/Chess/sounds/GameOver.wav");
    Mix_Chunk* soundBloop = Mix_LoadWAV("C:/Visual Studio Code/Chess/sounds/Bloop.wav");

    //initial
    presentState(renderer);
    bool quit = false;
    SDL_Event event;
    //loop start
    while(!quit) { 
        if(numPlayers == 0 || 
           numPlayers == 1 && (board->getMoveIndex() == board->getTotalMoves() - 1 &&
           (playerColor == 1 && !isWhiteTurn || playerColor == 0 && isWhiteTurn))) {
            pair<int, int> Move = board->getBestMove(isWhiteTurn);
            int squareStart = Move.first, squareEnd = Move.second;
            if(board->getPiece(squareEnd) != 0) 
                Mix_PlayChannel(-1, soundCapture, 0);
            if(board->getSpecialMoves(squareStart) & 
            1ULL << squareEnd)
                if(abs(board->getPiece(squareStart)) == 1) 
                    Mix_PlayChannel(-1, soundCastle, 0);
                else Mix_PlayChannel(-1, soundCapture, 0);
            Mix_PlayChannel(-1, soundPlace, 0);
            board->makeMove(squareStart, squareEnd);
            prevStart = {squareStart / 8, squareStart % 8};
            prevEnd = {squareEnd / 8, squareEnd % 8};
            isWhiteTurn = !isWhiteTurn;
            if(board->isCheck(0) || board->isCheck(1))
                Mix_PlayChannel(-1, soundCheck, 0);
            if(isGameOver()) {
                Mix_PlayChannel(-1, soundOver, 0);
                presentState(renderer);
                goto game_over;
            }
            presentState(renderer);
        }
        else {
            SDL_WaitEvent(&event);
            switch(event.type)
            {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_MOUSEBUTTONUP:
                    //left mouse button case
                    if(event.button.button == SDL_BUTTON_LEFT) {
                        int col = mouseX / SQUARE_SIZE;
                        int row = mouseY / SQUARE_SIZE;
                        int clickedPiece = board->getPiece(row * 8 + col);
                        if(isPieceSelected) {
                            int squareStart = selectedRow * 8 + selectedCol;
                            int squareEnd = row * 8 + col;
                            if(1ULL << squareEnd & 
                            board->getValidMoves(squareStart)) {
                                if(board->getPiece(squareEnd) != 0) 
                                    Mix_PlayChannel(-1, soundCapture, 0);
                                if(board->getSpecialMoves(squareStart) & 
                                1ULL << squareEnd)
                                    if(abs(board->getPiece(squareStart)) == 1) 
                                        Mix_PlayChannel(-1, soundCastle, 0);
                                    else Mix_PlayChannel(-1, soundCapture, 0);
                                Mix_PlayChannel(-1, soundPlace, 0);
                                board->makeMove(squareStart, squareEnd);
                                prevStart = {selectedRow, selectedCol};
                                prevEnd = {row, col};
                                isPieceSelected = false;
                                selectedPiece = 0;
                                selectedRow = -1;
                                selectedCol = -1;
                                if(board->isCheck(0) || board->isCheck(1))
                                    Mix_PlayChannel(-1, soundCheck, 0);
                                isWhiteTurn = !isWhiteTurn;
                                if(isGameOver()) {
                                    Mix_PlayChannel(-1, soundOver, 0);
                                    presentState(renderer);
                                    goto game_over;
                                }
                            }
                            else {
                                if(selectedRow != row || selectedCol != col)
                                Mix_PlayChannel(-1, soundInvalid, 0);
                                isPieceSelected = false;
                                selectedPiece = 0;
                                selectedRow = -1;
                                selectedCol = -1;
                            }
                        }
                        else {
                            if(clickedPiece != 0 &&
                            (clickedPiece > 0 && isWhiteTurn && 
                            (playerColor == 1 || numPlayers == 2)) ||
                            (clickedPiece < 0 && !isWhiteTurn && 
                            (playerColor == 0 || numPlayers == 2))) {
                                isPieceSelected = true;
                                selectedPiece = clickedPiece;
                                selectedRow = row;
                                selectedCol = col;
                            }
                        }
                        presentState(renderer);
                    }
                    //right mouse button case toggles perspective
                    else if(event.button.button == SDL_BUTTON_RIGHT) {
                        togglePerspective = !togglePerspective;
                        Mix_PlayChannel(-1, soundBloop, 0);
                        presentState(renderer);
                    }
                    break;
                case SDL_KEYDOWN:
                    if(event.key.keysym.sym == SDLK_DOWN) {
                        if(!isPieceSelected && board->undoMove()) {
                            presentState(renderer);
                            isWhiteTurn = !isWhiteTurn;
                            Mix_PlayChannel(-1, soundBloop, 0);
                        }
                    }
                    if(event.key.keysym.sym == SDLK_UP) {
                        if(!isPieceSelected && board->redoMove()) {
                            presentState(renderer);
                            isWhiteTurn = !isWhiteTurn;
                            Mix_PlayChannel(-1, soundBloop, 0);
                        }
                    }
                    break;
                case SDL_MOUSEMOTION: 
                    if(isPieceSelected) 
                        presentState(renderer);
                    //continuously track the mouse
                    if(togglePerspective) {
                        mouseX = event.motion.x;
                        mouseY = event.motion.y;}
                    else { //track invertedly if togglePerspective
                        mouseX = SQUARE_SIZE * (BOARD_SIZE) - event.motion.x;
                        mouseY = SQUARE_SIZE * (BOARD_SIZE) - event.motion.y;}
                    break;
            }
        }
    }

    game_over:
    while(!quit) {
        SDL_WaitEvent(&event);
        switch(event.type)
        {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_DOWN) {
                    if(!isPieceSelected && board->undoMove()) {
                        presentState(renderer);
                        isWhiteTurn = !isWhiteTurn;
                        Mix_PlayChannel(-1, soundBloop, 0);
                    }
                }
                if(event.key.keysym.sym == SDLK_UP) {
                    if(!isPieceSelected && board->redoMove()) {
                        presentState(renderer);
                        isWhiteTurn = !isWhiteTurn;
                        Mix_PlayChannel(-1, soundBloop, 0);
                    }
                }
                case SDL_MOUSEBUTTONUP:
                    //left mouse button case
                    if(event.button.button == SDL_BUTTON_RIGHT) {
                        togglePerspective = !togglePerspective;
                        Mix_PlayChannel(-1, soundBloop, 0);
                        presentState(renderer);
                    }
                    break;
        }
    }

    //Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_FreeChunk(soundPlace);
    Mix_FreeChunk(soundCapture);
    Mix_FreeChunk(soundCastle);
    Mix_FreeChunk(soundInvalid);
    Mix_FreeChunk(soundCheck);
    Mix_FreeChunk(soundOver);
    Mix_FreeChunk(soundBloop);
    Mix_CloseAudio();
    SDL_Quit();
}