/*************************************************
 Tesserae
 Original Concept by Kent Brewster
 (http://www.sff.net/people/k.brewster/)
 in his game Stained Glass
**************************************************/
#include <PalmOS.h>
#include "TessRsc.h"

#define ROMVersionMinimum				0x03000000
#define RomOS35						0x03500000
#define TessDBName	"TesseraeDB"
#define TessDBType	'DATA'
#define TessCreatorID	'TeSS'
#define TessPrefVers    0x0001

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
#define GREEN 	0x00008000
#define PURPLE	0x00800080
#define ORANGE	0x00FF9900
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
  Boolean squarePieces;
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
  UInt16 from : 8;
  UInt16 to   : 8;
} Move;

typedef struct aGame {
  //You only actually need a pointer cause you can use the recover handle
  //functions to get the handle when it is needed for resizing/freeing memory
  Board theBoard;
  //Move *moves;
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
//static Pref Game;


//static DmOpenRef TessDB;
static struct {
  UInt32 RomVersion;
  UInt32 ScreenWidth;
  UInt32 ScreenHeight;
  UInt16 ScreenHeader;
  UInt16 ScreenBorder;
  UInt16 MinimumPixelSize;
  UInt16 MaxWidth;
  UInt16 MaxHeight;
  Boolean Color;
} DeviceSettings;
//End Global Variables

/*
static void MovePushMove(aGame *g, Move m) {
  MemHandle h = MemPtrRecoverHandle(g->moves);
  if (g->numMoves >= (MemHandleSize(h) / sizeof(Move))) {
    MemHandleUnlock(h);
    MemHandleResize(h, sizeof(Move) * (g->numMoves + 8));
    g->moves = MemHandleLock(h);
  }
  g->moves[g->numMoves] = m;
  g->numMoves++;  
}

static void MovePopMove(aGame *g, Move *m) {
  MemHandle h = MemPtrRecoverHandle(g->moves);
  if (g->numMoves == 0) {
    //Empty Stack!
    return;
  }
  else if ((g->numMoves + 8) < (MemHandleSize(h) / sizeof(Move))) {
    MemHandleUnlock(h);
    MemHandleResize(h, sizeof(Move) * (g->numMoves - 8));
    g->numMoves -= 8;
    g->moves = MemHandleLock(h);
  }
  g->numMoves--;
  *m = g->moves[g->numMoves - 2];
}

static void MoveGetMove(aGame *g, Move *m, UInt16 pos) {
  //MemHandle h = MemPtrRecoverHandle(g->moves);
  if (pos >= g->numMoves) {
    //Bad Position!
    return;
  }
  *m = g->moves[pos];
}
*/
static UInt16 RandomNum(UInt16 n) {
  //this is ghetto, but fixes divide by zero issues should they arise.

  if (n == 0)
    return 0;
  else
    return SysRandom(0) / (1 + sysRandomMax / n);
}

static Board MakeBoard(UInt8 width, UInt8 height, Boolean squarePieces) {
  Board b;
  UInt16 i;
  b.g.width = width;
  b.g.height = height;
  b.g.squarePieces = squarePieces;
  //The -1 at the end is to take the lines between cells into account.
  b.squareWidth = (DeviceSettings.ScreenWidth - 
		   (DeviceSettings.ScreenBorder * 3)) / (b.g.width) -1;
  b.squareHeight = (DeviceSettings.ScreenHeight - DeviceSettings.ScreenHeader -
		    (DeviceSettings.ScreenBorder * 3)) / (b.g.height) -1;
  if (b.squareWidth % 2 != 1)
    b.squareWidth--;
  if (b.squareHeight % 2 != 1)
    b.squareHeight--;
  //Make the pieces square if need be.
  if (b.g.squarePieces) {
    if (b.squareHeight >= b.squareWidth)
      b.squareHeight = b.squareWidth;
    else
      b.squareWidth = b.squareHeight;
  }
  b.rect.topLeft.x = ((DeviceSettings.ScreenWidth -
		    (DeviceSettings.ScreenBorder * 2) -
		    ((b.squareWidth) * b.g.width)) /2);
  b.rect.topLeft.y = (((DeviceSettings.ScreenHeight -
		     (DeviceSettings.ScreenBorder * 2) -
		     DeviceSettings.ScreenHeader -
		     (b.squareHeight * b.g.height)) /2) +
		   DeviceSettings.ScreenHeader);
  b.rect.extent.x = ((DeviceSettings.ScreenWidth - 
		      b.rect.topLeft.x));
  b.rect.extent.y = ((DeviceSettings.ScreenHeight -
		      b.rect.topLeft.y) +
		     DeviceSettings.ScreenHeader);
  
  b.size = width * height;
  b.g.board = MemHandleLock(MemHandleNew(sizeof(Square) * b.size));
  MemSet(b.g.board, (b.size * sizeof(Square)), 0x00);
  for (i = 0; i < b.size; i++) {
    //Sets the bit in b.g.board[i] to make it a usable part of the board
    b.g.board[i].onBoard = 1;
  }
  return b;
}

static void FillBoardRandom(GameSettings *g) {
  //Level 0 means all primary color tiles
  //Level 1 means primary and secondary color tiles
  //Level 2 means primary, secondary and gray tiles.
  UInt16 i, tempNum;
  //this large loop is really ghetto, I could make it way quicker
  //if i had seperate loops for each difficulty type. Also this would
  //allow a lot more flexibility.
  for (i = 0; i < (g->width * g->height);i++) {
    if (g->board[i].onBoard) {
      //Clears the attribute bits that might have been set 
      g->board[i].attributes = 0;
      //This gives us a number between 1 and 7 (0 would be invalid cause there
      //would be no tile there.
      if (g->skillLevel == difficultyEasy) {
	tempNum = RandomNum(3);
	if (tempNum == 0) {
	  g->board[i].attributes = 1;
	}
	else if (tempNum == 1) {
	  g->board[i].attributes = 2;
	}
	else if (tempNum == 2) {
	  g->board[i].attributes = 4;
	}
	else
	  return;
      }
      //these next two are the same, but will be different
      else if (g->skillLevel == difficultyIntermediate) {
	g->board[i].attributes += RandomNum(6) + 1;
      }
      else if (g->skillLevel == difficultyMoreDifficult) {
	g->board[i].attributes += RandomNum(6) + 1;
      }
      //these next two are the same, but will be different
      else if (g->skillLevel == difficultyAdvanced) {
	g->board[i].attributes += RandomNum(7) + 1;
      }
      //It's not actually impossible, just very difficult.
      else if (g->skillLevel == difficultyImpossible) {
	g->board[i].attributes += RandomNum(7) + 1;
      }
      //if not one of the defined difficultytypes we don't know how to fill it
      else {
	return;
      }
    }
  }
}
static aGame GetSavedGame() {
  aGame ag;
  Boolean unsaved = false;
  UInt16 agSize = sizeof(ag), boardSize, movesSize;
  //Forcing never loading the preferences
  if (true &&
      (PrefGetAppPreferences(TessCreatorID, Pref_aGameStruct,
			     &ag, &agSize,
			     unsaved) != noPreferenceFound)) {
    boardSize = (ag.theBoard.size * sizeof(Square));
    ag.theBoard.g.board = MemHandleLock(MemHandleNew(boardSize));
    MemSet(ag.theBoard.g.board, boardSize, 0x00);
    if (PrefGetAppPreferences(TessCreatorID, Pref_SquaresArray,
			      ag.theBoard.g.board, &boardSize,
			      unsaved) != noPreferenceFound) {
      
      //movesSize = sizeof(ag.numMoves * sizeof(Move)) + 1;
      //ag.moves = MemHandleLock(MemHandleNew(movesSize));
      //if (PrefGetAppPreferences(TessCreatorID, Pref_MovesArray,
      //			&ag.moves, &movesSize,
      //			unsaved) != noPreferenceFound) {
      return ag;
      //}
      //Insert the proper unlocks so that there is no memory leak;
      //MemHandleUnlock(MemPtrRecoverHandle(ag.moves));
    }
    MemHandleUnlock(MemPtrRecoverHandle(ag.theBoard.g.board));
  }
  
  MemSet(&ag, agSize, 0x00);

  // No preferences exist yet, so set the defaults
  ag.theBoard = MakeBoard(8, 8, true);
  ag.theBoard.g.skillLevel = difficultyEasy;
  ag.theBoard.g.showPossible = true;
  ag.theBoard.pieceSelected = -1;
  ag.theBoard.blinkRate = SysTicksPerSecond() / 2;
  ag.theBoard.blinking = false;
  ag.theBoard.blinkOn = false;
  
  FillBoardRandom(&ag.theBoard.g);
  //ag.moves = MemHandleLock(MemHandleNew(8 * sizeof(Move)));
  //MemSet(ag.moves, (8 * sizeof(Move)), 0x00);

  return ag;
}
static void StoreSavedGame(aGame ag) {
  Boolean unsaved = false;
  UInt16 agSize = sizeof(ag), boardSize, movesSize;
  MemHandle h1, h2;

  PrefSetAppPreferences(TessCreatorID, Pref_aGameStruct,
			TessPrefVers, &ag,
			agSize, unsaved);
  boardSize = ag.theBoard.size * sizeof(Square);
  PrefSetAppPreferences(TessCreatorID, Pref_SquaresArray,
			TessPrefVers, ag.theBoard.g.board,
			boardSize, unsaved);
//movesSize = sizeof(ag.numMoves * sizeof(Move));
//PrefSetAppPreferences(TessCreatorID, Pref_MovesArray,
//		TessPrefVers, &ag.moves,
//		movesSize, unsaved);
//if (ag.moves) {
//  h1 = MemPtrRecoverHandle(ag.moves);
//if (h1)
//  MemHandleUnlock(h1);
//  }
  if (ag.theBoard.g.board) {
    h2 = MemPtrRecoverHandle(ag.theBoard.g.board);
    if (h2)
      MemHandleUnlock(h2);
  }
}

static Int16 GetIndex(Board *b, UInt8 x, UInt8 y) {
  if ((x < b->g.width) && (y < b->g.height))
    return (x + (b->g.width * y));
  else
    return -1;
}

static PointType GetXY(Board *b, UInt16 Index) {
  PointType p;
  if (Index < b->size && Index >= 0) {
    p.x = (Index % b->g.width);
    p.y = (Index / b->g.width);
  }
  else {
    p.x = -20;
    p.y = -20;
    //Display and error message or something.
  }
  return (p);
}

static PointType AbsoluteToRelative(Board *b, PointType p1) {
  PointType p2;
  p2.x = p1.x - b->rect.topLeft.x;
  p2.y = p1.y - b->rect.topLeft.y;
  return p2;
}

static UInt16 ScreenCoordToBoardIndex(Board *b, Coord x, Coord y) {
  UInt8 row, column;
  if (RctPtInRectangle(x, y, &b->rect)) {
    PointType p;
    p.x = x;
    p.y = y;
    p = AbsoluteToRelative(b, p);
    row = p.x / (b->squareWidth + 1);
    column = p.y / (b->squareHeight + 1);
    return GetIndex(b, row, column);
  }
  else
    return -1;
}

//Was more useful when I used a handle for the board within board,
//but now its just a static array.
static void FreeBoard(Board *b) {
  //if (b->size) {
    //if (MemPtrRecoverHandle(b->g.board)) {
      MemHandleFree(MemPtrRecoverHandle(b->g.board));
      //Clear any other contents so it won't be used again.
      //MemSet(b, sizeof(Board), 0x00);
      //}
      //}
}


static void DrawCircleInRect(RectangleType r) {
  UInt8 j = 0;
  //Palm says the whole FrameBitsType thing doesn't have a garunteed structure
  //So I don't know if this will work in future revisions.
  FrameBitsType frameBits;
  frameBits.word = rectangleFrame;

  r.topLeft.x += j;
  r.topLeft.y += j;
  r.extent.x  -= (j * 2) -1;
  r.extent.y  -= (j * 2) -1;
  frameBits.bits.cornerDiam = (r.extent.x /2) +1;
  frameBits.bits.width = 1;
  WinDrawRectangleFrame(frameBits.word, &r);
}
static void DrawPlusInRect(RectangleType r) {
  WinDrawLine(r.topLeft.x + (r.extent.x /2), r.topLeft.y,
	      r.topLeft.x + (r.extent.x /2), r.topLeft.y + r.extent.y);
  WinDrawLine(r.topLeft.x, r.topLeft.y + (r.extent.y /2),
	      r.topLeft.x + r.extent.x, r.topLeft.y + (r.extent.y /2));
}
/*
static void DrawXInRect(RectangleType r) {
  WinDrawLine(r.topLeft.x, r.topLeft.y,
	      r.topLeft.x + r.extent.x, r.topLeft.y + r.extent.y);
  WinDrawLine(r.topLeft.x + r.extent.x, r.topLeft.y,
  r.topLeft.x, r.topLeft.y + r.extent.y);
  }
*/
static void DrawSquareInRect(RectangleType r) {
  UInt8 j = 2;
  FrameType frame = rectangleFrame;
  r.topLeft.x += j;
  r.topLeft.y += j;
  r.extent.x  -= (j * 2) -1;
  r.extent.y  -= (j * 2) -1;
  WinDrawRectangleFrame(frame, &r);
}
//This could be a billion times faster if I had a window setup for each
//of the different tiles and then copied them in rather than drawing each
//one on it's own.
static void DrawOneSquare(Board *b, PointType p, Square s) {
  RectangleType color, r;
  IndexedColorType oldBGIndex;
  //The +1 and -3 keep it within the bounds of the rectangle.
  r.topLeft.x = p.x +2;
  r.topLeft.y = p.y +2;
  r.extent.x = b->squareWidth -5;
  r.extent.y = b->squareHeight -5;
  if (DeviceSettings.Color) {
    color.topLeft = p;
    color.extent.x = b->squareWidth;
    color.extent.y = b->squareHeight; 
    oldBGIndex = WinSetBackColor(colors[s.attributes]);
    WinEraseRectangle(&color, 0);
    WinSetBackColor(oldBGIndex);
  }
  if ((s.attributes >> 0) %2) {    
    DrawPlusInRect(r);
  }
  if ((s.attributes >> 1) %2) {
    DrawSquareInRect(r);
  }
  if ((s.attributes >> 2) %2) {
    DrawCircleInRect(r);
  }
}

static void DrawALoc(Board *b, UInt16 loc){
  Square s = b->g.board[loc];
  PointType boardPos = GetXY(b, loc);
  RectangleType r;
  r.topLeft.x = (b->rect.topLeft.x + ((1 + b->squareWidth) * boardPos.x));
  r.topLeft.y = (b->rect.topLeft.y + ((1 + b->squareHeight) * boardPos.y));
  r.extent.x = b->squareWidth;
  r.extent.y = b->squareHeight;
  WinEraseRectangle(&r, 0);
  DrawOneSquare(b, r.topLeft, s);
}
static void DrawBox(Board *b, UInt16 index) {
  //j = 3 gives a nice effect that would work well for checkers
  UInt8 j = 6;
  //FrameBitsType frameBits;
  RectangleType r;
  PointType p;
  if (index != Game.theBoard.pieceSelected) {
    p = GetXY(b, index);
    r.topLeft.x = b->rect.topLeft.x + ((b->squareWidth+1) * p.x);
    r.topLeft.y = b->rect.topLeft.y + ((b->squareHeight+1) * p.y);
    r.extent.x = (b->squareWidth);
    r.extent.y = (b->squareHeight);
    //Palm says FrameBitsType doesn't have a garunteed structure
    //So I don't know if this will work in future revisions.
    //frameBits.word = rectangleFrame;
    
    r.topLeft.x += j;
    r.topLeft.y += j;
    r.extent.x  -= (j * 2);
    r.extent.y  -= (j * 2);
    //frameBits.bits.cornerDiam = (r.extent.x /2) +1;
    //frameBits.bits.width = 1;
    WinDrawRectangle(&r, 3);
    //WinDrawRectangleFrame(frameBits, &r);
    //b->g.board[pieceSelected]
  }
  else {
    DrawALoc(b, index);
  }
}
static void DrawSquares(Board *b) {
  FrameType frame = rectangleFrame;
  Int16 i, j;
  RectangleType r;
  //UInt32 TotalSeconds = 2, TicksPerSec = SysTicksPerSecond();
  //Int32 delay = (((TotalSeconds * TicksPerSec) / b->size) - );
  //Int32 delay = (SysTicksPerSecond() / 50);
  
  //If these extents were one less we wouldn't get that cool shadow effect.
  b->rect.extent.x = (b->squareWidth+1) * b->g.width;
  b->rect.extent.y = (b->squareHeight+1) * b->g.height;
  WinEraseRectangle(&b->rect, 0);
  WinDrawRectangleFrame(frame, &b->rect);

  r.extent.x = b->squareWidth;
  r.extent.y = b->squareHeight;
  for (j = b->g.height -1; j >= 0; j--) {
    //Set the vertical position
    r.topLeft.y = b->rect.topLeft.y + (j * (b->squareHeight + 1));
    for (i = 0; i < b->g.width; i++) {
      //Set the horizontal position
      r.topLeft.x = b->rect.topLeft.x + (i * (b->squareWidth + 1));
      //For Delay, if desired.
      //SysTaskDelay(delay);
      WinDrawRectangleFrame(frame, &r);
      if (b->g.board[GetIndex(b, i, j)].onBoard) {
	DrawOneSquare(b, r.topLeft, b->g.board[GetIndex(b, i, j)]);
      }
      else {
	const CustomPatternType halfGray ={0x55,0xAA,0x55,0xAA,
					   0x55,0xAA,0x55,0xAA};
	WinSetPattern(&halfGray);
	WinFillRectangle(&r, 0);
      }
    }
  }
}

//Taken from Palm Programming Bible
/*
static void DrawBitmap (UInt16 bitmapID, Int16 x, Int16 y){
  MemHandle bitmapH;
  // Retrieve a handle to the bitmap resource.
  bitmapH = DmGetResource(bitmapRsc, bitmapID);
  if (bitmapH) {
    BitmapType *bitmap;
    // Lock the bitmap handle to retrieve a pointer, then
    // draw the bitmap.
    bitmap = (BitmapPtr)MemHandleLock(bitmapH);
    WinDrawBitmap(bitmap, x, y);
    MemHandleUnlock(bitmapH);
    // Release the bitmap resource.
    DmReleaseResource(bitmapH);
  }
}
*/

//Checks to see if the points are the proper distance apart.
static Boolean checkSquares(PointType p1, PointType p2) {
  Boolean positions;
  positions = (((p1.x + 2 == p2.x) ||
		(p1.x - 2 == p2.x) ||
		(p1.x     == p2.x)) &&
	       ((p1.y + 2 == p2.y) ||
		(p1.y - 2 == p2.y) ||
		(p1.y     == p2.y)));
  return positions;
}

static Boolean move(Board *b, UInt16 src, UInt16 dest) {
  UInt16 mid;
  Square newSrc, newMid, newDest;
  PointType srcPt, midPt, destPt;
  srcPt = GetXY(b, src);
  destPt = GetXY(b, dest);
  if (checkSquares(srcPt, destPt)) {
    midPt.x = ((srcPt.x + destPt.x) / 2);
    midPt.y = ((srcPt.y + destPt.y) / 2);
    mid = GetIndex(b, midPt.x, midPt.y);

    //If all of the spaces are on the board.
    if (b->g.board[src].onBoard && b->g.board[mid].onBoard
	&& b->g.board[dest].onBoard) {
    
      newSrc.attributes = b->g.board[src].attributes;
      newMid.attributes = b->g.board[mid].attributes;
      newDest.attributes = b->g.board[dest].attributes;
      
      //If the middle has some tiles on it.
      if ((newMid.attributes) &&
	  //And either the src and the middle are primaries.
	  ((((ISPOWEROF2(newSrc.attributes)) &&
	     (ISPOWEROF2(newMid.attributes))) ||
	    //Or the mid contains everything that is in the src
	    ((newSrc.attributes & newMid.attributes) == newSrc.attributes)) &&
	   //And either the src and dest are the same
	   ((newSrc.attributes == newDest.attributes) ||
	    //or the destination has nothing that the source does
	    ((newSrc.attributes & newDest.attributes) == 0)))) {
	
	newMid.attributes = (((ISPOWEROF2(newSrc.attributes)) &&
			      (ISPOWEROF2(newMid.attributes))) ? 0 :
			     (newMid.attributes & (~newSrc.attributes)));
	newDest.attributes = (newSrc.attributes | newDest.attributes);
	newSrc.attributes = 0;
	
	b->g.board[src].attributes = newSrc.attributes;
	b->g.board[mid].attributes = newMid.attributes;
	b->g.board[dest].attributes = newDest.attributes;
	
	DrawALoc(b, src);
	DrawALoc(b, mid);
	DrawALoc(b, dest);
	return(true);
      }
    }
  }
  b->pieceSelected = -1;
  return false;
}
static void newGame() {
  //Game = GetSavedGame();
  Game.theBoard = MakeBoard(Game.theBoard.g.width, Game.theBoard.g.height,
		 Game.theBoard.g.squarePieces);
  Game.theBoard.pieceSelected = -1;
  Game.theBoard.blinkRate = SysTicksPerSecond() / 2;
  Game.theBoard.blinking = false;
  Game.theBoard.blinkOn = false;
  Game.theBoard.g.skillLevel = difficultyEasy;
  FillBoardRandom(&Game.theBoard.g);
  DrawSquares(&Game.theBoard);
}

static RGBColorType ColorHexToRGB(Int32 hex) {
  RGBColorType rgb;
  rgb.r = ((hex & 0x00FF0000) >> 16);
  rgb.g = ((hex & 0x0000FF00) >> 8);
  rgb.b = ((hex & 0x000000FF) >> 0);
  rgb.index = WinRGBToIndex(&rgb);
  return rgb;
}

static void setupColors() {
  UInt8 i;
  for (i = 0; i < 8; i++) {
    colors[i] = ColorHexToRGB(colorsHex[i]).index;
  }
}

static void setupDeviceSettings() {
  //For 8bit color...
  UInt32 ColorDepths, ColorDrawMode = 8;
  //For 16bit color (not needed):
  //ColorDrawMode = 16;
  
  if (DeviceSettings.RomVersion >= RomOS35)
    WinPushDrawState();
  WinScreenMode(winScreenModeGet, &DeviceSettings.ScreenWidth,
		&DeviceSettings.ScreenHeight, &ColorDepths,
		NULL);
  if (ColorDrawMode & ColorDepths) {
    // Change color depth and refresh the screen.
    WinScreenMode(winScreenModeSet, NULL, NULL, &ColorDrawMode, NULL);
    DeviceSettings.Color = true;
    FrmUpdateForm(MainForm, frmRedrawUpdateCode);
    setupColors();
  }
  else {
    DeviceSettings.Color = false;
  }

  DeviceSettings.ScreenHeight = 160;
  DeviceSettings.ScreenHeader = 15;
  DeviceSettings.ScreenBorder = 3;
  DeviceSettings.MinimumPixelSize = 15;
  DeviceSettings.MaxWidth = ((DeviceSettings.ScreenWidth -
			      DeviceSettings.ScreenBorder) / 
			     DeviceSettings.MinimumPixelSize);
  DeviceSettings.MaxHeight = ((DeviceSettings.ScreenWidth -
			      DeviceSettings.ScreenHeader -
			      DeviceSettings.ScreenBorder) /
			     DeviceSettings.MinimumPixelSize);
}

//Stolen from the palm SDK examples.
static Err RomVersionCompatible (UInt32 requiredVersion, UInt16 launchFlags) {
  // See if we're on in minimum required version of the ROM or later.
  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &DeviceSettings.RomVersion);
  if (DeviceSettings.RomVersion < requiredVersion) {
    if ((launchFlags & (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
	(sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) {
      FrmAlert (RomIncompatibleAlert);
      
      // Pilot 1.0 will continuously relaunch this app unless we switch to 
      // another safe one.
      if (DeviceSettings.RomVersion < 0x02000000) {
	AppLaunchWithCommand(sysFileCDefaultApp,
			     sysAppLaunchCmdNormalLaunch, NULL);
      }
    }
    return (sysErrRomIncompatible);
  }
  return (0);
}

static Err StartApplication(UInt16 launchFlags) {
  Err  err;
  err = RomVersionCompatible (ROMVersionMinimum, launchFlags);
  if (err)
    return (err);
  
  setupDeviceSettings();
  
  /*TessDB = DmOpenDatabaseByTypeCreator(TessDBType, TessCreatorID, dmModeReadWrite);
  if (! TessDB) {
    err = DmCreateDatabase(0, TessDBName, TessCreatorID, TessDBType, false);
    if (err)
      return err;
    TessDB = DmOpenDatabaseByTypeCreator(TessDBType, TessCreatorID, dmModeReadOnly);
    if (! TessDB)
      return DmGetLastErr();
      }*/

  Game = GetSavedGame();

  FrmGotoForm(MainForm);
  return 0;
}


static void StopApplication(void) {
  StoreSavedGame(Game);
  //This isn't needed, StoreSavedGame unlocks the necessary handles
  FreeBoard(&Game.theBoard);
  //DmCloseDatabase(TessDB);
  if (DeviceSettings.RomVersion >= RomOS35)
    WinPopDrawState();
  FrmCloseAllForms();
}

static Boolean MainMenuHandleEvent(UInt16 menuID) {
  Boolean    handled = false;
  //FormType   *form = FrmGetActiveForm();
  FormType   *frmP;
  UInt16 whichButton;
  
  switch (menuID) {
  case MainGamePreferences:
    frmP = FrmInitForm(FrmPreferences);
    /* initialize your controls on the form here */
    // open the dialog, and wait until a button is pressed to close it.
    whichButton = FrmDoDialog(frmP);
    if (whichButton == PreferencesOKButton) {
      /* get data from controls on the form to save/apply changes */
    }
    FrmDeleteForm(frmP);
    
    //FrmPopupForm(FrmPreferences);
    handled = true;
    break;
  case MainOptionsAbout:
    FrmAlert(AboutAlert);
    handled = true;
    break;  
  case MainGameInstructions:
    FrmHelp(InstructionsStr);
    handled = true;
    break;  
  case MainGameNew:
    FreeBoard(&Game.theBoard);
    newGame();
    handled = true;
    break;  
  default:
    break;
  }
  
  return handled;
}


static Boolean MainFormHandleEvent(EventPtr event)
{
  Boolean	handled = false;
  FormType	*form = FrmGetActiveForm();
  UInt16 boardIndex, formID;

  switch (event->eType) {
  case frmOpenEvent:
    formID = event->data.frmOpen.formID;
    FrmDrawForm(form);
    if (formID == MainForm) {
      DrawSquares(&Game.theBoard);
    }
    handled = true;
    break;
  case ctlSelectEvent:
    switch (event->data.ctlSelect.controlID) {
    case MainFormNewButton:
      FreeBoard(&Game.theBoard);
      newGame();
      handled = true;
      break;
    case MainFormRDButton:
      //Game.skillLevel = difficultyIntermediate;
      DrawSquares(&Game.theBoard);
      handled = true;
      break;
    default:
      break;
    }
    break;
    case penDownEvent:
    boardIndex = ScreenCoordToBoardIndex(&Game.theBoard, event->screenX, event->screenY);
    if (-1 != boardIndex) {
        //If there is no piece selected, select the piece.
      if (-1 == Game.theBoard.pieceSelected) {
	if (Game.theBoard.g.board[boardIndex].attributes != 0x00) {
	  DrawBox(&Game.theBoard, boardIndex);
	  Game.theBoard.pieceSelected = boardIndex;
	}
      }
      //If the same piece has been clicked again, deselect that piece
      else if (boardIndex == Game.theBoard.pieceSelected) {
	DrawBox(&Game.theBoard, boardIndex);
	Game.theBoard.pieceSelected = -1;
      }
      else {
	DrawBox(&Game.theBoard, Game.theBoard.pieceSelected);
	if (move(&Game.theBoard, Game.theBoard.pieceSelected, boardIndex)) {
	  //DrawBox(&Game.theBoard, boardIndex);
	  Game.theBoard.pieceSelected = -1;
	}
      }
      handled = true;
    }
    //the pen down event is not within the board.
    else {
    }
    break;
  case menuEvent:
    handled = MainMenuHandleEvent(event->data.menu.itemID);
    break;
  default:
    break;
  }
  
  return handled;
}

static Boolean ApplicationHandleEvent(EventPtr event)
{
  FormType  *form;
  UInt16    formID;
  Boolean   handled = false;
  
  
  if (event->eType == frmLoadEvent) {
    formID = event->data.frmLoad.formID;
    form = FrmInitForm(formID);
    FrmSetActiveForm(form);
    switch (formID) {
    case MainForm:
      FrmSetEventHandler(form, MainFormHandleEvent);
      break;  
    default:
      break;
    }
    handled = true;
  }
  return handled;
}

static void EventLoop(void) {
  EventType  event;
  UInt16     error;
  
  do {
    EvtGetEvent(&event, evtWaitForever);
    if (! SysHandleEvent(&event))
      if (! MenuHandleEvent(0, &event, &error))
	if (! ApplicationHandleEvent(&event))
	  FrmDispatchEvent(&event);
  }
  while (event.eType != appStopEvent);
}

UInt32 PilotMain(UInt16 launchCode, MemPtr cmdPBP,
                 UInt16 launchFlags) {
  Err  err;
  
  switch (launchCode) {
  case sysAppLaunchCmdNormalLaunch:
    if ((err = StartApplication(launchFlags)) == 0) {
      EventLoop();
      StopApplication();
    }
    break;
  case sysAppLaunchCmdSaveData:
    FrmSaveAllForms ();
    break;
  default:
    break;
  }
  return err;
}
