/*************************************************
 Tesserae
 Original Concept by Kent Brewster
 (http://www.sff.net/people/k.brewster/)
 in his game Stained Glass
**************************************************/
#include <PalmOS.h>
#include "TessRsc.h"

#define ROMVersionMinimum				0x02000000
#define RomOS35						0x03500000
#define LEVEL_BEGINNER 1
#define LEVEL_INTERMEDIATE 2
#define LEVEL_ADVANCED 3

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

#define ATTRIBUTES_MASK 0x07
#define GETATTRIBUTES(x) (x & ATTRIBUTES_MASK)
#define ONBOARD_MASK 0x08
#define ONBOARD(x) (x & ONBOARD_MASK) >> 3
#define ISPOWEROF2(x) (0 == (x&(x-1)))

typedef UInt8 Square;

typedef struct Board {
  Square *board;	//You must allocate the necessary memory for board
  UInt8 width;		//of size width*height
  UInt8 height;
  UInt16 size;		//Bastard.
  UInt8 squareWidth;
  UInt8 squareHeight;
  UInt8 skillLevel;
  RectangleType rect;
  Boolean squarePieces;
  Int16 pieceSelected;
  Int32 blinkRate;
  Int32 timeToBlink;
  Boolean blinking;
  Boolean blinkOn;
} Board;

//Begin GLobal Variables
static Board bb;
//Think binary counting here. the fourth 
static const UInt32 colorsHex[8] = {WHITE,YELLOW,BLUE,GREEN,RED,ORANGE,PURPLE,GRAY};
//colors for each of the possible tiles.
static IndexedColorType colors[8];
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

static UInt16 RandomNum(UInt16 n) {
  //this is ghetto, but fixes divide by zero issues should they arise.
  if (n == 0)
    return 0;
  else
    return SysRandom(0) / (1 + sysRandomMax / n);
}

static Int16 GetIndex(Board *b, UInt8 x, UInt8 y) {
  if ((x < b->width) && (y < b->height))
    return (x + (b->width * y));
  else
    return -1;
}

static PointType GetXY(Board *b, UInt16 Index) {
  PointType p;
  if (Index < b->size && Index >= 0) {
    p.x = (Index % b->width);
    p.y = (Index / b->width);
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

static void FillBoardRandom(Board *b) {
  //Level 0 means all primary color tiles
  //Level 1 means primary and secondary color tiles
  //Level 2 means primary, secondary and gray tiles.
  UInt16 i, tempNum;
  for (i = 0; i < (b->width * b->height);i++) {
    if (ONBOARD(b->board[i])) {
      //Clears the attribute bits that might have been set 
      b->board[i] = ((b->board[i] >> 3) << 3);
      //This gives us a number between 1 and 7 (0 would be invalid cause there
      //would be no tile there.
      if (b->skillLevel == LEVEL_BEGINNER) {
	tempNum = RandomNum(3);
	if (tempNum == 0) {
	  b->board[i] += tempNum + 1;
	}
	else if (tempNum == 1) {
	  b->board[i] += tempNum + 1;
	}
	else if (tempNum == 2) {
	  b->board[i] += tempNum + 2;
	}
	else
	  return;
      }
      else if (b->skillLevel == LEVEL_INTERMEDIATE) {
	b->board[i] += RandomNum(6) + 1;
      }
      else {
	b->board[i] += RandomNum(7) + 1;
      }
    }
  }
}

static Board MakeBoard(UInt8 width, UInt8 height, Boolean squarePieces) {
  Board b;
  UInt16 i;
  b.width = width;
  b.height = height;
  b.squarePieces = squarePieces;
  //The -1 at the end is to take the lines between cells into account.
  b.squareWidth = (DeviceSettings.ScreenWidth - 
		   (DeviceSettings.ScreenBorder * 3)) / (b.width) -1;
  b.squareHeight = (DeviceSettings.ScreenHeight - DeviceSettings.ScreenHeader -
		    (DeviceSettings.ScreenBorder * 3)) / (b.height) -1;
  if (b.squareWidth % 2 != 1)
    b.squareWidth--;
  if (b.squareHeight % 2 != 1)
    b.squareHeight--;
  //Make the pieces square if need be.
  if (b.squarePieces) {
    if (b.squareHeight >= b.squareWidth)
      b.squareHeight = b.squareWidth;
    else
      b.squareWidth = b.squareHeight;
  }
  b.rect.topLeft.x = ((DeviceSettings.ScreenWidth -
		    (DeviceSettings.ScreenBorder * 2) -
		    ((b.squareWidth) * b.width)) /2);
  b.rect.topLeft.y = (((DeviceSettings.ScreenHeight -
		     (DeviceSettings.ScreenBorder * 2) -
		     DeviceSettings.ScreenHeader -
		     (b.squareHeight * b.height)) /2) +
		   DeviceSettings.ScreenHeader);
  b.size = width * height;
  b.board = MemHandleLock(MemHandleNew(sizeof(Square) * b.size));
  for (i = 0; i < b.size; i++) {
    //Sets the bit in b.board[i] to make it a usable part of the board
    b.board[i] = ONBOARD_MASK;
  }
  return b;
}

static void FreeBoard(Board *b) {
  if (b->size) {
    if (MemPtrRecoverHandle(b->board)) {
      MemHandleFree(MemPtrRecoverHandle(b->board));
      //Clear any other contents so it won't be used again.
      MemSet(b, sizeof(Board), 0x00);
    }
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
static void DrawOneSquare(Board *b, PointType p, Square s) {
  RectangleType color, r;
  IndexedColorType oldBGIndex;
  //The +1 and -3 keep it within the bounds of the rectangle.
  r.topLeft.x = p.x +2;
  r.topLeft.y = p.y +2;
  r.extent.x = b->squareWidth -5;
  r.extent.y = b->squareHeight -5;
  color.topLeft = p;
  color.extent.x = b->squareWidth;
  color.extent.y = b->squareHeight; 
  oldBGIndex = WinSetBackColor(colors[(GETATTRIBUTES(s))]);
  WinEraseRectangle(&color, 0);
  WinSetBackColor(oldBGIndex);
  if ((s >> 0) %2) {    
    DrawPlusInRect(r);
  }
  if ((s >> 1) %2) {
    DrawSquareInRect(r);
  }
  if ((s >> 2) %2) {
    DrawCircleInRect(r);
  }
}

static void DrawALoc(Board *b, UInt16 loc){
  Square s = b->board[loc];
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
  if (index != bb.pieceSelected) {
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
    //b->board[pieceSelected]
  }
  else {
    DrawALoc(b, index);
  }
}
static void DrawSquares(Board *b) {
  FrameType frame = rectangleFrame;
  Int16 i, j;
  RectangleType r;
  
  //If these extents were one less we wouldn't get that cool shadow effect.
  b->rect.extent.x = (b->squareWidth+1) * b->width;
  b->rect.extent.y = (b->squareHeight+1) * b->height;
  WinEraseRectangle(&b->rect, 0);
  WinDrawRectangleFrame(frame, &b->rect);

  r.extent.x = b->squareWidth;
  r.extent.y = b->squareHeight;
    for (i = 0; i < b->width; i++) {
    //Set the horizontal position
    r.topLeft.x = b->rect.topLeft.x + (i * (b->squareWidth + 1));
    for (j = 0; j < b->height; j++) {
      //Set the vertical position
      r.topLeft.y = b->rect.topLeft.y + (j * (b->squareHeight + 1));
      WinDrawRectangleFrame(frame, &r);
      if (ONBOARD(b->board[GetIndex(b, i, j)])) {
	DrawOneSquare(b, r.topLeft, b->board[GetIndex(b, i, j)]);
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
    
    newSrc = GETATTRIBUTES(b->board[src]);
    newMid = GETATTRIBUTES(b->board[mid]);
    newDest = GETATTRIBUTES(b->board[dest]);
    
    if ((newMid) &&
		   ((((ISPOWEROF2(newSrc)) &&
		      (ISPOWEROF2(newMid))) ||
		     ((newSrc & newMid) == newSrc)) &&
		    ((newSrc == newDest) || ((newSrc & newDest) == 0)))) {
    
      newMid = (ONBOARD_MASK |
		(((ISPOWEROF2(newSrc)) && (ISPOWEROF2(newMid))) ? 0 :
		 (newMid & (~newSrc))));
      newDest = (ONBOARD_MASK | (newSrc | newDest));
      newSrc = ONBOARD_MASK;
      
      b->board[src] = newSrc;
      b->board[mid] = newMid;
      b->board[dest] = newDest;
      
      DrawALoc(b, src);
      DrawALoc(b, mid);
      DrawALoc(b, dest);
      return(true);
    }
  }
  b->pieceSelected = -1;
  return false;
}
static void newGame() {
  bb = MakeBoard(8, 7, false);
  bb.skillLevel = LEVEL_BEGINNER;
  bb.pieceSelected = -1;
  bb.blinkRate = SysTicksPerSecond() / 2;
  bb.blinking = false;
  bb.blinkOn = false;
  FillBoardRandom(&bb);
  DrawSquares(&bb);
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
  if (err) return (err);
  setupDeviceSettings();
  FrmGotoForm(MainForm);
  return 0;
}


static void StopApplication(void) {
  FreeBoard(&bb);
  if (DeviceSettings.RomVersion >= RomOS35)
    WinPopDrawState();
  FrmCloseAllForms();
}

static Boolean MainMenuHandleEvent(UInt16 menuID) {
    Boolean    handled = false;
    //FormType   *form = FrmGetActiveForm();
    
    switch (menuID) {
    case MainOptionsAbout:
      FrmAlert(AboutAlert);
      handled = true;
      break;  
    case MainGameInstructions:
      FrmHelp(InstructionsStr);
      handled = true;
      break;  
    case MainGameNew:
      FreeBoard(&bb);
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
  UInt16 boardIndex;

  switch (event->eType) {
  case frmOpenEvent:
    FrmDrawForm(form);
    handled = true;
    break;
  case ctlSelectEvent:
    switch (event->data.ctlSelect.controlID) {
    case MainFormNewButton:
      FreeBoard(&bb);
      newGame();
      handled = true;
      break;
    default:
      break;
    }
    break;
    case penDownEvent:
    boardIndex = ScreenCoordToBoardIndex(&bb, event->screenX, event->screenY);
    if (-1 != boardIndex) {
        //If there is no piece selected, select the piece.
      if (-1 == bb.pieceSelected) {
	if (GETATTRIBUTES(bb.board[boardIndex]) != 0x00) {
	  DrawBox(&bb, boardIndex);
	  bb.pieceSelected = boardIndex;
	}
      }
      //If the same piece has been clicked again, deselect that piece
      else if (boardIndex == bb.pieceSelected) {
	DrawBox(&bb, boardIndex);
	bb.pieceSelected = -1;
      }
      else {
	DrawBox(&bb, bb.pieceSelected);
	if (move(&bb, bb.pieceSelected, boardIndex)) {
	  //DrawBox(&bb, boardIndex);
	  bb.pieceSelected = -1;
	}
      }
      handled = true;
    }
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
  default:
    break;
  }
  return err;
}
