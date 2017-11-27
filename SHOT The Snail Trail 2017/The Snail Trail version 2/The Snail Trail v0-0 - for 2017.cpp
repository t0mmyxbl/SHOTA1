//THE SNAIL TRAIL GAME - version 0.0
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
#include <iostream >         //for output and input
#include <iomanip>           //for formatted output in 'cout'
#include <conio.h>           //for getch
#include <fstream>           //for files
#include <string>            //for string
#include "hr_time.h"         //for timers

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
const float ENERGY_USED(LIFE_SPAN/30.0);// each move uses this much energy (30 moves with no food and you're dead!)
const char DEADSNAIL('o');				// just the shell left...
const char GRASS(' ');					// open space
const char WALL('+');                   // garden wall

const char  WORM('~');					// worms are high energy food = 50% energy!
const float WORM_ENERGY(LIFE_SPAN/2.0);	// eat worms to keep living in the fast lane!
const int   WORM_COUNT(8);				// how many worms are available

const char SLIME('.');					// snail produce
const int  SLIMELIFE(20);				// how long slime lasts (in keypresses)

const char PELLET('-');					// should be invisible, but for testing using a visible character.
const int  NUM_PELLETS(15);				// number of slug pellets scattered about
const float PELLET_POISON(LIFE_SPAN / 10.0); // each pellet saps a bit of health too (10% of what remains)

const char LETTUCE('@');				// a lettuce
const int  LETTUCE_QUOTA(6);			// how many lettuces you need to eat before you win.
const float LETTUCE_ENERGY (LIFE_SPAN/LETTUCE_QUOTA);		// energy available from eating a lettuce (less than a worm!)

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
const char Bleep('\a');					// annoying Bleep
string Bleeeep("\a\a");					// very annoying Bleeps

const int LEFTM(SIZEX + 3);				//define left margin for messages (avoiding garden)

//define a few global control constants for our snail
int	  snailStillAlive(true);			// snail starts alive!
float lifeLeft(LIFE_SPAN);				// Life starts at 100%, need to eat lettuces and worms to live
int   lettucesEaten(0);					// win when this reaches LETTUCE_QUOTA
bool  fullOfLettuce(false);				// when full and alive snail has won!


CStopWatch	InitTime,
			FrameTime,
			PaintTime;					// create stopwatchs for timing



// Start of the 'SNAIL TRAIL' listing
//---------------------------------
int __cdecl main()
{
	//function prototypes

	void initialiseGame(int&, bool&, char[][SIZEX], char[][SIZEX], int[], int[][2], char[][SIZEX]);
	void paintGame( string message, char[][SIZEX]);
	void clearMessage(string& message);

	int getKeyPress();
	void analyseKey(string& message, int, int move[2]);
	void moveSnail(char[][SIZEX], int[], int[],  string&, char[][SIZEX], char[][SIZEX]);
	void moveFrogs(int[], int[][2], string&, char[][SIZEX], char[][SIZEX]);
	void placeSnail(char[][SIZEX], int[]);
	void dissolveSlime(char[][SIZEX], char[][SIZEX]);
	void showFood(char[][SIZEX], char[][SIZEX]);

	int anotherGo (int, int);

	// Timing info
	void showTimes (double, double, double, int, int);

	//local variables
	//arrays that store ...
	char garden      [SIZEY][SIZEX];		// the game 'world'
	char slimeTrail  [SIZEY][SIZEX];		// lifetime of slime counters overlay
	char foodSources [SIZEY][SIZEX];		// remember where the lettuces are planted and worms are

	string message;							// various messages are produced in game.
	int  snail[2];							// the snail's current position (x,y)
	int  frogs[NUM_FROGS][2];				// coordinates of the frog contingent n * (x,y)
	int  move[2];							// the requested move direction

	int  key, newGame(!QUIT);				// start new game by not quitting initially!

	// Now start the game...

	Seed();									//seed the random number generator
	
	// ******************************** Main Game Loop **************************************
	while ((newGame | 0x20) != QUIT)		// keep playing games
	{
		Clrscr();

		// ************** code to be timed ***********************************************
		InitTime.startTimer();

		//initialise garden (incl. walls, frogs, lettuces & snail)
		initialiseGame( lettucesEaten, fullOfLettuce, slimeTrail, foodSources, snail, frogs, garden);
		message = "READY TO SLITHER!? PRESS A KEY...";
		
		InitTime.stopTimer();
		// *************** end of timed section ******************************************


		paintGame( message, garden);			//display game info, garden & messages
		key = getKeyPress();					//get started or quit game


		// ******************************** Frame Loop **************************************
		while (((key | 0x20) != QUIT) && snailStillAlive && !fullOfLettuce)	//user not bored, and snail not dead or full
		{
			FrameTime.startTimer(); // not part of game

			// ************** code to be timed ***********************************************

			analyseKey    (message, key, move);			// get next move from keyboard
			moveSnail     (foodSources, snail, move, message, garden, slimeTrail);
			dissolveSlime (garden, slimeTrail);			// remove slime over time from garden
			showFood	  (garden, foodSources);		// show remaining lettuces and worms on ground
			placeSnail    (garden, snail);				// move snail in garden
			moveFrogs	  (snail, frogs, message, garden, foodSources);	// frogs attempt to home in on snail


			//FrameTime.stopTimer(); // you should eventually uncomment this and comment out the identical line 4 lines down

			paintGame( message, garden);				// display game info, garden & messages
			clearMessage(message);						// reset message array

			// *************** end of timed section ******************************************

			FrameTime.stopTimer(); // not part of game

			showTimes (InitTime.getElapsedTime(), FrameTime.getElapsedTime(), PaintTime.getElapsedTime(), LEFTM,6);

			key = getKeyPress();						// display menu & read in next option
		}

		// ******************************** End of Frame  Loop **************************************
		
		//							If alive...								If dead...
		(snailStillAlive) ? message = "WELL DONE, YOU WON!" : message = "REST IN PEAS.";
		
		if (!snailStillAlive) garden[snail[0]][snail[1]] = DEADSNAIL;
		paintGame ( message, garden);					// display final game info, garden & message

		newGame = anotherGo (LEFTM,20);					// Prompt to play again, or Quit game.

	} 	// ******************************** End of Main Game Loop **************************************

	return 0;
} //end main


// FUNCTION DEFINITIONS //////////////////////////////////////////////////////////////////////////////////

//**************************************************************************
//													set game configuration

void initialiseGame(int& Eaten, bool& fullUp, char slimeTrail[][SIZEX], char foodSources[][SIZEX],
	int snail[], int frogs[][2], char garden[][SIZEX])
{ //initialise garden & place snail somewhere

	void setGarden   (char[][SIZEX]);
	void setSnailInitialCoordinates(int[]);
	void placeSnail  (char[][SIZEX], int[]);
	void initialiseSlimeTrail  (char[][SIZEX]);
	void initialiseFoodSources(char[][SIZEX]);
	void showFood(char[][SIZEX], char[][SIZEX]);
	void scatterStuff(char[][SIZEX], char[][SIZEX], int[]);
	void scatterFrogs(char[][SIZEX], int[], int[][2]);

	snailStillAlive = true;					// bring snail to life!
	setSnailInitialCoordinates(snail);		// initialise snail position
	setGarden   (garden);					// reset the garden
	placeSnail  (garden, snail);			// place snail at a random position in garden
	initialiseSlimeTrail  (slimeTrail);		// no slime until snail moves
	initialiseFoodSources(foodSources);		// lettuces not been planted yet
	scatterStuff(garden, foodSources, snail);	// randomly scatter stuff about the garden (see function for details)
	showFood(garden, foodSources);			// show lettuces on ground
	scatterFrogs(garden, snail, frogs);		// randomly place a few frogs around
	
	lifeLeft = LIFE_SPAN;					// reset life span (health)
	Eaten = 0;								// reset number of lettuces eaten
	fullUp = false;							// snail is hungry again
}

//**************************************************************************
//												randomly drop snail in garden
void setSnailInitialCoordinates (int snail[])
{ //set snail's coordinates inside the garden at random at beginning of game

	snail[0] = Random(SIZEY - 2);		// vertical coordinate in range [1..(SIZEY - 2)]
	snail[1] = Random(SIZEX - 2);		// horizontal coordinate in range [1..(SIZEX - 2)]
}

//**************************************************************************
//						set up garden array to represent grass and walls

void setGarden(char garden[][SIZEX])
{ //reset to empty garden configuration

	for (int col(0); col < SIZEX; ++col)
	{
		for (int row(0); row < SIZEY; ++row)
		{
			garden[row][col] = GRASS;				// grow some 'grass'
			if ((row == 0) || (row == SIZEY - 1))	// insert top or bottom walls where needed
				garden[row][col] = WALL;
			if ((col == 0) || (col == SIZEX - 1))	// insert left & right walls where needed
				garden[row][col] = WALL;
		}
	}
} //end of setGarden

//**************************************************************************
//														place snail in garden
void placeSnail (char garden[][SIZEX], int snail[])
{ //place snail at its new position in garden

	garden [snail[0]][snail[1]] = SNAIL;
} //end of placeSnail

//**************************************************************************
//												slowly dissolve slime trail

void dissolveSlime (char garden[][SIZEX], char slimeTrail[][SIZEX])
{// go through entire slime trail and decrement each item of slime in order

	for (int x = 1; x < SIZEX - 1; x++)
	for (int y = 1; y < SIZEY - 1; y++)
	{
		if (slimeTrail[y][x] <= SLIMELIFE && slimeTrail[y][x] > 0)	// if this bit of slime exists
		{
			slimeTrail[y][x] --;									// then dissolve slime a little.
			if (slimeTrail[y][x] == 0)								// if totally dissolved then
				garden[y][x] = GRASS;								// then remove slime from garden
		}
	}
}

//**************************************************************************
//											show available food on the garden
void showFood (char garden[][SIZEX], char foodSources[][SIZEX])
{
	for (int x = 1; x < SIZEX - 1; x++)
		for (int y = 1; y < SIZEY - 1; y++)
		{
			if (foodSources[y][x] == WORM) garden[y][x] = WORM;
			if (foodSources[y][x] == LETTUCE) garden[y][x] = LETTUCE;
		}
}

//**************************************************************************
//													paint the game on screen
void paintGame( string msg, char garden[][SIZEX])
{ //display game title, messages, snail & other elements on screen

	void showTitle (int,int);
	void showDateAndTime (int, int);
	void showTimingHeadings(int, int);
	void paintGarden (const char[][SIZEX]);
	void showOptions (int, int);
	void showMessage (string, int, int);
	void showPelletCount (int, int, int);
	void showSnailhealth (float, int, int);

	// ************** code to be timed ***********************************************
	PaintTime.startTimer();

	showTitle (5,0);				// display game title
	showDateAndTime (LEFTM,1);		// display system clock
	showTimingHeadings(LEFTM,5);	// display Timings Heading
	paintGarden     (garden);		// display garden contents
	showOptions     (LEFTM,14);		// display menu options available
	showSnailhealth (lifeLeft, 6, 22);		// show snail's health
	showMessage     (msg, 6, 23);	// display status message, if any

	PaintTime.stopTimer(); 
	// *************** end of timed section ******************************************

} //end of paintGame


//**************************************************************************
//													display garden on screen
void paintGarden (const char garden[][SIZEX])
{ //display garden content on screen

	SelectBackColour(clGreen);
	SelectTextColour(clDarkBlue);
	Gotoxy(0, 2);
	for (int y(0); y < (SIZEY); ++y)
	{
		for (int x(0); x < (SIZEX); ++x)
		{
			cout << garden[y][x];			// display current garden contents
		}
		cout << endl;
	}
} //end of paintGarden


//**************************************************************************
//															no slime yet!
void initialiseSlimeTrail (char slimeTrail[][SIZEX])
{ // set the whole array to 0

	for (int x = 1; x < SIZEX - 1; x++)			// can't slime the walls
		for (int y = 1; y < SIZEY - 1; y++)
			slimeTrail[y][x] = 0;
}


//**************************************************************************
//													no lettuces or worms yet!
void initialiseFoodSources (char foodSources[][SIZEX])
{ // set the whole array to grass

	for (int x = 1; x < SIZEX - 1; x++)		// can't put stuff in the walls!
		for (int y = 1; y < SIZEY - 1; y++)
			foodSources[y][x] = GRASS;
}


//**************************************************************************
//												implement arrow key move
void analyseKey(string& msg, int key, int move[2])
{ //calculate snail movement required depending on the arrow key pressed

	switch (key)		//...depending on the selected key...
	{
	case LEFT:	//prepare to move left
		move[0] = 0; move[1] = -1;	// decrease the X coordinate
		break;
	case RIGHT: //prepare to move right
		move[0] = 0; move[1] = +1;	// increase the X coordinate
		break;
	case UP: //prepare to move up
		move[0] = -1; move[1] = 0;	// decrease the Y coordinate
		break;
	case DOWN: //prepare to move down
		move[0] = +1; move[1] = 0;	// increase the Y coordinate
		break;
	default:  					// this shouldn't happen
		msg = "INVALID KEY";	// prepare error message
		move[0] = 0;			// move snail out of the garden
		move[1] = 0;
	}
}


//**************************************************************************
//			scatter some stuff around the garden (slug pellets, lettuces, and worms)

void scatterStuff (char garden[][SIZEX], char foodSources[][SIZEX], int snail[])
{
	// ensure stuff doesn't land on the snail, or each other.
	// prime x,y coords with initial random numbers before checking

	for (int slugP = 0; slugP < NUM_PELLETS; slugP++)				// scatter some slug pellets...
	{
		int x(Random(SIZEX - 2)), y(Random(SIZEY - 2));				// seed x and y with random coords
		while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[y][x] == PELLET);	// avoid snail and other pellets

		garden[y][x] = PELLET;										// hide pellets around the garden
	}

	for (int food = 0; food < LETTUCE_QUOTA; food++)				// scatter lettuces for eating...
	{
		int x(Random(SIZEX - 2)), y(Random(SIZEY - 2));				// seed x and y with random coords
		while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[y][x] == PELLET || foodSources[y][x] == LETTUCE);	// avoid existing snail, pellets and other lettucii

		foodSources[y][x] = LETTUCE;								// plant a lettuce in the foodSources array
	}
	for (int food = 0; food < WORM_COUNT; food++)					// scatter worms...
	{
		int x(Random(SIZEX - 2)), y(Random(SIZEY - 2));				// seed x and y with random coords
		while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1]) || garden[y][x] == PELLET || foodSources[y][x] == LETTUCE || garden[y][x] == WORM);	// avoid existing snail, pellets,lettuces and worms!

		foodSources[y][x] = WORM;									// place a worm in the foodSources array
	}	
}


//**************************************************************************
//									some frogs have arrived looking for lunch

void scatterFrogs (char garden[][SIZEX], int snail[], int frogs[][2])
{
	// need to avoid the snail initially (seems a bit unfair otherwise!). Frogs aren't affected by
	// slug pellets, btw, and will absorb them, and they may land on lettuces or worms without damage.

	for (int f = 0; f < NUM_FROGS; f++)					// for each frog passing by ...
	{
		int x(Random(SIZEX - 2)), y(Random(SIZEY - 2));	// prime coords before checking
		while (((y = Random(SIZEY - 2)) == snail[0]) && ((x = Random(SIZEX - 2)) == snail[1])|| garden[y][x] == FROG);		// avoid snail and existing frogs

		frogs[f][0] = y;								// store initial positions of frog
		frogs[f][1] = x;
		garden[frogs[f][0]][frogs[f][1]] = FROG;		// put frogs on garden (this may overwrite a slug pellet)
	}
}


//**************************************************************************
//							move the Frogs toward the snail - watch for eagles!

void moveFrogs (int snail[], int frogs[][2], string& msg, char garden[][SIZEX], char lettuces[][SIZEX])
{
	//	Frogs move toward the snail. They jump 'n' positions at a time in either or both x and y
	//	directions, but don't jump out of the garden. 
	//  If they land on the snail then it's dead meat. They might jump over it by accident.
	//	They can land on lettuces, worms and slug pellets - in the latter case the pellet is
	//  absorbed harmlessly by the frog (thus inadvertently helping the snail!).
	//	Frogs may also be randomly eaten by an eagle, with only the bones left behind.

	bool eatenByEagle(char[][SIZEX], int[]);

	for (int f = 0; f<NUM_FROGS; f++)
	{
		if ((frogs[f][0] != DEAD_FROG_BONES) && snailStillAlive)		// if frog not been gotten by an eagle or GameOver
		{
			// jump off garden (taking any slug pellet with it)... check it wasn't on a lettuce or worm though...

			if (lettuces[frogs[f][0]][frogs[f][1]] == LETTUCE)
				garden[frogs[f][0]][frogs[f][1]] = LETTUCE;
			else  if (lettuces[frogs[f][0]][frogs[f][1]] == WORM)
				garden[frogs[f][0]][frogs[f][1]] = WORM; 
				else garden[frogs[f][0]][frogs[f][1]] = GRASS;

			// Work out where to jump to depending on where the snail is...
			
			// see which way to jump in the Y direction (up and down)

			if (snail[0] - frogs[f][0] > 0)
			{
				frogs[f][0] += FROGLEAP;  
				if (frogs[f][0] >= SIZEY - 1) frogs[f][0] = SIZEY - 2;  // don't go over the garden walls!

			} 
			else if (snail[0] - frogs[f][0] < 0)
			{
				frogs[f][0] -= FROGLEAP;  
				if (frogs[f][0] < 1) frogs[f][0] = 1;					// don't go over the garden walls!
			};

			// see which way to jump in the X direction (left and right)

			if (snail[1] - frogs[f][1] > 0)
			{
				frogs[f][1] += FROGLEAP;  
				if (frogs[f][1] >= SIZEX - 1) frogs[f][1] = SIZEX - 2;
			}
			else if (snail[1] - frogs[f][1] < 0)
			{
				frogs[f][1] -= FROGLEAP;  
				if (frogs[f][1] < 1)	frogs[f][1] = 1;
			};

			if (!eatenByEagle (garden, frogs[f]))						// not gotten by eagle?
			{
				if (frogs[f][0] == snail[0] && frogs[f][1] == snail[1])	// landed on snail? - grub up!
				{
					msg = "FROG GOT YOU!";
					cout << Bleeeep;									// produce a death knell
					snailStillAlive = false;							// snail is dead!
				}
				else garden[frogs[f][0]][frogs[f][1]] = FROG;			// display frog on garden (thus destroying any pellet that might be there).
			}
			else {
				msg = "EAGLE GOT A FROG";
				cout << Bleep;											//produce a warning sound
			}
		}
	}// end of FOR loop
}

bool eatenByEagle(char garden[][SIZEX], int frog[])
{ //There's a 1 in 'EagleStrike' chance of being eaten

	if (Random(int(EagleStrike * 100.0)) == int(EagleStrike * 100.0))
	{
		garden[frog[0]][frog[1]] = DEAD_FROG_BONES;				// show remnants of frog in garden
		frog[0] = DEAD_FROG_BONES;								// and mark frog as deceased
		return true;
	}
	else return false;
}

// end of moveFrogs


//**************************************************************************
//											implement player's move command

void moveSnail(char foodSources[][SIZEX], int snail[], int keyMove[], string& msg, char garden[][SIZEX], char slimeTrail[][SIZEX])
{
	// move snail on the garden when possible.
	// check intended new position & move if possible...
	// ...depending on what's on the intended next position in garden.

	lifeLeft -= ENERGY_USED;			// just moving costs energy, so deplete it
	if (lifeLeft < 0.0)					// check if snail has run out of energy
	{
		msg = "EXHAUSTED! TIME TO DIE...";
		cout << Bleeeep;
		snailStillAlive = false;		// if exhausted, game over 
		return;
	}

	int targetY(snail[0] + keyMove[0]);
	int targetX(snail[1] + keyMove[1]);
	switch (garden[targetY][targetX]) //depending on what is at target position
	{

	case LETTUCE:		// increment lettuce count and win if snail is full
		garden[snail[0]][snail[1]] = SLIME;				//lay a trail of slime
		slimeTrail[snail[0]][snail[1]] = SLIMELIFE;		//set slime LIFE_SPAN
		snail[0] += keyMove[0];							//go in direction indicated by keyMove
		snail[1] += keyMove[1];
		foodSources[snail[0]][snail[1]] = GRASS;		// eat the lettuce, repace with grass
		lettucesEaten++;								// keep a count

		lifeLeft += LETTUCE_ENERGY;						// add energy to snail's LIFE_SPAN!
		if (lifeLeft > LIFE_SPAN) lifeLeft = LIFE_SPAN;	// can't acquire more than 100% energy

		fullOfLettuce = (lettucesEaten == LETTUCE_QUOTA); // if full, stop the game as snail wins!
		fullOfLettuce ? msg = "LAST LETTUCE EATEN" : msg = "LETTUCE EATEN";
		fullOfLettuce ? cout << Bleeeep		       : cout << Bleep;
		// WIN! WIN! WIN!
		break;

	case WORM:			// if snail eats a worm, life extends... 
		garden[snail[0]][snail[1]] = SLIME;				// lay a trail of slime
		slimeTrail[snail[0]][snail[1]] = SLIMELIFE;		// set slime LIFE_SPAN
		snail[0] += keyMove[0];							// go in direction indicated by keyMove
		snail[1] += keyMove[1];
		foodSources[snail[0]][snail[1]] = GRASS;		// eat the worm, only grass left behind
		msg = "WORM EATEN";
		cout << Bleep;

		if (lifeLeft > (LIFE_SPAN - WORM_ENERGY)) lifeLeft = LIFE_SPAN;	// can't have more than 100% life span!
		else lifeLeft += WORM_ENERGY;
		break;

	case PELLET:		// warn that a pellet has been slithered over and poisoned the snail a bit
		garden[snail[0]][snail[1]] = SLIME;				// lay a trail of slime
		slimeTrail[snail[0]][snail[1]] = SLIMELIFE;		// set slime LIFE_SPAN
		snail[0] += keyMove[0];							// go in direction indicated by keyMove
		snail[1] += keyMove[1];
		msg = "PELLET ALERT!";
		cout << Bleep;									// produce a warning sound

		lifeLeft *= (LIFE_SPAN - PELLET_POISON);		// lose a bit of health
		break;

	case FROG:			//	kill snail if it throws itself at a frog!
		garden[snail[0]][snail[1]] = SLIME;				// lay a final trail of slime
		snail[0] += keyMove[0];							// go in direction indicated by keyMove
		snail[1] += keyMove[1];
		msg = "OH NO! A FROG!";
		cout << Bleeeep;								// produce a death knell
		snailStillAlive = false;						// game over
		break;

	case WALL:					// Oops, bumped into garden wall
		cout << Bleep;			// produce a warning sound
		msg = "THAT'S A WALL!";
		lifeLeft += ENERGY_USED; // didn't move, so return some health!
		break;

	case GRASS:
	case DEAD_FROG_BONES:	// it's safe to move over dead/missing frogs too
		garden[snail[0]][snail[1]] = SLIME;				// lay a trail of slime
		slimeTrail[snail[0]][snail[1]] = SLIMELIFE;		// set slime life span
		snail[0] += keyMove[0];							// go in direction indicated by keyMove
		snail[1] += keyMove[1];
		break;

	case SLIME:				// Been here before, snail doesn't cross his own slime!
		cout << Bleep;		// produce a warning sound
		msg = "THAT'S SLIME!";
		lifeLeft += ENERGY_USED; // didn't move, so return some health!
		break;

	default: 
		msg = "NOT MOVED!";
		lifeLeft += ENERGY_USED; // didn't move, so return some health!
		
	}
} //end of MoveSnail


//// HELPER AND DISPLAY FUNCTIONS ////////////////////////////////////////////

//**************************************************************************
//											 get control key from player
int getKeyPress()
{ //get command from user

	int command;
							// read in the selected option
	command = _getch();  	// to read arrow keys
	while (command == 224)	// to clear extra info from buffer
		command = _getch();
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

	Clrscr();
	SelectBackColour(clBlack);
	SelectTextColour(clYellow);
	Gotoxy(column, row);
	cout << "...THE SNAIL TRAIL..." << endl;
	SelectBackColour(clWhite);
	SelectTextColour(clRed);

} //end of showTitle

void showDateAndTime(int column, int row)
{ //show current date and time

	SelectBackColour(clWhite);
	SelectTextColour(clBlack);
	Gotoxy(column, row);
	cout << "DATE: " << GetDate();
	Gotoxy(column, row+1);
	cout << "TIME: " << GetTime();
} //end of showDateAndTime

void showOptions(int column, int row)
{ //show game options
	Gotoxy(column, row);
	SelectBackColour(clWhite);
	SelectTextColour(clBlack);
	cout << "Instructions";
	SelectBackColour(clRed);
	SelectTextColour(clYellow);
	Gotoxy(column, row += 1);
	cout << "* TO MOVE USE ARROW KEYS - EAT ALL " << LETTUCE_QUOTA << " LETTUCES TO WIN.";
	Gotoxy(column, row += 1);
	cout << "* EAT WORMS (" << WORM << ") AND LETTUCES (" << LETTUCE << ") TO BOOST HEALTH.";
	Gotoxy(column, row += 1);
	cout << "* EACH MOVE AND INVISIBLE SLUG PELLETS DEPLETE HEALTH.";
	Gotoxy(column, row += 1);
	cout << "* TO QUIT ANY TIME USE 'Q'";
} //end of showOptions

void showMessage(string msg, int column, int row)
{ //display auxiliary messages if any

	SelectBackColour(clBlack);
	SelectTextColour(clYellow);
	Gotoxy(column, row);
	cout << msg;			//display current message
} //end of showMessage


void showSnailhealth (float health, int column, int row)
{ //display snail life left

	SelectBackColour(clBlack);
	SelectTextColour(clYellow);
	Gotoxy(column, row);
	cout << fixed << setprecision(2);

	if (health < (LIFE_SPAN / 4.0))   // if health drops below 25% highlight it in RED
			SelectTextColour(clRed);
	else SelectTextColour(clYellow);

	if (health < 0.0)  cout << "Health: none!";
	else cout<< "Health: " << health * 100.0 << "%";

} //end of showMessage


void showTimingHeadings(int column, int row)
{ 
	SelectBackColour(clBlack);
	SelectTextColour(clYellow);
	Gotoxy(column, row);
	cout << "Game Timings:";
} //end of showTimingHeadings

int anotherGo(int column, int row)
{ //show end message and hold output screen

	SelectBackColour(clRed);
	SelectTextColour(clYellow);
	Gotoxy(column, row);
	cout << "PRESS 'Q' AGAIN TO QUIT, OR ANY KEY TO CONTINUE";
	SelectBackColour(clBlack);
	SelectTextColour(clWhite);

	return (getKeyPress());
} // end of anotherGo


void showTimes(double InitTimeSecs, double FrameTimeSecs, double PaintTimeSecs, int column, int row)
{ // show various times as a measure of performance

#define milli (1000.)
#define micro (1000000.)


	SelectBackColour(clBlack);
	SelectTextColour(clWhite);
	Gotoxy(column, row);
	cout << setprecision(5) << "Initialise game= " << InitTimeSecs * micro << " us" ;
	Gotoxy(column, row+1);
	cout << setprecision(5) << "Paint Game=      " << PaintTimeSecs * milli << " ms";
	Gotoxy(column, row+3);
	cout << setprecision(3) << "Frames/sec=      " << (double) 1.0 / FrameTimeSecs << " at "<<setprecision(5) << FrameTimeSecs * milli << " ms/frame";

} // end of showTimes

// End of the whole 'SNAIL TRAIL' listing //////////////////////////////////////////////////////////////////////////

