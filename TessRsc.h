//wierd characters
#define LEFTARROW		'0x03'
#define RIGHTARROW		'0x04'

// Main form
#define MainForm		1000
#define	PreferencesForm		1001
#define	BoardEditForm		1002

#define MainFormNewButton	2000
#define MainFormRDButton	2003
#define MainFormUDButton	2004
#define	PreferencesOKButton	2001
#define	PreferencesCancelButton	2002
// Menubar
#define MainMenuBar		1000

// Menu commands
#define MainGameNew		1100
#define MainGameUndo		1103
#define MainGameInstructions	1101
#define	MainGamePreferences	1102
#define MainOptionsAbout	1200

// Alerts
#define AboutAlert		7000
#define	NewGameAlert		7001
#define ErrorAlert		7002
#define RomIncompatibleAlert	7003

#define InstructionsStr		3000


#define	HeresTheInfo		4000

#define	ShowPossibleMoves	6001
#define	HeightTrigger		6002
#define	HeightList		6003
#define	WidthTrigger		6004
#define	WidthList		6005

#define HeightRightArrow	6006

#define	BoardEditOKButton	6101
#define	BoardEditCancelButton	6102

#define	HeightUpArrow		6103
#define	HeightDownArrow		6104

//Warning! Do not use IDs > 9000 (that's where PilRC's AUTOID elements start)


//Non Resource ID Constants (Only used for preprocessing)
//Alignment of colon (on right side of text) on pref panel
#define ColonAlign 60
//Space between the two lines
#define SpaceApart 50
//Vertical centering line of the top line of text
#define TopCenter 55
#define BotCenter TopCenter + SpaceApart
//How far 
#define TilesAlign ColonAlign + 15
