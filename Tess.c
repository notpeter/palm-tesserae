/*************************************************
 Tesserae
 Original Concept by Kent Brewster
 (http://www.sff.net/people/k.brewster/)
 in his game Stained Glass
**************************************************/
#include <PalmOS.h>
#include "TessRsc.h"
#include "Tess.h"

#define ROMVersionMinimum				0x03000000
#define RomOS35						0x03500000
#define TessDBName	"TesseraeDB"
#define TessDBType	'DATA'
#define TessCreatorID	'TeSS'
#define TessPrefVers    0x0001


static void DrawBox(Board *b, UInt16 index, Boolean Round) {
  UInt8 j, k = 0;
  //FrameBitsType frameBits;
  RectangleType r;
  PointType p;
  
  if (b->squareWidth >= b->squareHeight) {
    j = ((b->squareHeight / 4) + 1);
  }
  else {
    j = ((b->squareWidth / 4) + 1);
  }


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

  if (Round)
    k = (r.extent.x / 2);

  //k here can never fully round out the corner cause of how it is defined.
  //Redo this to fix it.
  WinDrawRectangle(&r, k);
  //WinDrawRectangleFrame(frameBits, &r);
  //b->g.board[pieceSelected]
}

//assumes coordinates are already correct
static Boolean isValidMove(Board *b, UInt16 src, UInt16 mid, UInt16 dest) {
  if ((b->g.board[src].attributes &&
       b->g.board[mid].attributes) &&
      //And either the src and the middle are primaries.
      ((((ISPOWEROF2(b->g.board[src].attributes)) &&
	 (ISPOWEROF2(b->g.board[mid].attributes))) ||
	//Or the mid contains everything that is in the src
	((b->g.board[src].attributes & b->g.board[mid].attributes) == b->g.board[src].attributes)) &&
       //And either the src and dest are the same
       ((b->g.board[src].attributes == b->g.board[dest].attributes) ||
	//or the destination has nothing that the source does
	((b->g.board[src].attributes & b->g.board[dest].attributes) == 0)))) {
    return true;
  }
  else
    return false;
}

static daMoves validMoves(aGame *g, UInt16 index) {
  Board *b = &(g->theBoard);
  daMoves l; //what moves are legal;
  
  UInt16 src = index, mid, dest;

  l.byte = 0xFF; //Starts with all moves are possible.

  //If it is not on the board, or if the space is empty. no valid moves.
  if (!(b->g.board[index].onBoard &&
	b->g.board[index].attributes)) {
    daMoves g;
    g.byte = 0;
    return g;
  }
  //in the first two rows -- no n, ne, nw
  if ((index / b->g.width) < 2) {
    l.d.nn = l.d.ne = l.d.nw = 0;
  }
  //in the last two rows -- no sw, s, se
  if ((index / b->g.width) > b->g.height - 2) {
    l.d.sw = l.d.ss = l.d.se = 0;
  }
  //in first two columns -- no nw, w, sw
  if ((index % b->g.width) < 2) {
    l.d.nw = l.d.ww = l.d.sw = 0;
  }
  //in the last two columns -- no se, e, ne
  if ((index % b->g.width) > b->g.width - 2) {
    l.d.se = l.d.ee = l.d.ne = 0;
  }
  if (l.d.nn) {
    mid = src - b->g.width;
    dest = (src - (b->g.width * 2));
    l.d.nn = isValidMove(b, src, mid, dest);
  }
  if (l.d.ne) {
    mid = (src - b->g.width) + 1;
    dest = (src - (b->g.width * 2)) + 2;
    l.d.ne = isValidMove(b, src, mid, dest);
  }
  if (l.d.ee) {
    mid = src + 1;
    dest = src + 2;
    l.d.ee = isValidMove(b, src, mid, dest);
  }
  if (l.d.se) {
    mid = (src + b->g.width) + 1;
    dest = (src + (b->g.width * 2)) + 2;
    l.d.se = isValidMove(b, src, mid, dest);
  }
  if (l.d.ss) {
    mid = src + b->g.width;
    dest = (src + (b->g.width * 2));
    l.d.ss = isValidMove(b, src, mid, dest);
  }
  if (l.d.sw) {
    mid = (src + b->g.width) - 1;
    dest = (src + (b->g.width * 2)) - 2;
    l.d.sw = isValidMove(b, src, mid, dest);
  }
  if (l.d.ww) {
    mid = src - 1;
    dest = src - 2;
    l.d.ww = isValidMove(b, src, mid, dest);
  }
  if (l.d.nw) {
    mid = (src - b->g.width) - 1;
    dest = (src - (b->g.width * 2)) - 2;
    l.d.nw = isValidMove(b, src, mid, dest);
  }
  return l;
}
static Boolean anyValidMoves(aGame *g) {
  UInt16 i;
  for (i=0; i < g->theBoard.size; i++)
    if ((validMoves(g, i)).byte)
      return 1;
  return 0;
}
//The erase flag toggles whether the function is erasing or drawing.
static void DrawPossibilities(Board *b, daMoves l, UInt16 boardIndex,
			     Boolean erase) {
  //Fix this so it draws circles, squares or something else depending on
  //what the resulting peice will be.

  DrawBox(b, boardIndex, true);
  if (l.d.nn) {
    DrawBox(b, (boardIndex - (b->g.width * 2)), true);
  }
  if (l.d.ne) {
    DrawBox(b, (boardIndex - (b->g.width * 2)) + 2, true);
  }
  if (l.d.ee) {
    DrawBox(b, boardIndex + 2, true);
  }
  if (l.d.se) {
    DrawBox(b, (boardIndex + (b->g.width * 2)) + 2, true);
  }
  if (l.d.ss) {
    DrawBox(b, boardIndex + (b->g.width * 2), true);
  }
  if (l.d.sw) {
    DrawBox(b, boardIndex + (b->g.width * 2) - 2, true);
  }
  if (l.d.ww) {
    DrawBox(b, boardIndex - 2, true);
  }
  if (l.d.nw) {
    DrawBox(b, (boardIndex - (b->g.width * 2)) - 2, true);
  }
  //Square s = b->g.board[boardIndex];
  /*if (ISPOWEROF2(s.attributes)) {
    //Draw a circle
    DrawBox(b, boardIndex, true);
  }
  else {
    //Draw a square
    DrawBox(b, boardIndex, false);
    }*/
}
static void DrawPossibleMoves(aGame *g, UInt16 index) {
  daMoves f = validMoves(g, index);
  DrawPossibilities(&(g->theBoard), f, index, false);
}
static void updateMoveCounterDisplay(UInt32 numMoves) {
  FormType *frm = FrmGetActiveForm();
  Char numMovesStr[maxStrIToALen];
  StrIToA(numMovesStr, numMoves);
  FrmHideObject(frm, FrmGetObjectIndex(frm, MainFormMovesLabel));
  FrmCopyLabel (frm, MainFormMovesLabel, numMovesStr);
  FrmShowObject(frm, FrmGetObjectIndex(frm, MainFormMovesLabel));
}
static void MovePushMove(aGame *g, Move m) {
  MemHandle h = MemPtrRecoverHandle(g->moves);
  if (g->numMoves >= (MemHandleSize(h) / sizeof(Move))) {
    MemHandleUnlock(h);
    MemHandleResize(h, sizeof(Move) * (g->numMoves + 8));
    g->moves = MemHandleLock(h);
  }
  g->moves[g->numMoves] = m;
  g->numMoves++;
  updateMoveCounterDisplay(g->numMoves);
}

static void MovePopMove(aGame *g, Move *m) {
  //MemHandle h = MemPtrRecoverHandle(g->moves);
  if (g->numMoves == 0) {
    //Empty Stack!
    return;
  }
  /*else if (g->numMoves + 8) < (MemHandleSize(h) / sizeof(Move))) {
    MemHandleUnlock(h);
    MemHandleResize(h, sizeof(Move) * (g->numMoves - 8));
    g->numMoves -= 8;
    g->moves = MemHandleLock(h);
    }*/
  g->numMoves--;
  *m = g->moves[g->numMoves];
  updateMoveCounterDisplay(g->numMoves);
}


static void MoveGetMove(aGame *g, Move *m, UInt16 pos) {
  if (pos >= g->numMoves) {
    //Bad Position!
    return;
  }
  *m = g->moves[pos];
}

static UInt16 RandomNum(UInt16 n) {
  //this is ghetto, but fixes divide by zero issues should they arise.

  if (n == 0)
    return 0;
  else
    return SysRandom(0) / (1 + sysRandomMax / n);
}

static Board MakeBoard(UInt8 width, UInt8 height) {
  Board b;
  UInt16 i;
  b.g.width = width;
  b.g.height = height;
  //The -1 at the end is to take the lines between cells into account.
  b.squareWidth = (((DeviceSettings.ScreenWidth - 2) -
		   (DeviceSettings.ScreenBorder * 2)) / (b.g.width));
  b.squareHeight = (((DeviceSettings.ScreenHeight - 2) -
		    (DeviceSettings.ScreenHeader +
		    (DeviceSettings.ScreenBorder * 2))) / (b.g.height));
  if (b.squareWidth % 2 != 1)
    b.squareWidth--;
  if (b.squareHeight % 2 != 1)
    b.squareHeight--;
  if (b.squareHeight >= b.squareWidth)
    b.squareHeight = b.squareWidth;
  else
    b.squareWidth = b.squareHeight;
  b.rect.topLeft.x = ((DeviceSettings.ScreenWidth -
		    (DeviceSettings.ScreenBorder * 2) -
		    ((b.squareWidth) * b.g.width)) /2) + 1;
  b.rect.topLeft.y = (((DeviceSettings.ScreenHeight -
		     (DeviceSettings.ScreenBorder * 2) -
		     DeviceSettings.ScreenHeader -
		     ((b.squareHeight) * b.g.height)) /2) +
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
      
      movesSize = ((ag.numMoves+8) * sizeof(Move));
      ag.moves = MemHandleLock(MemHandleNew(movesSize));
      MemSet(ag.moves, movesSize, 0x00);
      if (PrefGetAppPreferences(TessCreatorID, Pref_MovesArray,
				ag.moves, &movesSize,
				unsaved) != noPreferenceFound) {
	return ag;
      }
      MemHandleUnlock(MemPtrRecoverHandle(ag.moves));
      MemHandleFree(MemPtrRecoverHandle(ag.moves));
    }
    MemHandleUnlock(MemPtrRecoverHandle(ag.theBoard.g.board));
    MemHandleFree(MemPtrRecoverHandle(ag.theBoard.g.board));
  }
  
  MemSet(&ag, agSize, 0x00);

  // No preferences exist yet, so set the defaults
  ag.theBoard = MakeBoard(7, 6);
  ag.theBoard.g.skillLevel = defaultDifficulty;
  ag.theBoard.g.showPossible = true;
  ag.theBoard.pieceSelected = -1;
//  ag.theBoard.blinkRate = SysTicksPerSecond() / 2;
  ag.theBoard.blinking = false;
  ag.theBoard.blinkOn = false;
  
  FillBoardRandom(&ag.theBoard.g);
  movesSize = ((ag.numMoves+8) * sizeof(Move));
  ag.moves = MemHandleLock(MemHandleNew(movesSize));
  MemSet(ag.moves, movesSize, 0x00);

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
  movesSize = ag.numMoves * sizeof(Move);
  PrefSetAppPreferences(TessCreatorID, Pref_MovesArray,
			TessPrefVers, ag.moves,
			movesSize, unsaved);
  if (ag.moves) {
    h1 = MemPtrRecoverHandle(ag.moves);
    if (h1)
      MemHandleUnlock(h1);
      MemHandleFree(h1);
  }
  if (ag.theBoard.g.board) {
    h2 = MemPtrRecoverHandle(ag.theBoard.g.board);
    if (h2)
      MemHandleUnlock(h2);
      MemHandleFree(h2);
  }
}

static Int16 GetIndex(Board *b, UInt8 x, UInt8 y) {
  if ((x < b->g.width) && (y < b->g.height))
    return (x + (b->g.width * y));
  else
    return -1;
}

PointType GetXY(Board *b, UInt16 Index) {
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
  return p;
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
  MemHandle h2;
  if (b->g.board) {
    h2 = MemPtrRecoverHandle(b->g.board);
    if (h2)
      MemHandleUnlock(h2);
      MemHandleFree(h2);
  }
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
  frameBits.bits.cornerDiam = (r.extent.x /2);
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
static void DrawSelection(Board *b, UInt16 boardIndex) {
  Square s = b->g.board[boardIndex];
  if (boardIndex != b->pieceSelected) {
    if (ISPOWEROF2(s.attributes)) {
      //Draw a circle
      DrawBox(b, boardIndex, true);
    }
    else {
      //Draw a square
      DrawBox(b, boardIndex, false);
    }
  }
  else {
    DrawALoc(b, boardIndex);
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

//Undo X Moves (-1 = all moves)
static void MoveUndo(aGame *g, Int16 count) {
  Board *b = &(g->theBoard);
  Move m;
  Int16 i;
  if (g->numMoves == 0)
    return;
  else if (count == -1)
    count = g->numMoves;

  for (i = 0; i < count; i++) {
    MovePopMove(g, &m);
    b->g.board[m.from] = m.src;
    b->g.board[m.over] = m.mid;
    b->g.board[m.to] = m.dest;
  }
  DrawALoc(b, m.from);
  DrawALoc(b, m.over);
  DrawALoc(b, m.to);
}

static Boolean move(aGame *g, UInt16 src, UInt16 dest) {
  Board *b = &(g->theBoard);
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
	{
	  Move m;
	  m.from = src;
	  m.over = mid;
	  m.to = dest;
	  m.src = b->g.board[src];
	  m.mid = b->g.board[mid];
	  m.dest = b->g.board[dest];
	  
	  MovePushMove(&Game, m);
	}
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
  Game.theBoard = MakeBoard(Game.theBoard.g.width, Game.theBoard.g.height);
  Game.theBoard.pieceSelected = -1;
  //Game.theBoard.blinkRate = SysTicksPerSecond() / 2;
  Game.theBoard.blinking = false;
  Game.theBoard.blinkOn = false;
  Game.theBoard.g.skillLevel = difficultyEasy;
  FillBoardRandom(&Game.theBoard.g);
  //DrawSquares(&Game.theBoard);
  Game.numMoves = 0;
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

  //DeviceSettings.ScreenHeight = 160;
  DeviceSettings.ScreenHeader = 15;
  DeviceSettings.ScreenBorder = 5;
  DeviceSettings.MinimumPixelSize = 15;
  DeviceSettings.MinWidth = 5;
  DeviceSettings.MinHeight = 5;

  DeviceSettings.MaxWidth = ((DeviceSettings.ScreenWidth -
			      DeviceSettings.ScreenBorder) / 
			     DeviceSettings.MinimumPixelSize);
  DeviceSettings.MaxHeight = ((DeviceSettings.ScreenWidth -
			      DeviceSettings.ScreenHeader -
			      DeviceSettings.ScreenBorder) /
			     DeviceSettings.MinimumPixelSize);
  if (DeviceSettings.MaxWidth > MAX_SIZE_WIDTH)
    DeviceSettings.MaxWidth = MAX_SIZE_WIDTH;
  if (DeviceSettings.MaxHeight > MAX_SIZE_HEIGHT)
    DeviceSettings.MaxHeight = MAX_SIZE_HEIGHT;
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
  //FreeBoard(&Game.theBoard);
  if (DeviceSettings.RomVersion >= RomOS35)
    WinPopDrawState();
  FrmCloseAllForms();
}

static void Ctl_SetVal(FormPtr frm, UInt16 Id, Int16 value) {
  ControlType *ctl = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, Id));
  CtlSetValue(ctl, value);
}
static Int16 Ctl_GetVal(FormPtr frm, UInt16 Id) {
  ControlType *ctl = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, Id));
  return (CtlGetValue(ctl));
}

static Boolean PreferencesEventHandler (EventPtr eventP) {
  Boolean handled = false;
  FormPtr frm = FrmGetActiveForm ();

  if (eventP->eType == ctlSelectEvent) {
    UInt16 cId = eventP->data.ctlSelect.controlID;
    if (cId == HeightDownArrow || cId == HeightUpArrow ||
	cId == WidthDownArrow || cId == WidthUpArrow) {
      prefsTouched = true;
      //Begin Height section
      if (cId == HeightDownArrow || cId == HeightUpArrow) {
	if (cId == HeightDownArrow) {
	  if (tempPreferences.Height > DeviceSettings.MinHeight) {
	    tempPreferences.Height -= 1;
	  }
	}
	else if (cId == HeightUpArrow &&
		 tempPreferences.Height < DeviceSettings.MaxHeight) {
	  tempPreferences.Height += 1;
	}
      }//End Height Section
      //Begin width section
      else if (cId == WidthDownArrow || cId == WidthUpArrow) {
	if (cId == WidthDownArrow &&
	    tempPreferences.Width > DeviceSettings.MinWidth) {
	  tempPreferences.Width -= 1;
	}
	else if (cId == WidthUpArrow &&
		 tempPreferences.Width < DeviceSettings.MaxWidth) {
	  tempPreferences.Width += 1;
	}
      }//End Width Section


      {
	UInt16 widthUp, widthDown, heightUp, heightDown;
	Char HeightAsStr[maxStrIToALen], WidthAsStr[maxStrIToALen];
	//This sets the up/down scroll buttons
	widthUp = FrmGetObjectIndex(frm, WidthUpArrow);
	widthDown = FrmGetObjectIndex(frm, WidthDownArrow);
	heightUp = FrmGetObjectIndex(frm, HeightUpArrow);
	heightDown = FrmGetObjectIndex(frm, HeightDownArrow);
	//I learned how to do this in the addressbook's function
	//PreferencesUpdateScrollers which lives in DataPref.c
	FrmUpdateScrollers (frm, widthUp, widthDown,
			    tempPreferences.Width<DeviceSettings.MaxWidth,
			    tempPreferences.Width>DeviceSettings.MinWidth);
	FrmUpdateScrollers (frm, heightUp, heightDown,
			    tempPreferences.Height<DeviceSettings.MaxHeight,
			    tempPreferences.Height>DeviceSettings.MinHeight);
	//This sets the labels of height/width selectors
	StrIToA(HeightAsStr, tempPreferences.Height);
	StrIToA(WidthAsStr, tempPreferences.Width);
	
	FrmCopyLabel(FrmGetActiveForm(), HeightLabel, HeightAsStr);
	FrmCopyLabel(FrmGetActiveForm(), WidthLabel, WidthAsStr);
      }
      handled = true;
    }
    else if (eventP->data.ctlSelect.controlID == PreferencesOKButton) {
      //The Yes button returns 1 from this frmalert. (The No button returns 0)
      //Change this so that prefs touched isn't used --
      //make it so that it compares the current gamesettings to the
      //temp preferences.
      if (!prefsTouched) {
	FrmReturnToForm (0);
	handled = true;	
      }
      else if (FrmAlert(NewGameAlert) == 0) {
	GameSettings *gs = &Game.theBoard.g;

	gs->height = tempPreferences.Height;
	gs->width = tempPreferences.Width;
	gs->showPossible = Ctl_GetVal(frm, ShowPossibleMoves);

	FreeBoard(&Game.theBoard);
	newGame();

	FrmUpdateForm(MainForm, frmRedrawUpdateCode);
	FrmReturnToForm (0);
	handled = true;
      }
      else
	handled = false;
    }
    else if (eventP->data.ctlSelect.controlID == PreferencesCancelButton) {
      FrmReturnToForm (0);
      handled = true;
    }
    else {
      prefsTouched = true;
    }
  }
  else if (eventP->eType == frmOpenEvent) {
    //GameSettings *gs = &Game.theBoard.g;
    
    //ControlType *ctl;
    //Char *label;
    //ListType *listP;

    prefsTouched = false;
    tempPreferences.Height = Game.theBoard.g.height;
    tempPreferences.Width = Game.theBoard.g.width;
    Ctl_SetVal(frm, ShowPossibleMoves, Game.theBoard.g.showPossible);
    

    FrmDrawForm (frm);
    {
      Char HeightAsStr[maxStrIToALen], WidthAsStr[maxStrIToALen];
      StrIToA(HeightAsStr, tempPreferences.Height);
      StrIToA(WidthAsStr, tempPreferences.Width);
      
      FrmCopyLabel(FrmGetActiveForm(), HeightLabel, HeightAsStr);
      FrmCopyLabel(FrmGetActiveForm(), WidthLabel, WidthAsStr);
    }    
    handled = true;
  }
  return (handled);
}

static Boolean MainMenuHandleEvent(UInt16 menuID) {
  Boolean    handled = false;
  //FormType   *form = FrmGetActiveForm();
  
  switch (menuID) {
  case MainGamePreferences:
    FrmPopupForm(PreferencesForm);
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
    //OK button is 0, cancel is 1. (and the default)
    if (FrmAlert(NewGameAlert) == 0) {
      FreeBoard(&Game.theBoard);
      newGame();
      updateMoveCounterDisplay(Game.numMoves);
    }
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
      updateMoveCounterDisplay(Game.numMoves);
    }
    handled = true;
    break;
  case ctlSelectEvent:
    switch (event->data.ctlSelect.controlID) {
    case MainFormNewButton:
      //OK button is 0, cancel is 1. (and the default)
      if (FrmAlert(NewGameAlert) == 0) {
	FreeBoard(&Game.theBoard);
	newGame();
	updateMoveCounterDisplay(Game.numMoves);
	//This isn't needed cause it calls frmupdate event.
	DrawSquares(&Game.theBoard);
      }
      //else {
      //DrawSquares(&Game.theBoard);
      //}
      handled = true;
      break;
    //Redraw
    case MainFormRDButton:
      //Game.skillLevel = difficultyIntermediate;
      DrawSquares(&Game.theBoard);
      handled = true;
      break;
    //Undo
    case MainFormUDButton:
      MoveUndo(&Game, 1);
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
	  DrawSelection(&Game.theBoard, boardIndex);
	  //DrawBox(&Game.theBoard, boardIndex, false);
	  Game.theBoard.pieceSelected = boardIndex;
	  //if (Game.theBoard.g.showPossible) {
	  if (true) {
	    DrawPossibleMoves(&Game, boardIndex);
	  }
	}
      }
      //If the same piece has been clicked again, deselect that piece
      else if (boardIndex == Game.theBoard.pieceSelected) {
	DrawSelection(&Game.theBoard, boardIndex);
	//DrawBox(&Game.theBoard, boardIndex, false);
	Game.theBoard.pieceSelected = -1;
	//This is a little excessive (an entire redraw), but it works now..
	DrawSquares(&Game.theBoard);
      }
      else {
	DrawSelection(&Game.theBoard, Game.theBoard.pieceSelected);
	//DrawBox(&Game.theBoard, Game.theBoard.pieceSelected, false);
	if (move(&Game, Game.theBoard.pieceSelected, boardIndex)) {
	  Game.theBoard.pieceSelected = -1;
	  if (!(anyValidMoves(&Game))) {
	    //This means the game is over;
	    if (FrmAlert(NewGameAlert) == 0) {
	      FreeBoard(&Game.theBoard);
	      newGame();
	      updateMoveCounterDisplay(Game.numMoves);
	    }
	  }
	}
	DrawSquares(&Game.theBoard);
      }
      handled = true;
    }
    //the pen down event is not within the board.
    else {
    }
    break;
  case frmUpdateEvent:
    {
      RectangleType r;
      r.topLeft.x = 0;
      r.topLeft.y = 0;
      r.extent.x = DeviceSettings.ScreenWidth;
      r.extent.y = DeviceSettings.ScreenHeight;
      WinEraseRectangle(&r, 0);
      //This is needed so that the buttons and the menu get redrawn properly
      FrmDrawForm(form);
      //This actually draws the squares
      DrawSquares(&Game.theBoard);
      updateMoveCounterDisplay(Game.numMoves);
      //You must return true, otherwise the caller erases the form
      //(and everything performed above), and calls FrmDrawForm itself.
      handled = true;
      break;
    }
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
    case PreferencesForm:
      FrmSetEventHandler(form, PreferencesEventHandler);
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
    if (Game.theBoard.g.showPossible) {
	EvtGetEvent(&event, 1000);
    }
    else {
      EvtGetEvent(&event, evtWaitForever);
    }
    if (! SysHandleEvent(&event))
      if (! MenuHandleEvent(0, &event, &error))
	if (! ApplicationHandleEvent(&event))
	  FrmDispatchEvent(&event);
    if (Game.theBoard.blinking) {
      if (Game.theBoard.blinkOn) {
	//blink;
      }
      else {
	//turn off the blink.
      }
      Game.theBoard.blinkOn = !(Game.theBoard.blinkOn);
    }
  } while (event.eType != appStopEvent);
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
