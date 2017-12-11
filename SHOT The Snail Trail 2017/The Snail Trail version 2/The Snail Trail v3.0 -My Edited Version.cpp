//THE SNAIL TRAIL GAME - Version 2.0 - Instrumented Version plus Automated Input & De-Randomised
// This version 2.0 is used for:
// - running the same game moves repeatedly to allow for comparisons as improvements are made,
// - uses key presses previously saved to 'SnailTrailMoves.csv' file for automated game play (see version 1.0).
// Note to watch the game step by step, use a breakpoint in the main game loop and use F5/Continue in the debugger.

// Other changes in this version:
// (1) - Replaced bleeps with text to keep the noise levels down.
// (2) - Addition of the test file input for "SnailTrailMoves_FOR_DEMO_USE_999_SEED.csv", see openFiles function.
//       HEALTH WARNING - running the test file makes the console display flicker and flash a great deal.


// This version includes the version 1.0 changes:
// - saving timing data to a file, 'SnailTrailTimes.csv', not just the screen.
// - producing averages for the various timing data.
// - saving key presses to a 'SnailTrailMoves.csv' file for subsequent automated game play.

bool record_Every_Step(false);						//NEW 'true' = record all details, 'false' = summary only

#define studentName     "Thomas Clark"				//NEW Please change this as appropriate!


// Comments showing 'NEW' are on lines additional to the original Snail Trail version 0.
// Comments showing 'NEW2' are on lines additional to the original Snail Trail version 1.
//***************************************************************************************************************

// A.Oram 2017, based on the P.Vacher's skeletal program from first year FoP.
/*
A snail (the player) moves about the garden leaving a trail of slime that it doesn't like to cross, but this does dissolve in due course.
Each move absorbs a small amount of energy (health) that needs to be topped up.
Some swine has scattered a number of hard-to-see slug pellets around and if the snail slithers over these it's health declines a little.
The garden has frogs that leap about trying to make lunch of the snail, but may jump over it by accident. They may incidentally
soak up a slug pellet, thus helping the snail stay alive.
If they land on the snail or the snail runs into a frog, it's curtains!
Frogs also have a certain % chance of being carried off by a hungry eagle, so the snail may live to see another day.
To win eat all the lettuces - stay above 0% health by eating low-carb lettuces or high carb worms.
Health less than zero is death!

This code is certainly snail-like, can you get the frame rate up?

Screenshot of console during gameplay (mono colour!):
___________________________________________________________________________
      ...THE SNAIL TRAIL...
      DATE: 30/10/2017
++++++++++++++++++++++++++++++   TIME: 17:18:18
+                          - +
+              --            +
+   @~                       +   Game Timings:
+     ~                    @ +   Initialise game= 35.63743 us
+                ...         +   Paint Game=      56.04570 ms
+               ..  X        +
+           ~-  .      ...&M +   Frames/sec=      17.748 at 56.34363 ms/frame
+               .. .....     +
+                ...         +
+                            +
+       -                    +
+    @                       +   Instructions
+        -                   +   * TO MOVE USE ARROW KEYS - EAT ALL 6 LETTUCES TO WIN.
+                     -      +   * EAT WORMS (~) AND LETTUCES (@) TO BOOST HEALTH.
+ ~    ~     -    @         -+   * EACH MOVE AND INVISIBLE SLUG PELLETS DEPLETE HEALTH.
+       ~ @                  +   * TO QUIT ANY TIME USE 'Q'
+           -           -   -+
+        ~              @   -+
++++++++++++++++++++++++++++++
      Health: 47.10%
      PELLET ALERT!
__________________________________________________________________________

x co-ordinates follow this pattern (y co-ordinates similar):

0,1,2 ...    ...SIZEX-2, SIZEX-1

Garden interior extends 1..SIZEX-2, 1..SIZEY-2.
Walls are at 0 and SIZEX-1, 0 and SIZEY-1.

Annoying bleeps from the PC's speaker announce significant events, check the message shown to see what happened.

*/

//---------------------------------
//include libraries
//include standard libraries
//#include <iostream>          //for output and input
#include <iomanip>           //for formatted output in 'cout'
//#include <conio.h>           //for getch
#include <fstream>           //for files
#include <string>            //for string
#include "hr_time.h"         //for timers
//#include <stdio.h>

using namespace std;

//include our own libraries
#include "RandomUtils.h"     //for Seed, Random,
#include "ConsoleUtils.h"    //for Clrscr, Gotoxy, etc.
#include "TimeUtils.h"       //for GetTime, GetDate, etc.

// global constants

// garden dimensions
const int SIZEY(20);					// vertical dimension
const int SIZEX(30);					// horizontal dimension

										//constants used for the garden & its inhabitants
const char SNAIL('&');					// snail (player's icon)
const float LIFE_SPAN(1.0);				// Snail's LIFE_SPAN starts full!
const float ENERGY_USED(LIFE_SPAN / 30.0);// each move uses this much energy (30 moves with no food and you're dead!)
const char DEADSNAIL('o');				// just the shell left...
const char GRASS(' ');					// open space
const char WALL('+');                   // garden wall

const char  WORM('~');					// worms are high energy food = 50% energy!
const float WORM_ENERGY(LIFE_SPAN / 2.0);	// eat worms to keep living in the fast lane!
const int   WORM_COUNT(8);				// how many worms are available

const char SLIME('.');					// snail produce
const int  SLIMELIFE(20);				// how long slime lasts (in keypresses)

const char PELLET('-');					// should be invisible, but for testing using a visible character.
const int  NUM_PELLETS(15);				// number of slug pellets scattered about
const float PELLET_POISON(LIFE_SPAN / 10.0); // each pellet saps a bit of health too (10% of what remains)

const char LETTUCE('@');				// a lettuce
const int  LETTUCE_QUOTA(6);			// how many lettuces you need to eat before you win.
const float LETTUCE_ENERGY(LIFE_SPAN / LETTUCE_QUOTA);		// energy available from eating a lettuce (less than a worm!)

const int  NUM_FROGS(2);
const char FROG('M');
const char DEAD_FROG_BONES('X');		// Dead frogs are marked as such in their 'y' coordinate for convenience
const int  FROGLEAP(5);					// How many spaces do frogs jump when they move (setting to 1 = short life for snail!)
const float  EagleStrike(0.33f);		// There's a 1 in 30 chance of an eagle strike on a frog

										// the keyboard arrow codes
const int UP(72);						// up key
const int DOWN(80);						// down key
const int RIGHT(77);					// right key
const int LEFT(75);						// left key

										// other command letters
const char QUIT('q');					//end the game
//const char Bleep('\a');				// annoying Bleep
//string Bleeeep("\a\a");				// very annoying Bleeps
//const char Bleep('B');				// NEW2 Silent Version ! annoying Bleep
//string Bleeeep("BB");				// NEW2 Silent Version ! very annoying Bleeps

const int LEFTM(SIZEX + 3);				//define left margin for messages (avoiding garden)

										//define a few global control constants for our snail
int	  snailStillAlive(true);			// snail starts alive!
float lifeLeft(LIFE_SPAN);				// Life starts at 100%, need to eat lettuces and worms to live
int   lettucesEaten(0);					// win when this reaches LETTUCE_QUOTA
bool  fullOfLettuce(false);				// when full and alive snail has won!

CStopWatch	InitTime,
FrameTime,
PaintTime;					// create stopwatchs for timing

//*******************************************************************
// NEW Declarations dealing with instrumentation and saving game data

ofstream ST_Times;		//NEW
ofstream ST_Moves;		//NEW
ifstream ST_PlayList;   //NEW2	- play from a recorded series of moves.

bool InitTimesAlreadySaved(FALSE); //NEW
char moveResult(0);		//NEW saves result of snail's last  move for output file
char gameEvent(0);		//NEW saves any event such as Frog being eaten by eagle for output to file.
const char WIN('W');	//NEW
const char STUCK('S');	//NEW

float InitTimeTotal(0.);	//NEW
float FrameTimeTotal(0.);	//NEW
float PaintTimeTotal(0.);	//NEW
int GamesPlayed(0);			//NEW
int TotalMovesMade(0);		//NEW

//*******************************************************************


// Start of the 'SNAIL TRAIL' listing
//---------------------------------
int __cdecl main()
{
	//function prototypes
	void showTitle(int, int);
	void showDate(int, int);
	void showTimingHeadings(int, int);
	void showOptions(int, int);

	void initialiseGame(int&, bool&, int[], int[][2], char[][SIZEY][SIZEX]);
	void paintGame(string message, char[][SIZEY][SIZEX]);
	void clearMessage(string& message);

	int getKeyPress();
	void analyseKey(string& message, int, int move[2]);
	void moveSnail(int[], int[], string&, char[][SIZEY][SIZEX]);
	void moveFrogs(int[], int[][2], string&, char[][SIZEY][SIZEX]);
	void placeSnail(char[][SIZEY][SIZEX], int[]);
	void dissolveSlime(char[][SIZEY][SIZEX]);
	void showFood(char[][SIZEY][SIZEX]);

	int anotherGo(int, int);

	// Timing info
	void showTimes(float, float, float, int, int);
	void saveData(float, float, float, int, string, float);	//NEW
	void openFiles(void);										//NEW


	//local variables
	//arrays that store ...
	char garden[2][SIZEY][SIZEX];			// the game 'world'
	//char slimeTrail[SIZEY][SIZEX];		// lifetime of slime counters overlay
	//char foodSources[SIZEY][SIZEX];		// remember where the lettuces are planted and worms are

	string message;							// various messages are produced in game.
	int  snail[2];							// the snail's current position (x,y)
	int  frogs[NUM_FROGS][2];				// coordinates of the frog contingent n * (x,y)
	int  move[2];							// the requested move direction

	int  key, newGame(!QUIT);				// start new game by not quitting initially!

											// Now start the game...

	//Seed();									//seed the random number generator
	srand(999);								//NEW2 - Use same seed number as that used to generate the 'Moves' file to ensure
											//NEW2   the same 'random' event sequences occur.
	openFiles();							//NEW - open files for timers and automated gameplay


	// ******************************** Main Game Loop **************************************
	while ((newGame | 0x20) != QUIT)		// keep playing games
	{
		Clrscr();
		InitTimesAlreadySaved = FALSE;		// NEW - only output Init timing once per game

		showTitle(5, 0);				// display game title
		showDate(LEFTM, 1);		// display system clock date
		showTimingHeadings(LEFTM, 5);	// display Timings Heading
		showOptions(LEFTM, 14);		// display menu options available

		// ************** code to be timed ***********************************************
		InitTime.startTimer();

		//initialise garden (incl. walls, frogs, lettuces & snail)
		initialiseGame(lettucesEaten, fullOfLettuce, snail, frogs, garden);
		message = "READY TO SLITHER!? PRESS A KEY...\t\t";

		InitTime.stopTimer();
		// *************** end of timed section ******************************************

		paintGame(message, garden);			//display game info, garden & messages
		key = getKeyPress();				//get started or quit game


		// ******************************** Frame Loop **************************************
		while (((key | 0x20) != QUIT) && snailStillAlive && !fullOfLettuce)	//user not bored, and snail not dead or full
		{
			FrameTime.startTimer(); // not part of game

			// ************** code to be timed ***********************************************

			analyseKey(message, key, move);				// get next move from keyboard
			moveSnail(snail, move, message, garden);
			dissolveSlime(garden);			// remove slime over time from garden
			showFood(garden);				// show remaining lettuces and worms on ground
			placeSnail(garden, snail);					// move snail in garden
			moveFrogs(snail, frogs, message, garden);	// frogs attempt to home in on snail


			FrameTime.stopTimer(); // you should eventually uncomment this and comment out the identical line 4 lines down

			paintGame(message, garden);				// display game info, garden & messages
			//clearMessage(message);						// reset message array. NEW need to comment this out

			// *************** end of timed section ******************************************

			//FrameTime.stopTimer(); // not part of game

			showTimes(InitTime.getElapsedTime(), FrameTime.getElapsedTime(), PaintTime.getElapsedTime(), LEFTM, 6);
			
			// NEW Save performance data outside of game loop
			saveData(InitTime.getElapsedTime(), FrameTime.getElapsedTime(), PaintTime.getElapsedTime(), key, message, lifeLeft); //NEW

			clearMessage(message);						// NEW reset message array here, moved from above (so we can use it above in saveTimes)
			InitTimesAlreadySaved = TRUE;				// NEW prevent repeated saving of Init timing


			key = getKeyPress();;						// display menu & read in next option
			//system("pause");
		}

		// ******************************** End of Frame  Loop **************************************

		//							If alive...								If dead...
		(snailStillAlive) ? message = "WELL DONE, YOU WON!" : message = "REST IN PEAS.";

		if (!snailStillAlive) garden[0][snail[0]][snail[1]] = DEADSNAIL;
		paintGame(message, garden);					// display final game info, garden & message

		newGame = anotherGo(LEFTM, 20);					// Prompt to play again, or Quit game.
		//system("pause");

	} 	// ******************************** End of Main Game Loop **************************************
	
	//NEW Report final frame time (only the paintGame time will be different from last set as frame won't be recalculated).
	saveData (InitTime.getElapsedTime(), FrameTime.getElapsedTime(), PaintTime.getElapsedTime(), key, message, lifeLeft); //NEW
	
	//system("pause");
	return 0;
} //end main


  // FUNCTION DEFINITIONS //////////////////////////////////////////////////////////////////////////////////

  //**************************************************************************
  //													set game configuration

void initialiseGame(int& Eaten, bool& fullUp, int snail[], int frogs[][2], char garden[][SIZEY][SIZEX])
{ //initialise garden & place snail somewhere


	void setGarden(char[][SIZEY][SIZEX]);
	void setSnailInitialCoordinates(int[]);
	void placeSnail(char[][SIZEY][SIZEX], int[]);
	void initialiseSlimeTrailAndFood(char[][SIZEY][SIZEX]);
	void showFood(char[][SIZEY][SIZEX]);
	void scatterStuff(char[][SIZEY][SIZEX], int[]);
	void scatterFrogs(char[][SIZEY][SIZEX], int[], int[][2]);

	snailStillAlive = true;					// bring snail to life!

	setSnailInitialCoordinates(snail);		// initialise snail position
	setGarden(garden);					// reset the garden
	placeSnail(garden, snail);			// place snail at a random position in garden
	initialiseSlimeTrailAndFood(garden);		// no slime until snail moves & lettuces not placed yet
	scatterStuff(garden, snail);	// randomly scatter stuff about the garden (see function for details)
	showFood(garden);			// show lettuces on ground
	scatterFrogs(garden, snail, frogs);		// randomly place a few frogs around
	

	lifeLeft = LIFE_SPAN;					// reset life span (health)
	Eaten = 0;								// reset number of lettuces eaten
	fullUp = false;							// snail is hungry again
}

//**************************************************************************
//												randomly drop snail in garden
void setSnailInitialCoordinates(int snail[])
{ //set snail's coordinates inside the garden at random at beginning of game

	snail[0] = Random(SIZEY - 2);		// vertical coordinate in range [1..(SIZEY - 2)]
	snail[1] = Random(SIZEX - 2);		// horizontal coordinate in range [1..(SIZEX - 2)]
}

//**************************************************************************
//						set up garden array to represent grass and walls

void setGarden(char garden[][SIZEY][SIZEX])
{ //reset to empty garden configuration

	for (int row(0); row < SIZEY; ++row)
	{
		for (int col(0); col < SIZEX; ++col)
		{
			garden[0][row][col] = GRASS;				// grow some 'grass'
			if ((row == 0) || (row == SIZEY - 1))	// insert top or bottom walls where needed
				garden[0][row][col] = WALL;
			if ((col == 0) || (col == SIZEX - 1))	// insert left & right walls where needed
				garden[0][row][col] = WALL;
		}
	}
} //end of setGarden

  //**************************************************************************
  //														place snail in garden
void placeSnail(char garden[][SIZEY][SIZEX], int snail[])
{ //place snail at its new position in garden

	garden[0][snail[0]][snail[1]] = SNAIL;
} //end of placeSnail

  //**************************************************************************
  //												slowly dissolve slime trail

void dissolveSlime(char garden[][SIZEY][SIZEX])
{// go through entire slime trail and decrement each item of slime in order

	for (int y = 1; y < SIZEY - 1; y++)
		for (int x = 1; x < SIZEX - 1; x++)
		{
			if (garden[2][y][x] <= SLIMELIFE && garden[2][y][x] > 0)	// if this bit of slime exists
			{
				garden[2][y][x] --;									// then dissolve slime a little.
				if (garden[2][y][x] == 0)								// if totally dissolved then
					garden[0][y][x] = GRASS;								// then remove slime from garden
			}
		}
}

//**************************************************************************
//											show available food on the garden
void showFood(char garden[][SIZEY][SIZEX])
{
	for (int y = 1; y < SIZEY - 1; y++)
		for (int x = 1; x < SIZEX - 1; x++)
		{
			if (garden[1][y][x] == WORM) garden[0][y][x] = WORM;
			if (garden[1][y][x] == LETTUCE) garden[0][y][x] = LETTUCE;
		}
}

//**************************************************************************
//													paint the game on screen
void paintGame(string msg, char garden[][SIZEY][SIZEX])
{ //display game title, messages, snail & other elements on screen



	void showTime(int, int);

	void paintGarden(const char[][SIZEY][SIZEX]);

	void showMessage(string, int, int);
	void showPelletCount(int, int, int);
	void showSnailhealth(float, int, int);

	// ************** code to be timed ***********************************************
	PaintTime.startTimer();



	showTime(LEFTM, 2);		// display system clock time

	paintGarden(garden);		// display garden contents

	showSnailhealth(lifeLeft, 6, 22);		// show snail's health
	showMessage(msg, 6, 23);	// display status message, if any

	PaintTime.stopTimer();
	// *************** end of timed section ******************************************

} //end of paintGame


  //**************************************************************************
  //													display garden on screen
void paintGarden(const char garden[][SIZEY][SIZEX])
{ //display garden content on screen
	string gardenOutput = "";

	SelectBackColour(clGreen);
	SelectTextColour(clDarkBlue);
	Gotoxy(0, 2);
	for (int y(0); y < (SIZEY); ++y)
	{	
		for (int x(0); x < (SIZEX); ++x)
		{
			gardenOutput.push_back(garden[0][y][x]);			// display current garden contents
		}
		gardenOutput.push_back('\n');
	}
	puts(gardenOutput.c_str());
} //end of paintGarden


  //**************************************************************************
  //															no slime yet!
void initialiseSlimeTrailAndFood(char garden[][SIZEY][SIZEX])
{ // set the whole array to 0

	for (int y = 1; y < SIZEY - 1; y++)			// can't slime the walls
		for (int x = 1; x < SIZEX - 1; x++) {
			garden[2][y][x] = 0;
			garden[1][y][x] = GRASS;
		}
}

//**************************************************************************
//												implement arrow key move
void analyseKey(string& msg, int key, int move[2])
{ //calculate snail movement required depending on the arrow key pressed
	if (key == LEFT){
		move[0] = 0; move[1] = -1;	// decrease the X coordinate
	}
	else if (key == RIGHT){
		move[0] = 0; move[1] = +1;	// increase the X coordinate
	}
	else if (key == UP) {
		move[0] = -1; move[1] = 0;	// decrease the Y coordinate
	}
	else if (key == DOWN){
		move[0] = +1; move[1] = 0;	// increase the Y coordinate
	}
	else {
		msg = "INVALID KEY";	// prepare error message
		move[0] = 0;			// move snail out of the garden
		move[1] = 0;
	}
}

//**************************************************************************
//			scatter some stuff around the garden (slug pellets, lettuces, and worms)

void scatterStuff(char garden[][SIZEY][SIZEX], int snail[])
{
	// ensure stuff doesn't land on the snail, or each other.
	// prime x,y coords with initial random numbers before checking

//=================SET PELLET POSITIONS==========================
	int x(Random(SIZEX - 2)), y(Random(SIZEY - 2));				// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET);	// avoid snail and other pellets

	garden[0][y][x] = PELLET;	// hide pellets around the garden #1

	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET);	// avoid snail and other pellets

	garden[0][y][x] = PELLET;	// hide pellets around the garden #2

	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET);	// avoid snail and other pellets

	garden[0][y][x] = PELLET;	// hide pellets around the garden #3

	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET);	// avoid snail and other pellets

	garden[0][y][x] = PELLET;	// hide pellets around the garden #4

	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET);	// avoid snail and other pellets

	garden[0][y][x] = PELLET;	// hide pellets around the garden #5

	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET);	// avoid snail and other pellets

	garden[0][y][x] = PELLET;	// hide pellets around the garden #6

	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET);	// avoid snail and other pellets

	garden[0][y][x] = PELLET;	// hide pellets around the garden #7

	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET);	// avoid snail and other pellets

	garden[0][y][x] = PELLET;	// hide pellets around the garden #8

	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET);	// avoid snail and other pellets

	garden[0][y][x] = PELLET;	// hide pellets around the garden #9

	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET);	// avoid snail and other pellets

	garden[0][y][x] = PELLET;	// hide pellets around the garden #10

	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET);	// avoid snail and other pellets

	garden[0][y][x] = PELLET;	// hide pellets around the garden #11

	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET);	// avoid snail and other pellets

	garden[0][y][x] = PELLET;	// hide pellets around the garden #12

	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET);	// avoid snail and other pellets

	garden[0][y][x] = PELLET;	// hide pellets around the garden #13

	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET);	// avoid snail and other pellets

	garden[0][y][x] = PELLET;	// hide pellets around the garden #14

	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET);	// avoid snail and other pellets

	garden[0][y][x] = PELLET;	// hide pellets around the garden #15

//===============================================================

//=================SET LETTUCE POSITIONS=========================

	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET || garden[1][y][x] == LETTUCE);	// avoid existing snail, pellets and other lettucii

	garden[1][y][x] = LETTUCE;								// plant a lettuce in the foodSources array #1

	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET || garden[1][y][x] == LETTUCE);	// avoid existing snail, pellets and other lettucii

	garden[1][y][x] = LETTUCE;								// plant a lettuce in the foodSources array #2

	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords				// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET || garden[1][y][x] == LETTUCE);	// avoid existing snail, pellets and other lettucii

	garden[1][y][x] = LETTUCE;								// plant a lettuce in the foodSources array #3

	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords			// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET || garden[1][y][x] == LETTUCE);	// avoid existing snail, pellets and other lettucii

	garden[1][y][x] = LETTUCE;								// plant a lettuce in the foodSources array #4

	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords			// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET || garden[1][y][x] == LETTUCE);	// avoid existing snail, pellets and other lettucii

	garden[1][y][x] = LETTUCE;								// plant a lettuce in the foodSources array #5

	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords			// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET || garden[1][y][x] == LETTUCE);	// avoid existing snail, pellets and other lettucii

	garden[1][y][x] = LETTUCE;								// plant a lettuce in the foodSources array #6

//==========================================================================

//==================PLACE WORMS=============================================
	
	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET || garden[1][y][x] == LETTUCE || garden[0][y][x] == WORM);	// avoid existing snail, pellets,lettuces and worms!

	garden[1][y][x] = WORM;									// place a worm in the foodSources array #1

	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET || garden[1][y][x] == LETTUCE || garden[0][y][x] == WORM);	// avoid existing snail, pellets,lettuces and worms!

	garden[1][y][x] = WORM;									// place a worm in the foodSources array #2

	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET || garden[1][y][x] == LETTUCE || garden[0][y][x] == WORM);	// avoid existing snail, pellets,lettuces and worms!

	garden[1][y][x] = WORM;									// place a worm in the foodSources array #3

	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET || garden[1][y][x] == LETTUCE || garden[0][y][x] == WORM);	// avoid existing snail, pellets,lettuces and worms!

	garden[1][y][x] = WORM;									// place a worm in the foodSources array #4

	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET || garden[1][y][x] == LETTUCE || garden[0][y][x] == WORM);	// avoid existing snail, pellets,lettuces and worms!

	garden[1][y][x] = WORM;									// place a worm in the foodSources array #5

	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET || garden[1][y][x] == LETTUCE || garden[0][y][x] == WORM);	// avoid existing snail, pellets,lettuces and worms!

	garden[1][y][x] = WORM;									// place a worm in the foodSources array #6

	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET || garden[1][y][x] == LETTUCE || garden[0][y][x] == WORM);	// avoid existing snail, pellets,lettuces and worms!

	garden[1][y][x] = WORM;									// place a worm in the foodSources array #7

	x = (Random(SIZEX - 2));
	y = (Random(SIZEY - 2));				// seed x and y with random coords
	while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == PELLET || garden[1][y][x] == LETTUCE || garden[0][y][x] == WORM);	// avoid existing snail, pellets,lettuces and worms!

	garden[1][y][x] = WORM;									// place a worm in the foodSources array #8

//==========================================================================
}


//**************************************************************************
//									some frogs have arrived looking for lunch

void scatterFrogs(char garden[][SIZEY][SIZEX], int snail[], int frogs[][2])
{
	// need to avoid the snail initially (seems a bit unfair otherwise!). Frogs aren't affected by
	// slug pellets, btw, and will absorb them, and they may land on lettuces or worms without damage.

		int x(Random(SIZEX - 2)), y(Random(SIZEY - 2));	// prime coords before checking
		while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == FROG);		// avoid snail and existing frogs

		frogs[0][0] = y;								// store initial positions of frog
		frogs[0][1] = x;
		garden[0][frogs[0][0]][frogs[0][1]] = FROG;		// put frogs on garden (this may overwrite a slug pellet) #1

		x = (Random(SIZEX - 2));
		y = (Random(SIZEY - 2));	// prime coords before checking
		while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[0][y][x] == FROG);		// avoid snail and existing frogs

		frogs[1][0] = y;								// store initial positions of frog
		frogs[1][1] = x;
		garden[0][frogs[1][0]][frogs[1][1]] = FROG;		// put frogs on garden (this may overwrite a slug pellet) #2

}


//**************************************************************************
//							move the Frogs toward the snail - watch for eagles!

void moveFrogs(int snail[], int frogs[][2], string& msg, char garden[][SIZEY][SIZEX])
{
	//	Frogs move toward the snail. They jump 'n' positions at a time in either or both x and y
	//	directions, but don't jump out of the garden. 
	//  If they land on the snail then it's dead meat. They might jump over it by accident.
	//	They can land on lettuces, worms and slug pellets - in the latter case the pellet is
	//  absorbed harmlessly by the frog (thus inadvertently helping the snail!).
	//	Frogs may also be randomly eaten by an eagle, with only the bones left behind.

	bool eatenByEagle(char[][SIZEY][SIZEX], int[]);

//======================FROG 1===========================

		if ((frogs[0][0] != DEAD_FROG_BONES) && snailStillAlive)		// if frog not been gotten by an eagle or GameOver
		{
			// jump off garden (taking any slug pellet with it)... check it wasn't on a lettuce or worm though...

			if (garden[1][frogs[0][0]][frogs[0][1]] == LETTUCE)
				garden[0][frogs[0][0]][frogs[0][1]] = LETTUCE;
			else  if (garden[1][frogs[0][0]][frogs[0][1]] == WORM)
				garden[0][frogs[0][0]][frogs[0][1]] = WORM;
			else garden[0][frogs[0][0]][frogs[0][1]] = GRASS;

			// Work out where to jump to depending on where the snail is...

			// see which way to jump in the Y direction (up and down)

			if (snail[0] - frogs[0][0] > 0)
			{
				frogs[0][0] += FROGLEAP;
				if (frogs[0][0] >= SIZEY - 1) frogs[0][0] = SIZEY - 2;  // don't go over the garden walls!

			}
			else if (snail[0] - frogs[0][0] < 0)
			{
				frogs[0][0] -= FROGLEAP;
				if (frogs[0][0] < 1) frogs[0][0] = 1;					// don't go over the garden walls!
			};

			// see which way to jump in the X direction (left and right)

			if (snail[1] - frogs[0][1] > 0)
			{
				frogs[0][1] += FROGLEAP;
				if (frogs[0][1] >= SIZEX - 1) frogs[0][1] = SIZEX - 2;
			}
			else if (snail[1] - frogs[0][1] < 0)
			{
				frogs[0][1] -= FROGLEAP;
				if (frogs[0][1] < 1)	frogs[0][1] = 1;
			};

			if (!eatenByEagle(garden, frogs[0]))						// not gotten by eagle?
			{
				if (frogs[0][0] == snail[0] && frogs[0][1] == snail[1])	// landed on snail? - grub up!
				{
					msg = "FROG GOT YOU! BB";
					snailStillAlive = false;							// snail is dead!
					gameEvent = DEADSNAIL;								//NEW record result
				}
				else garden[0][frogs[0][0]][frogs[0][1]] = FROG;			// display frog on garden (thus destroying any pellet that might be there).
			}
			else {
				msg = "EAGLE GOT A FROG B";
			}
		}
//====================================================

//====================FROG 2==========================
		if ((frogs[1][0] != DEAD_FROG_BONES) && snailStillAlive)		// if frog not been gotten by an eagle or GameOver
		{
			// jump off garden (taking any slug pellet with it)... check it wasn't on a lettuce or worm though...

			if (garden[1][frogs[1][0]][frogs[1][1]] == LETTUCE)
				garden[0][frogs[1][0]][frogs[1][1]] = LETTUCE;
			else  if (garden[1][frogs[1][0]][frogs[1][1]] == WORM)
				garden[0][frogs[1][0]][frogs[1][1]] = WORM;
			else garden[0][frogs[1][0]][frogs[1][1]] = GRASS;

			// Work out where to jump to depending on where the snail is...

			// see which way to jump in the Y direction (up and down)

			if (snail[0] - frogs[1][0] > 0)
			{
				frogs[1][0] += FROGLEAP;
				if (frogs[1][0] >= SIZEY - 1) frogs[1][0] = SIZEY - 2;  // don't go over the garden walls!

			}
			else if (snail[0] - frogs[1][0] < 0)
			{
				frogs[1][0] -= FROGLEAP;
				if (frogs[1][0] < 1) frogs[1][0] = 1;					// don't go over the garden walls!
			};

			// see which way to jump in the X direction (left and right)

			if (snail[1] - frogs[1][1] > 0)
			{
				frogs[1][1] += FROGLEAP;
				if (frogs[1][1] >= SIZEX - 1) frogs[1][1] = SIZEX - 2;
			}
			else if (snail[1] - frogs[1][1] < 0)
			{
				frogs[1][1] -= FROGLEAP;
				if (frogs[1][1] < 1)	frogs[1][1] = 1;
			};

			if (!eatenByEagle(garden, frogs[1]))						// not gotten by eagle?
			{
				if (frogs[1][0] == snail[0] && frogs[1][1] == snail[1])	// landed on snail? - grub up!
				{
					msg = "FROG GOT YOU! BB";
					snailStillAlive = false;							// snail is dead!
					gameEvent = DEADSNAIL;								//NEW record result
				}
				else garden[0][frogs[1][0]][frogs[1][1]] = FROG;			// display frog on garden (thus destroying any pellet that might be there).
			}
			else {
				msg = "EAGLE GOT A FROG B";
			}
		}
//=================================================
}

bool eatenByEagle(char garden[][SIZEY][SIZEX], int frog[])
{ //There's a 1 in 'EagleStrike' chance of being eaten

	if (!(Random(int(EagleStrike * 100.0)) == int(EagleStrike * 100.0)))
	{
		return false;
	}
	else {
		garden[0][frog[0]][frog[1]] = DEAD_FROG_BONES;				// show remnants of frog in garden
		frog[0] = DEAD_FROG_BONES;								// and mark frog as deceased
		gameEvent = DEAD_FROG_BONES;							//NEW record result
		return true;
	}
}

// end of moveFrogs


//**************************************************************************
//											implement player's move command

void moveSnail(int snail[], int keyMove[], string& msg, char garden[][SIZEY][SIZEX])
{
	// move snail on the garden when possible.
	// check intended new position & move if possible...
	// ...depending on what's on the intended next position in garden.

	lifeLeft -= ENERGY_USED;			// just moving costs energy, so deplete it. Assumes the move is made!
	if (lifeLeft > 0.0)					// check if snail has run out of energy
	{
		int targetY(snail[0] + keyMove[0]);
		int targetX(snail[1] + keyMove[1]);

		if ((garden[0][targetY][targetX] == GRASS) || (garden[0][targetY][targetX] == DEAD_FROG_BONES))
		{
			garden[0][snail[0]][snail[1]] = SLIME;				// lay a trail of slime
			garden[2][snail[0]][snail[1]] = SLIMELIFE;		// set slime life span
			snail[0] += keyMove[0];							// go in direction indicated by keyMove
			snail[1] += keyMove[1];
			moveResult = GRASS;								//NEW record result of move
		}
		else if (garden[0][targetY][targetX] == WALL)
		{
			msg += "B THAT'S A WALL!";
			lifeLeft += ENERGY_USED;	// didn't move, so return some health!
			moveResult = WALL;			//NEW record result of move
		}
		else if (garden[0][targetY][targetX] == LETTUCE)
		{
			garden[0][snail[0]][snail[1]] = SLIME;				//lay a trail of slime
			garden[2][snail[0]][snail[1]] = SLIMELIFE;		//set slime LIFE_SPAN
			snail[0] += keyMove[0];							//go in direction indicated by keyMove
			snail[1] += keyMove[1];
			garden[1][snail[0]][snail[1]] = GRASS;		// eat the lettuce, repace with grass
			lettucesEaten++;								// keep a count

			lifeLeft += LETTUCE_ENERGY;						// add energy to snail's LIFE_SPAN!
			if (lifeLeft > LIFE_SPAN) lifeLeft = LIFE_SPAN;	// can't acquire more than 100% energy

			fullOfLettuce = (lettucesEaten == LETTUCE_QUOTA); // if full, stop the game as snail wins!
			fullOfLettuce ? msg = "LAST LETTUCE EATEN BB" : msg = "LETTUCE EATEN B";
			// WIN! WIN! WIN!
			if (fullOfLettuce) gameEvent = WIN;				//NEW record result
			moveResult = LETTUCE;							//NEW record result of move
		}
		else if (garden[0][targetY][targetX] == PELLET)
		{
			garden[0][snail[0]][snail[1]] = SLIME;				// lay a trail of slime
			garden[2][snail[0]][snail[1]] = SLIMELIFE;		// set slime LIFE_SPAN
			snail[0] += keyMove[0];							// go in direction indicated by keyMove
			snail[1] += keyMove[1];
			msg = "PELLET ALERT! B";

			lifeLeft *= (LIFE_SPAN - PELLET_POISON);		// lose a bit of health
			moveResult = PELLET;							//NEW record result of move
		}
		else if (garden[0][targetY][targetX] == WORM)
		{
			garden[0][snail[0]][snail[1]] = SLIME;				// lay a trail of slime
			garden[2][snail[0]][snail[1]] = SLIMELIFE;		// set slime LIFE_SPAN
			snail[0] += keyMove[0];							// go in direction indicated by keyMove
			snail[1] += keyMove[1];
			garden[1][snail[0]][snail[1]] = GRASS;		// eat the worm, only grass left behind
			msg = "WORM EATEN B";

			if (lifeLeft > (LIFE_SPAN - WORM_ENERGY)) lifeLeft = LIFE_SPAN;	// can't have more than 100% life span!
			else lifeLeft += WORM_ENERGY;
			moveResult = WORM;								//NEW record result of move
		}
		else if (garden[0][targetY][targetX] == FROG)
		{
			garden[0][snail[0]][snail[1]] = SLIME;				// lay a final trail of slime
			snail[0] += keyMove[0];							// go in direction indicated by keyMove
			snail[1] += keyMove[1];
			msg = "OH NO! A FROG! BB";
			snailStillAlive = false;						// game over
			moveResult = FROG;								//NEW record result of move
			gameEvent = DEADSNAIL;							//NEW record result of move
		}
		else if (garden[0][targetY][targetX] == SLIME)
		{
			msg = "B THAT'S SLIME!";
			lifeLeft += ENERGY_USED; // didn't move, so return some health!
			moveResult = SLIME;								//NEW record result of move
		}
		else
		{
				msg = "NOT MOVED!";
				lifeLeft += ENERGY_USED; // didn't move, so return some health!
				moveResult = STUCK;								//NEW record result of move
		}
	}
	else
	{
		msg = "EXHAUSTED! TIME TO DIE... BB";
		snailStillAlive = false;		// if exhausted, game over 
		gameEvent = DEADSNAIL;			//NEW record result of move
		return;
	}


} //end of MoveSnail


  //// HELPER AND DISPLAY FUNCTIONS ////////////////////////////////////////////

  //**************************************************************************
  //											 get control key from player

int getKeyPress()				//NEW2 now altered to read from file
{ //get command from user

	int command;
	char dumpComma;				//NEW2

	ST_PlayList >> command;		//NEW2 Get command from prerecorded list (CSV file)
	ST_PlayList >> dumpComma;	//NEW2 remember to read and dump the comma separator

	//NEW2 The old method below now not used, all hands off now! You can still use version 1.0 
	//NEW2 to create different playlists for subsequent use in this version 2.0.
								// read in the selected option
	//command = _getch();  		// to read arrow keys
	//while (command == 224)	// to clear extra info from buffer
	//	command = _getch();

	//ST_Moves << command << ','; // NEW save commands as they're entered, as CSV file.

	if ((command == NULL) | (command > '~')) command = QUIT;	//NEW2 if playlist is empty or invalid 
																//NEW2 (no key should exceed the last keyboard ASCII char '~') 
																//NEW2 then supply QUIT characters to halt game
	return(command);

} //end of getKeyPress

  //**************************************************************************
  //											display info on screen
void clearMessage(string& msg)
{ //reset message
	msg = "";
} //end of clearMessage

  //**************************************************************************

void showTitle(int column, int row)
{ //display game title

	//Clrscr();
	SelectBackColour(clBlack);
	SelectTextColour(clYellow);
	Gotoxy(column, row);
	puts("...THE SNAIL TRAIL...");
	SelectBackColour(clWhite);
	SelectTextColour(clRed);

} //end of showTitle

void showDate(int column, int row)
{ //show current date and time
	string date = "DATE: " + GetDate();

	SelectBackColour(clWhite);
	SelectTextColour(clBlack);
	Gotoxy(column, row);
	puts(date.c_str());
} //end of showDateAndTime

void showTime(int column, int row)
{ //show current date and time
	string time = "TIME: " + GetTime();

	SelectBackColour(clWhite);
	SelectTextColour(clBlack);
	Gotoxy(column, row);
	puts(time.c_str());
} //end of showDateAndTime

void showOptions(int column, int row)
{ //show game options
	const int instructionsRows = 6;
	string instructions[instructionsRows];
	instructions[0] = "Instructions";
	instructions[2] = "* TO MOVE USE ARROW KEYS - EAT ALL " + to_string(LETTUCE_QUOTA) + " LETTUCES TO WIN.";
	instructions[3] = "* EAT WORMS (";
	instructions[3] += WORM;
	instructions[3] += ") AND LETTUCES (";
	instructions[3] += LETTUCE;
	instructions[3] += ") TO BOOST HEALTH.";
	instructions[4] = "* EACH MOVE AND INVISIBLE SLUG PELLETS DEPLETE HEALTH.";
	instructions[5] = "* TO QUIT ANY TIME USE 'Q'";
		
	SelectBackColour(clWhite);
	SelectTextColour(clBlack);
	Gotoxy(column, row);
	puts(instructions[0].c_str());
	row ++;

	SelectBackColour(clRed);
	SelectTextColour(clYellow);
	Gotoxy(column, row);
	puts(instructions[1].c_str());
	row++;

	Gotoxy(column, row);
	puts(instructions[2].c_str());
	row++;

	Gotoxy(column, row);
	puts(instructions[3].c_str());
	row++;

	Gotoxy(column, row);
	puts(instructions[4].c_str());
	row++;

	Gotoxy(column, row);
	puts(instructions[5].c_str());

} //end of showOptions

void showMessage(string msg, int column, int row)
{ //display auxiliary messages if any
	msg += "                                       ";
	SelectBackColour(clBlack);
	SelectTextColour(clYellow);
	Gotoxy(column, row);
	puts(msg.c_str());			//display current message
} //end of showMessage


void showSnailhealth(float health, int column, int row)
{ //display snail life left

	SelectBackColour(clBlack);
	SelectTextColour(clYellow);
	Gotoxy(column, row);

	if (health > (LIFE_SPAN / 4.0))   // if health drops below 25% highlight it in RED
		SelectTextColour(clYellow);	
	else 
		SelectTextColour(clRed);

	if (health > 0.0) {
		printf("Health: ");
		printf("%2.2f", health * 100.0);
		printf("%% ");
	}
	else { 
		printf("Health: none!");
	}
} //end of showMessage

void showTimingHeadings(int column, int row)
{
	SelectBackColour(clBlack);
	SelectTextColour(clYellow);
	Gotoxy(column, row);
	puts("Game Timings:");
} //end of showTimingHeadings

int anotherGo(int column, int row)
{ //show end message and hold output screen

	SelectBackColour(clRed);
	SelectTextColour(clYellow);
	Gotoxy(column, row);
	puts("PRESS 'Q' AGAIN TO QUIT, OR ANY KEY TO CONTINUE");
	SelectBackColour(clBlack);
	SelectTextColour(clWhite);

	return (getKeyPress());
} // end of anotherGo


void showTimes(float InitTimeSecs, float FrameTimeSecs, float PaintTimeSecs, int column, int row)
{ // show various times as a measure of performance

#define milli (1000.)
#define micro (1000000.)

	string timeInfo[4];

	SelectBackColour(clBlack);
	SelectTextColour(clWhite);

	timeInfo[0] = "Initialise game= " + to_string(InitTimeSecs * micro);
	timeInfo[0] += " us   ";

	timeInfo[1] = "Paint Game=      " + to_string(PaintTimeSecs * milli);
	timeInfo[1] += " ms  ";

	timeInfo[3] = "Frames/sec=      " + to_string((float) 1.0 / FrameTimeSecs);
	timeInfo[3] += " at " + to_string(FrameTimeSecs * milli);
	timeInfo[3] += " ms/frame   ";

	Gotoxy(column, row);
	puts(timeInfo[0].c_str());

	Gotoxy(column, row + 1);
	puts(timeInfo[1].c_str());

	Gotoxy(column, row + 3);
	puts(timeInfo[3].c_str());


} // end of showTimes


// NEW declarations ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void saveData (float InitTimeSecs, float FrameTimeSecs, float PaintTimeSecs, int key, string msg, float health)
{ //NEW save timing and other performance data to a file

#define milli (1000.)
#define micro (1000000.)
	ST_Times << fixed << setprecision(2);

	if (!InitTimesAlreadySaved) { // only do this once per game

		if (record_Every_Step) { // skip this if we're not saving each step's details
			ST_Times << "\n\nDATE:\t" << GetDate() << "\tTIME:\t" << GetTime()
				 << "\nInitialising game took " << InitTimeSecs * micro << " us";

			// Produce headings for rest of data
			ST_Times << "\n\nPaint Game (ms)\t\tMove made & Status\t\t\tHealth\t\tFrame rate/s\tTime/Frame (ms)\tScreen messages"
				      << "\n-----------------------------------------------------------------------------------------------------------";
		}
		InitTimeTotal += InitTimeSecs;
		GamesPlayed++;
	}

	if (record_Every_Step) { // skip this if we're not saving each step's details
		ST_Times << "\n\t" << PaintTimeSecs * milli
			<< "\t\t" << (key == LEFT ? "LEFT " : key == RIGHT ? "RIGHT" : key == UP ? "UP   " : key == DOWN ? "DOWN " : key == QUIT ? "QUIT " : "Wrong")
			<< " to " 
			<< (moveResult == GRASS  ? "GRASS  " : 
				moveResult == WALL   ? "WALL   " : 
				moveResult == PELLET ? "PELLET " : 
				moveResult == WORM   ? "WORM   " :
				moveResult == SLIME  ? "SLIME  " :
				moveResult == LETTUCE? "LETTUCE" :
				moveResult == STUCK  ? "STUCK  " : "DEATH  ")
			<< " and " 
			<< (gameEvent == FROG            ? "A FROG!   " : 
				gameEvent == DEAD_FROG_BONES ? "FROG GONE " : 
				gameEvent == WIN             ? "WON GAME  " : 
				gameEvent == DEADSNAIL       ? "LOST GAME " : "ALIVE     ")
			<< health * 100.0 << '%'
			<< "\t\t\t" << (float) 1.0 / FrameTimeSecs << "\t\t" << FrameTimeSecs * milli
		    << "\t\t" << msg;									// repeat messages seen on screen

		gameEvent = 0; // reset status
		moveResult = 0;

	}

	//NEW  Keep running totals
	PaintTimeTotal += PaintTimeSecs;
	FrameTimeTotal += FrameTimeSecs;
	TotalMovesMade++; // for the whole session, may be several games.

	if ((key | 0x20) == QUIT) {		// all games played, figure the averages and store

		ST_Times << "\n\nStudent: "<< studentName << "\tDATE: " << GetDate() << "\tTIME: " << GetTime()
			<< "\nSUMMARY for " << GamesPlayed << " games played, with " << TotalMovesMade << " moves entered."
		    << setprecision(3) 
			<< "\n\nAverage frames/sec=\t" << (float)TotalMovesMade / FrameTimeTotal
			<< "\nAverage Paint time=\t"   << (PaintTimeTotal * milli) / (float)TotalMovesMade << " ms"
			<< "\nAverage Init time=\t"    << (InitTimeTotal * micro) / (float)GamesPlayed << " us"

		    << "\n\n*************************************************************************************************************************";
	};

} //NEW end of save Times

void openFiles()
{ //NEW open the files that will store the times produced and the file with the moves recorded.
  //NEW NB. Files are closed automatically by system when program exits (not good but sufficient here)

	ST_Times.open("SnailTrailTimes.csv", ios::app);			// Open file for recording run times etc.
	//ST_Moves.open("SnailTrailMoves.csv", ios::app);		// Open file to record moves to.
	//ST_PlayList.open("SnailTrailMoves.csv", ios::in);		//NEW2 Open prerecorded file with game moves.
	
	
	//NEW2 NOTE: Your final submitted version should work with this supplied test file, below:
	ST_PlayList.open("SnailTrailMoves_FOR_DEMO_USE_999_SEED.csv", ios::in);

	//NEW2 This should produce a report in the output files of: "SUMMARY for 26 games played, with 940 moves entered."
	//NEW2 If yours doesn't it may mean your game play has been inadvertently altered from the original.



} //NEW end of openFiles


// End of the whole 'SNAIL TRAIL' listing //////////////////////////////////////////////////////////////////////////

