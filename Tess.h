//tess.h
#ifndef TESSH
#define TESSH
#include <PalmOS.h>

typedef enum TessPrefID {
  Pref_aGameStruct = 0x7000,
  Pref_SquaresArray = 0x7001,
  Pref_MovesArray = 0x7002
} TessPreVersionNumber;

typedef enum DifficultyType {
  difficultyEasy = 1,
  difficultyIntermediate,
  difficultyMoreDifficult,
  difficultyAdvanced,
  difficultyImpossible
} DifficultyType;

#define defaultDifficulty difficultyEasy;

#define WHITE	0x00FFFFFF
#define RED	0x00FF0000
#define BLUE 	0x000000FF
#define YELLOW 	0x00FFFF00
#define GREEN 	0x0000CC00
#define PURPLE	0x00CC00CC
#define ORANGE	0x00FFCC00
#define GRAY	0x00999999
//00000001 = Has +
//00000010 = Has []
//00000100 = Has O
//Other combinations make sense (example 00000101 has [] and +)
//00001000 = OnBoard (True if sqaure can hold a piece)

//#define ATTRIBUTES_MASK 0x07
//#define GETATTRIBUTES(x) (x & ATTRIBUTES_MASK)
//#define ONBOARD_MASK 0x08
//#define ONBOARD(x) (x & ONBOARD_MASK) >> 3
#define ISPOWEROF2(x) (0 == (x&(x-1)))
/* 
8 =  0100
7 =  0011
17= 10001
16= 10000
16 >> 3
  = 00100
*/
//typedef UInt8 Square;
typedef struct Square {
  unsigned onBoard	: 1;	//Is the tile on the board
  unsigned attributes	: 3;	//The attributes
} Square;

#define MAX_SIZE_WIDTH 25 //definite max
#define MAX_SIZE_HEIGHT 25 //definite max

//Universal Settings associated with any game (can be used in conjunction
//with the tiles and the moves for a game to recreate the game as it was played
//previously.
typedef struct GameSettings {
  //This was the seed used to generate this game
  UInt16 width;
  UInt16 height;
  //You only actually need a pointer cause you can use the recover handle
  //functions to get the handle when it is needed for resizing/freeing memory 
  UInt8 skillLevel;
  Square *board;
  //Show possible moves
  Boolean showPossible;
} GameSettings;

typedef struct Board {
  GameSettings g;
  //You only actually need a pointer cause you can use the recover handle
  //functions to get the handle when it is needed for resizing/freeing memory 
  UInt16 size;
  UInt16 squareWidth;
  UInt16 squareHeight;
  RectangleType rect;
  Int16 pieceSelected;
  Int32 blinkRate;
  Int32 timeToBlink;
  Boolean blinking;
  Boolean blinkOn;
} Board;

typedef struct Move {
  UInt16 from : 10;
  UInt16 over : 10;
  UInt16 to   : 10;
  Square src;
  Square mid;
  Square dest;
} Move;

typedef struct aGame {
  //You only actually need a pointer cause you can use the recover handle
  //functions to get the handle when it is needed for resizing/freeing memory
  Board theBoard;
  Move *moves;
  UInt16 numMoves;
} aGame;

//Preferences must include:
//An Array of type Move of moves
//An aGame structure (with all the board settings)
//An Array of Squares that fill the board.

//Begin GLobal Variables
//Think binary counting here. the third color is 1 + 2;
static const UInt32 colorsHex[8] = {WHITE,YELLOW,BLUE,GREEN,RED,
				    ORANGE,PURPLE,GRAY};
//colors for each of the possible tiles.
static IndexedColorType colors[8];
static aGame Game;
//True when the preferences have been modified.
static Boolean prefsTouched = 0;
//temporary preferences variables
static struct {
  UInt8 Height;
  UInt8 Width;
} tempPreferences;


//static DmOpenRef TessDB;
static struct {
  UInt32 RomVersion;
  UInt32 ScreenWidth;
  UInt32 ScreenHeight;
  UInt16 ScreenHeader;
  UInt16 ScreenBorder;
  UInt16 MinimumPixelSize;
  UInt16 MinWidth;
  UInt16 MinHeight;
  UInt16 MaxWidth;
  UInt16 MaxHeight;
  Boolean Color;
} DeviceSettings;
//End Global Variables
typedef union daMoves {
  UInt8 byte;
  struct d {
    unsigned nn: 1;
    unsigned ne: 1;
    unsigned ee: 1;
    unsigned se: 1;
    unsigned ss: 1;
    unsigned sw: 1;
    unsigned ww: 1;
    unsigned nw: 1;
    } d;
} daMoves;


PointType GetXY(Board *b, UInt16 Index);
#endif
