// $Id: mtilechess.c,v 1.8 2006-02-12 02:30:57 kmowery Exp $

/* 
 * The above lines will include the name and log of the last person 
 * to commit this file to CVS 
 */ 
 
/************************************************************************ 
** 
** NAME:        mtilechess.c 
** 
** DESCRIPTION: Tile Chess 
** 
** AUTHOR:      Brian Zimmer; Alan Roytman 
** 
** DATE:        2005-10-03 / 2005-12-15
** 
** UPDATE HIST: Version 1.6 - Final verson for Fall 2005
** 
**************************************************************************/ 
 
/************************************************************************* 
** 
** Everything below here must be in every game file 
** 
**************************************************************************/ 
 
#include <stdio.h> 
#include "gamesman.h" 
#include <stdlib.h> 
#include <unistd.h> 
#include <limits.h> 
 
/************************************************************************* 
** 
** Game-specific constants 
** 
**************************************************************************/ 
 
STRING   kGameName            = "Tile Chess"; /* The name of your game */ 
STRING   kAuthorName          = "Alan Roytman, Brian Zimmer"; /* Your name(s) */ 
STRING   kDBName              = "tilechess"; /* The name to store the database under */ 
 
BOOLEAN  kPartizan            = TRUE ; /* A partizan game is a game where each player has different moves from the same board (chess - different pieces) */ 
BOOLEAN  kGameSpecificMenu    = TRUE ; /* TRUE if there is a game specific menu. FALSE if there is not one. */ 
BOOLEAN  kTieIsPossible       = TRUE ; /* TRUE if a tie is possible. FALSE if it is impossible.*/ 
BOOLEAN  kLoopy               = TRUE ; /* TRUE if the game tree will have cycles (a rearranger style game). FALSE if it does not.*/ 
 
BOOLEAN  kDebugMenu           = FALSE ; /* TRUE only when debugging. FALSE when on release. */ 
BOOLEAN  kDebugDetermineValue = FALSE ; /* TRUE only when debugging. FALSE when on release. */ 
 
POSITION gNumberOfPositions   =  0; /* The number of total possible positions | If you are using our hash, this is given by the hash_init() function*/ 
POSITION gInitialPosition     =  0; /* The initial hashed position for your starting board */ 
POSITION kBadPosition         = -1; /* A position that will never be used */ 
 
void*	 gGameSpecificTclInit = NULL; 
 
/*  
 * Help strings that are pretty self-explanatory 
 * Strings than span more than one line should have backslashes (\) at the end of the line. 
 */ 
 
STRING kHelpGraphicInterface = 
"Not written yet"; 
 
STRING   kHelpTextInterface    = 
"The board is arranged like a standard chess board, with the exception that it can change size. The rows are specified by numeric values, while the rows are specified by letters. A square is referenced by the column and then the row, i.e. a1.";  
 
STRING   kHelpOnYourTurn = 
"Enter your move in a style similar to algebraic notation for chess. Specify the beginning square with the rank and file (a letter then a number), and the destination square. All of the pieces move in the same way as regular chess, with a few exceptions. You can jump over your own pieces with pieces other than the knight, and pawns can move, as well as capture, forwards and backwards.\nEx: If you want to move a king from square b2 to b3, type b2b3."; 
 
STRING   kHelpStandardObjective = 
"Try to checkmate your opponent's king."; 
 
STRING   kHelpReverseObjective = 
"Try to get your king checkmated."; 
 
STRING   kHelpTieOccursWhen = 
"the current player is not in check and cannot move his/her king."; 
 
STRING   kHelpExample = 
"    +---+---+---+---+---+\n\
  3 |   |   |   |   |   |\n\
    +---+---+---+---+---+\n\
  2 |   | K | B | k |   |\n\
    +---+---+---+---+---+\n\
  1 |   |   |   |   |   |\n\
    +---+---+---+---+---+\n\
      a   b   c   d   e\n\
It is Player's turn (white/uppercase).\n\
  Player's move [(undo)/<ranki><filei><rankf><filef>] : b2b3\n\
\n\
    +---+---+---+---+---+\n\
  4 |   |   |   |   |   |\n\
    +---+---+---+---+---+\n\
  3 |   | K |   |   |   |\n\
    +---+---+---+---+---+\n\
  2 |   |   | B | k |   |\n\
    +---+---+---+---+---+\n\
  1 |   |   |   |   |   |\n\
    +---+---+---+---+---+\n\
      a   b   c   d   e\n\
It is Computer's turn (black/lowercase).\n\
\n\
    +---+---+---+---+\n\
  5 |   |   |   |   |\n\
    +---+---+---+---+\n\
  4 |   | K |   |   |\n\
    +---+---+---+---+\n\
  3 |   |   | B |   |\n\
    +---+---+---+---+\n\
  2 |   |   | k |   |\n\
    +---+---+---+---+\n\
  1 |   |   |   |   |\n\
    +---+---+---+---+\n\
      a   b   c   d\n\
It is Player's turn (white/uppercase).\n\
  Player's move [(undo)/<ranki><filei><rankf><filef>] : b4c4\n\
\n\
    +---+---+---+\n\
  5 |   |   |   |\n\
    +---+---+---+\n\
  4 |   | K |   |\n\
    +---+---+---+\n\
  3 |   | B |   |\n\
    +---+---+---+\n\
  2 |   | k |   |\n\
    +---+---+---+\n\
  1 |   |   |   |\n\
    +---+---+---+\n\
      a   b   c\n\
It is Computer's turn (black/lowercase).\n\
The match ends in a draw. Excellent strategies, Player and Computer\n"; 
 
 
/************************************************************************* 
** 
** #defines and structs 
** 
**************************************************************************/ 
#define BMAX 6 
#define PLAYER1_TURN 1 
#define PLAYER2_TURN 0 
#define TOTAL_PIECES 3 
#define BOARD_SIZE 9 
 
// Constants specifying directions to "look" on the board 
#define UP 0 
#define DOWN 1 
#define LEFT 2 
#define RIGHT 3 
#define UL 4 
#define UR 5 
#define DL 6 
#define DR 7 
 
/************************************************************************* 
** 
** Global Variables 
** 
*************************************************************************/ 
int indexToBoard[48]; // PreProcessed array of legal placements 
int boardToIndex[512]; // The reverse of the previous array (placement to an index) 
char *piecesunhashed[6]; // Permutations of piece characters 
 
/*VARIANTS*/ 
BOOLEAN bishopVariant = TRUE; // Default is the Bishop Variant 
BOOLEAN rookVariant = FALSE; 
 
/************************************************************************* 
** 
** Function Prototypes 
** 
*************************************************************************/ 
STRING MToS(MOVE theMove); 
void reverse(char s[]); 
void itoa(int n, char s[], int base); 
int atobi(char s[], int base); 
BOOLEAN isEqualString(char s[], char t[]); 
int hashBishopPieces(char pieceArray[]); 
int hashRookPieces(char pieceArray[]); 
int isLegalPlacement(int place, int sideLength); 
void PreProcess(); 
POSITION hashBoard(char bA[], int currentPlayer); 
int getCurrTurn(POSITION position); 
char *unhashBoard(POSITION position); 
char *FillBoardArray(char *pieceArray, char *placeArray); 
void generateKingMoves(POSITION N, MOVELIST **moves, int place); 
BOOLEAN isSameTeam(char piece, int currentPlayer); 
MOVE createMove(int init, int final, int sidelength); 
BOOLEAN testMove(int newspot, int origspot, POSITION N); 
BOOLEAN inCheck(POSITION N); 
BOOLEAN kingCheck(char *bA, int place, int currentPlayer); 
BOOLEAN isLegalBoard(char *bA); 
void generateBishopMoves(POSITION N, MOVELIST **moves, int place); 
void generateRookMoves(POSITION N, MOVELIST **moves, int place); 
BOOLEAN rookCheck(char *bA, int place, int currentPlayer); 
BOOLEAN bishopCheck(char *bA, int place, int currentPlayer); 
BOOLEAN canMove(POSITION position); 
void generateMovesDirection(POSITION N, MOVELIST **moves, int place, int direction); 
 
/* External */ 
extern GENERIC_PTR	SafeMalloc (); 
extern void		SafeFree (); 
 
/************************************************************************ 
** 
** NAME:        InitializeGame 
** 
** DESCRIPTION: Prepares the game for execution. 
**              Initializes required variables. 
**              Sets up gDatabase (if necessary). 
**  
************************************************************************/ 
 
void InitializeGame () 
{ 
  gMoveToStringFunPtr = &MToS; 
  PreProcess(); // Generate the legal placements for pieces 
  /*gNumberOfPositions = (((MaxPlacementIndex*MaxPiecePermutationValue) +  
    MaxPiecePermutationValue-1) << 1) + MaxPlayerTurnValue; */ 
  gNumberOfPositions = (((boardToIndex[448]*BMAX) + 5) << 1) + PLAYER1_TURN; 
  if (rookVariant) { 
    gInitialPosition = (((boardToIndex[273]*BMAX) + hashRookPieces("KRk")) << 1) + PLAYER1_TURN; 
  } else if (bishopVariant) { 
    gInitialPosition = (((boardToIndex[7]*BMAX) + hashBishopPieces("KBk")) << 1) + PLAYER1_TURN; 
  } 
} 
 
/************************************************************************ 
** 
** NAME:        GenerateMoves 
** 
** DESCRIPTION: Creates a linked list of every move that can be reached 
**              from this position. Returns a pointer to the head of the 
**              linked list. 
**  
** INPUTS:      POSITION position : Current position for move 
**                                  generation. 
** 
** OUTPUTS:     (MOVELIST *)      : A pointer to the first item of 
**                                  the linked list of generated moves 
** 
** CALLS:       MOVELIST *CreateMovelistNode(); 
** 
************************************************************************/ 
 
MOVELIST *GenerateMoves (POSITION position) 
{ 
  MOVELIST *moves = NULL; 
  int currentPlayer, i; 
  char piece; 
  char *boardArray; 
  boardArray = unhashBoard(position); 
  currentPlayer = getCurrTurn(position); 
  for (i = 0; i < strlen(boardArray); i++) { 
    piece = boardArray[i]; 
    if (isSameTeam(piece,currentPlayer)){  
      // The piece we're look at is the current player's piece 
      if (piece >= 'a' && piece <= 'z')  
	piece = piece - 'a' + 'A'; // Capitalize piece for switch statement 
      switch (piece) { 
      case 'K':  
	generateKingMoves(position,&moves,i); 
	break;    				 
      case 'B': 
	generateBishopMoves(position,&moves,i); 
	break; 
      case 'R': 
	generateRookMoves(position,&moves,i); 
      default:  
	break; 
      } 
    } 
  } 
  return moves; 
} 
 
 
/************************************************************************ 
** 
** NAME:        DoMove 
** 
** DESCRIPTION: Applies the move to the position. 
**  
** INPUTS:      POSITION position : The old position 
**              MOVE     move     : The move to apply to the position 
** 
** OUTPUTS:     (POSITION)        : The position that results from move 
** 
** CALLS:       Some Board Hash Function 
**              Some Board Unhash Function 
** 
*************************************************************************/ 
 
POSITION DoMove (POSITION position, MOVE move) 
{ 
  char *boardArray; 
  int filef; 
  int rankf; 
  int filei; 
  int ranki; 
  char pieceMoved; 
  int sideLength = (int)pow(BOARD_SIZE,0.5)+2; 
  boardArray = unhashBoard(position); 
  //ANDing by 2^(4)-1 will give the last 4 bits 
  filef = move & 15; 
  rankf = (move >> 4) & 15; 
  filei = (move >> 8) & 15; 
  ranki = (move >> 12) & 15; 
  ranki -= 10; 
  rankf -= 10; 
  pieceMoved = boardArray[(sideLength-filei)*sideLength+ranki]; 
  boardArray[(sideLength-filei)*sideLength+ranki] = ' '; 
  boardArray[(sideLength-filef)*sideLength+rankf] = pieceMoved; 
  return hashBoard(boardArray,!getCurrTurn(position)); 
} 
 
 
/************************************************************************ 
** 
** NAME:        Primitive 
** 
** DESCRIPTION: Returns the value of a position if it fulfills certain 
**              'primitive' constraints. 
** 
**              Example: Tic-tac-toe - Last piece already placed 
** 
**              Case                                  Return Value 
**              ********************************************************* 
**              Current player sees three in a row    lose 
**              Entire board filled                   tie 
**              All other cases                       undecided 
**  
** INPUTS:      POSITION position : The position to inspect. 
** 
** OUTPUTS:     (VALUE)           : one of 
**                                  (win, lose, tie, undecided) 
** 
** CALLS:       None               
** 
************************************************************************/ 
 
VALUE Primitive (POSITION position) 
{ 
  if (inCheck(position) && !canMove(position)) { 
    // The king is checked and can't move 
    return (gStandardGame) ? lose : win; 
  } 
  else if (!inCheck(position) && !canMove(position)) { 
    // King is not in check and can't move - Stalemate 
    return tie; 
  } else { 
    return undecided; 
  } 
} 
 
 
/************************************************************************ 
** 
** NAME:   maxInt     PrintPosition 
** 
** DESCRIPTION: Prints the position in a pretty format, including the 
**              prediction of the game's outcome. 
**  
** INPUTS:      POSITION position    : The position to pretty print. 
**              STRING   playersName : The name of the player. 
**              BOOLEAN  usersTurn   : TRUE <==> it's a user's turn. 
** 
** CALLS:       Unhash() 
**              GetPrediction()      : Returns the prediction of the game 
** 
************************************************************************/ 
 
void PrintPosition (POSITION position, STRING playersName, BOOLEAN usersTurn) 
{ 
  char *boardArray; 
  int r, c, sideLength, x, y; 
  boardArray = unhashBoard(position); 
  sideLength = pow(strlen(boardArray),0.5); 
  int bi=0; 
  int minx=sideLength, maxx=0, miny=sideLength, maxy=0; 
   
  /* Determine the rectangle the pieces reside in.  
     This is done so as not to print unnecessary empty 
     rows and columns. */ 
  for(bi=0;bi<strlen(boardArray);bi++){ 
    if(boardArray[bi]!=' '){ 
      x = bi%sideLength; 
      y = bi/sideLength; 
      if(x<minx) minx=x; 
      if(x>maxx) maxx=x; 
      if(y<miny) miny=y; 
      if(y>maxy) maxy=y; 
    } 
  } 
  printf("\n"); 
  for(r = miny-1; r <= maxy+1; r++){ 
    printf("    +"); 
    for(c = minx-1; c <= maxx+1; c++){ 
      printf("---+"); 
    } 
    printf("\n"); 
    printf("  %d |", sideLength - r); 
    for(c = minx-1; c <= maxx+1; c++){ 
      printf(" %c |", boardArray[r*sideLength+c]); 
    } 
    printf("\n"); 
  } 
  printf("    +"); 
  for(c = minx-1; c <= maxx+1; c++){ 
    printf("---+"); 
  } 
  printf("\n"); 
  printf("     "); 
  for(c = 0; c <= maxx+1; c++){ 
    printf(" %c  ", 97+c); 
  } 
  printf("\n"); 
  printf("%s\n",GetPrediction(position,playersName,usersTurn)); 
  printf("It is %s's turn (%s).\n",playersName,(usersTurn) ? "white/uppercase":"black/lowercase"); 
} 
 
 
/************************************************************************ 
** 
** NAME:        PrintComputersMove 
** 
** DESCRIPTION: Nicely formats the computers move. 
**  
** INPUTS:      MOVE    computersMove : The computer's move.  
**              STRING  computersName : The computer's name.  
** 
************************************************************************/ 
 
void PrintComputersMove (MOVE computersMove, STRING computersName) 
{ 
     
} 
 
 
/************************************************************************ 
** 
** NAME:        PrintMove 
** 
** DESCRIPTION: Prints the move in a nice format. 
**  
** INPUTS:      MOVE move         : The move to print.  
** 
************************************************************************/ 
 
void PrintMove (MOVE move) 
{ 
  printf("%x",move); // For now, move is a hex number 
} 
 
 
/************************************************************************ 
** 
** NAME:        GetAndPrintPlayersMove 
** 
** DESCRIPTION: Finds out if the player wishes to undo, abort, or use 
**              some other gamesman option. The gamesman core does 
**              most of the work here.  
** 
** INPUTS:      POSITION position    : Current position 
**              MOVE     *move       : The move to fill with user's move.  
**              STRING   playersName : Current Player's Name 
** 
** OUTPUTS:     USERINPUT          : One of 
**                                   (Undo, Abort, Continue) 
** 
** CALLS:       USERINPUT HandleDefaultTextInput(POSITION, MOVE*, STRING) 
**                                 : Gamesman Core Input Handling 
** 
************************************************************************/ 
 
USERINPUT GetAndPrintPlayersMove (POSITION position, MOVE *move, STRING playersName) 
{ 
  USERINPUT input; 
  USERINPUT HandleDefaultTextInput(); 
   
  for (;;) { 
    /*********************************************************** 
     * CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT 
     ***********************************************************/ 
    printf("%8s's move [(undo)/<ranki><filei><rankf><filef>] : ", playersName); 
     
    input = HandleDefaultTextInput(position, move, playersName); 
     
    if (input != Continue) 
      return input; 
  } 
   
  /* NOTREACHED */ 
  return Continue; 
} 
 
 
/************************************************************************ 
** 
** NAME:        ValidTextInput 
** 
** DESCRIPTION: Rudimentary check to check if input is in the move form 
**              you are expecting. Does not check if it is a valid move. 
**              Only checks if it fits the move form. 
** 
**              Reserved Input Characters - DO NOT USE THESE ONE CHARACTER 
**                                          COMMANDS IN YOUR GAME 
**              ?, s, u, r, h, a, c, q 
**                                          However, something like a3 
**                                          is okay. 
**  
**              Example: Tic-tac-toe Move Format : Integer from 1 to 9 
**                       Only integers between 1 to 9 are accepted 
**                       regardless of board position. 
**                       Moves will be checked by the core. 
** 
** INPUTS:      STRING input : The string input the user typed. 
** 
** OUTPUTS:     BOOLEAN      : TRUE if the input is a valid text input. 
** 
************************************************************************/ 
 
BOOLEAN ValidTextInput (STRING input) 
{ 
  if(strlen(input)!=4) 
    return FALSE; 
  else{ 
    int sidelength = pow(BOARD_SIZE, 0.5)+2; 
    char maxChar = 'A' + sidelength-1; 
    char maxInt = '1' + sidelength-1; 
    char c = input[0]; 
    /* Make sure the rank and file are within the bounds 
       set by the current board size. */  
    if (c >= 'a' && c <= 'z') c = c - 'a' + 'A'; 
    if(c > maxChar || c < 'A') return FALSE; 
    c = input[2]; 
    if (c >= 'a' && c <= 'z') c = c - 'a' + 'A'; 
    if(c > maxChar || c < 'A') return FALSE; 
    c = input[1]; 
    if(c > maxInt || c < '1') return FALSE; 
    c = input[3]; 
    if(c > maxInt || c < '1') return FALSE; 
  } 
  return TRUE; 
} 
 
 
/************************************************************************ 
** 
** NAME:        ConvertTextInputToMove 
** 
** DESCRIPTION: Converts the string input your internal move representation. 
**         maxInt     Gamesman already checked the move with ValidTextInput 
**              and ValidMove. 
**  
** INPUTS:      STRING input : The VALID string input from the user. 
** 
** OUTPUTS:     MOVE         : Move converted from user input. 
** 
************************************************************************/ 
 
MOVE ConvertTextInputToMove (STRING input) 
{ 
  // For now, the move can be represented as a hex number 
  return (MOVE)atobi(input,16); 
} 
 
 
/************************************************************************ 
** 
** NAME:        GameSpecificMenu 
** 
** DESCRIPTION: Prints, receives, and sets game-specific parameters. 
** 
**              Examples 
**              Board Size, Board Type 
** 
**              If kGameSpecificMenu == FALSE 
**                   Gamesman will not enable GameSpecificMenu 
**                   Gamesman will not call this function 
**  
**              Resets gNumberOfPositions if necessary 
** 
************************************************************************/ 
 
void GameSpecificMenu () 
{ 
  printf("\n"); 
  printf("Tile Chess Game Specific Menu\n\n"); 
  printf("1) Bishop Variant\n"); 
  printf("2) Rook Variant\n"); 
  printf("b) Back to previous menu\n\n"); 
   
  printf("Select an option: "); 
   
  switch(GetMyChar()) { 
  case 'Q': case 'q': 
    ExitStageRight(); 
  case '1': 
    bishopVariant = TRUE; 
    rookVariant = FALSE; 
    break; 
  case '2': 
    rookVariant = TRUE; 
    bishopVariant = FALSE; 
    break; 
  case 'b': case 'B': 
    return; 
  default: 
    printf("\nSorry, I don't know that option. Try another.\n"); 
    HitAnyKeyToContinue(); 
    GameSpecificMenu(); 
    break; 
  } 
} 
 
/************************************************************************ 
** 
** NAME:        SetTclCGameSpecificOptions 
** 
** DESCRIPTION: Set the C game-specific options (called from Tcl) 
**              Ignore if you don't care about Tcl for now. 
**  
************************************************************************/ 
 
void SetTclCGameSpecificOptions (int options[]) 
{ 
     
} 
   
   
/************************************************************************ 
** 
** NAME:        GetInitialPosition 
** 
** DESCRIPTION: Called when the user wishes to change the initial 
**              position. Asks the user for an initial position. 
**              Sets new user defined gInitialPosition and resets 
**              gNumberOfPositions if necessary 
**  
** OUTPUTS:     POSITION : New Initial Position 
** 
************************************************************************/ 
 
POSITION GetInitialPosition () 
{ 
  return gInitialPosition; 
} 
 
 
/************************************************************************ 
** 
** NAME:        NumberOfOptions 
** 
** DESCRIPTION: Calculates and returns the number of variants 
**              your game supports. 
** 
** OUTPUTS:     int : Number of Game Variants 
** 
************************************************************************/ 
 
int NumberOfOptions () 
{ 
  return 2; 
} 
 
 
/************************************************************************ 
** 
** NAME:        getOption 
** 
** DESCRIPTION: A hash function that returns a number corresponding 
**              to the current variant of the game. 
**              Each set of variants needs to have a different number. 
** 
** OUTPUTS:     int : the number representation of the options. 
** 
************************************************************************/ 
 
int getOption () 
{ 
  if (bishopVariant) 
    return 1; 
  else if (rookVariant) 
    return 2; 
  else 
    BadElse("getOption"); 
 
  return -1; 
} 
 
 
/************************************************************************ 
** 
** NAME:        setOption 
** 
** DESCRIPTION: The corresponding unhash function for game variants. 
**              Unhashes option and sets the necessary variants. 
** 
** INPUT:       int option : the number representation of the options. 
** 
************************************************************************/ 
 
void setOption (int option) 
{ 
  if (option == 1) { 
    bishopVariant = TRUE; 
    rookVariant = FALSE; 
  } else if (option == 2) { 
    bishopVariant = FALSE; 
    rookVariant = TRUE; 
  } 
} 
 
 
/************************************************************************ 
** 
** NAME:        DebugMenu 
** 
** DESCRIPTION: Game Specific Debug Menu (Gamesman comes with a default 
**              debug menu). Menu used to debug internal problems. 
** 
**              If kDebugMenu == FALSE 
**                   Gamesman will not display a debug menu option 
**                   Gamesman will not call this function 
**  
************************************************************************/ 
 
void DebugMenu () 
{ 
     
} 
 
 
/************************************************************************ 
** 
** Everything specific to this module goes below these lines. 
** 
** Things you want down here: 
** Move Hasher 
** Move Unhasher 
** Any other function you deem necessary to help the ones above. 
**  
************************************************************************/ 
 
/* Returns a string representation of theMove */ 
STRING MToS(MOVE theMove) { 
  STRING move = (STRING) SafeMalloc(2); 
  sprintf(move, "%x", theMove); 
  return move; 
} 
 
/* Reverses the order of a string 
   PRECONDITION: s must end with a null character 
   POSTCONDITION: the characters in s are reversed */ 
void reverse(char s[]) { 
  int c, i, j; 
  for (i = 0, j = strlen(s)-1; i < j; i++, j--) { 
    c = s[i]; 
    s[i] = s[j]; 
    s[j] = c; 
  } 
} 
 
/* Returns the string of base 'base' as an integer 
   PRECONDITION: s must end in a null character, and contain only the digits 
   0-9, and the letters a-f (hexadecimal) */ 
int atobi(char s[], int base) { 
  int i; 
  int total = 0; 
  char c; 
  for (i = 0; s[i] != '\0'; i++) { 
    c = s[i]; 
    total = total*base + ((isalpha(c)) ? toupper(c)-'A'+10:c-'0'); 
  } 
  return total; 
} 
 
/* Sets s[] to be a string representation of a 
   base 'base' number n 
   POSTCONDITION: s ends in a null character contains 
   the digits of n*/ 
void itoa(int n, char s[], int base){ 
  int i, sign, nmodbase; 
  if ((sign = n) <0) 
    n = -n; 
  i=0; 
  do { 
    if ((nmodbase = n%base) > 9) 
      s[i++] = nmodbase-10 + 'a'; 
    else 
      s[i++] = n% base + '0'; 
  } while((n = n / base)>0); 
  if(sign<0) 
    s[i++] = '-'; 
  s[i] = '\0'; 
  reverse(s); 
} 
 
/* Fills the global arrays indexToBoard and boardToIndex 
   with the values of valid placements.  Also sets up piece 
   hashing/unhashing based on what the variant is set to. */ 
void PreProcess() { 
  int placement, legalcounter; 
  legalcounter = 0; 
  for (placement = 0; placement < 512; placement++) { 
    if (isLegalPlacement(placement,3)) { 
      indexToBoard[legalcounter] = placement; 
      boardToIndex[placement] = legalcounter; 
      legalcounter++; 
    } 
  } 
  if (rookVariant) { 
    piecesunhashed[0] = "kKR"; 
    piecesunhashed[1] = "kRK"; 
    piecesunhashed[2] = "RkK"; 
    piecesunhashed[3] = "RKk"; 
    piecesunhashed[4] = "KRk"; 
    piecesunhashed[5] = "KkR"; 
  } else if (bishopVariant) { 
    piecesunhashed[0] = "kKB"; 
    piecesunhashed[1] = "kBK"; 
    piecesunhashed[2] = "BkK"; 
    piecesunhashed[3] = "BKk"; 
    piecesunhashed[4] = "KBk"; 
    piecesunhashed[5] = "KkB"; 
  } 
} 
 
/* Returns TRUE if the strings s and t are equal, and FALSE 
   otherwise. 
   PRECONDITION: s and t both end in a null character */ 
BOOLEAN isEqualString(char s[], char t[]) { 
  int i; 
  for (i = 0; s[i] != '\0' && t[i] != '\0'; i++) { 
    if (s[i] != t[i]) { 
      return FALSE; 
    } 
  } 
  return TRUE; 
} 
 
/* Returns a number representation of a given 
   arrangement of pieces for the rook variant. 
   This function essentially handles how pieces are 
   hashed. */ 
int hashRookPieces(char pieceArray[]) { 
  if (isEqualString(pieceArray,"kKR")) 
    return 0; 
  else if (isEqualString(pieceArray,"kRK")) 
    return 1; 
  else if (isEqualString(pieceArray,"RkK")) 
    return 2; 
  else if (isEqualString(pieceArray,"RKk")) 
    return 3; 
  else if (isEqualString(pieceArray,"KRk")) 
    return 4; 
  else if (isEqualString(pieceArray,"KkR")) 
    return 5; 
  else  
    return -1; 
} 
 
/* Returns a number representation of a given 
   arrangement of pieces for the bishop variant. 
   This function essentially handles how pieces are 
   hashed. */ 
int hashBishopPieces(char pieceArray[]) { 
  if (isEqualString(pieceArray,"kKB")) 
    return 0; 
  else if (isEqualString(pieceArray,"kBK")) 
    return 1; 
  else if (isEqualString(pieceArray,"BkK")) 
    return 2; 
  else if (isEqualString(pieceArray,"BKk")) 
    return 3; 
  else if (isEqualString(pieceArray,"KBk")) 
    return 4; 
  else if (isEqualString(pieceArray,"KkB")) 
    return 5; 
  else 
    return -1; 
} 
 
/* Checks to see if any piece is separated from the rest of the pieces. 
   For each piece (1) that it finds, it makes sure that there is at least 
   1 other piece around it.  If it is a legal board, this function returns 
   1.  Otherwise, it returns 0. */ 
int isLegalPlacement(int place, int sideLength) { 
  char *pA; 
  int i, pieces, surrounding, boardSize; 
  pieces = surrounding = 0;
  boardSize = (int) pow(sideLength,2);
  pA = SafeMalloc((boardSize+2) * sizeof(char)); 
  place = place | (int)pow(2,boardSize); 
  itoa(place,pA,2);
  pA++; 
  for (i = 0; i < boardSize; i++) { 
    if (pA[i] == '1') { 
      pieces++; 
      if ((i-sideLength >= 0 && pA[i-sideLength] == '1') || 
	  (i+sideLength < boardSize && pA[i+sideLength] == '1') || 
	  ((i+1)%sideLength != 0 && pA[i+1] == '1') ||  
	  (i%sideLength != 0 && pA[i-1] == '1') || 
	  (i-sideLength >= 0 && (i+1)%sideLength != 0 && pA[i-sideLength+1] == '1') || 
	  (i-sideLength >= 0 && i%sideLength != 0 && pA[i-sideLength-1] == '1') || 
	  (i+sideLength < boardSize && (i+1)%sideLength != 0 && pA[i+sideLength+1] == '1') || 
	  (i+sideLength < boardSize && i%sideLength != 0 && pA[i+sideLength-1] == '1')) 
	surrounding++; 
      if (surrounding == 0) { 
	pA--; 
	SafeFree(pA); 
	return 0; 
      } else { 
	surrounding = 0; 
      } 
    } 
  } 
  pA--; 
  SafeFree(pA); 
  if (pieces != 3) { 
    return 0; 
  } else { 
    return 1; 
  } 
} 
 
/* Given a string which represents a board and the currentPlayer, 
   this function returns a POSITION, which is a number representing the 
   hashed value of the given board 
   EXAMPLE: 
    +---+---+---+---+---+ 
  3 |   |   |   |   |   | 
    +---+---+---+---+---+ 
  2 |   | K | B | k |   | 
    +---+---+---+---+---+ 
  1 |   |   |   |   |   | 
    +---+---+---+---+---+ 
      a   b   c   d   e 
      This board would be hashed as follows: 
      (((boardToIndex[7]*BMAX) + hashBishopPieces("KBk")) << 1) + currentPlayer 
      The number 7 comes  from a binary representation of a 3x3 board: 
      000 
      000 
      111  ------------> 000000111 -------> 7 */ 
POSITION hashBoard(char boardArray[], int currentPlayer) { 
  POSITION N; 
  int newPlacement = 0; 
  int B = 0, hashedPlacement; 
  char *snewPieces; 
  int sideLength = pow(strlen(boardArray),0.5); 
  int bi=0,pi=0,x,y; 
  int minx=sideLength, maxx=0, miny=sideLength, maxy=0; 
  snewPieces = SafeMalloc((TOTAL_PIECES+1) * sizeof(char)); 
   
  // Find the rectangle of the board that the pieces occupy 
  for(bi=0;bi<strlen(boardArray);bi++) { 
    if(boardArray[bi]!=' '){ 
      x = bi%sideLength; 
      y = bi/sideLength; 
      if(x<minx) minx=x; 
      if(x>maxx) maxx=x; 
      if(y<miny) miny=y; 
      if(y>maxy) maxy=y; 
    } 
  } 
 
  // Squarize the rectangle 
  while ((maxx-minx) < pow(BOARD_SIZE,0.5)-1) { 
    if(maxx < sideLength-1) 
      maxx++; 
    else { 
      int i, length = strlen(boardArray); 
      for (i = 0; i < length; i++) { 
	if ((i+1)%sideLength != 0) { 
	  boardArray[i] = boardArray[i + 1]; 
	} 
      } 
      minx--; 
    } 
  } 
  while ((maxy-miny) < pow(BOARD_SIZE,0.5)-1) { 
    if(miny>0) 
      miny--; 
    else { 
      int i, length = strlen(boardArray); 
      for (i = length-1; i >= sideLength; i--) { 
	boardArray[i] = boardArray[i - sideLength]; 
      } 
      maxy++; 
    } 
  } 
   
 
  /* Go through the spaces on the board inside the square just obtained. 
     If a piece is found, put a 1 in the placement number corresponding to 
     the place it was found. Then add that piece to the pieces string.*/ 
  for(y=miny;y<=maxy;y++){ 
    for(x=minx;x<=maxx;x++){ 
      bi=y*sideLength+x; 
      if(boardArray[bi]!=' '){ 
	newPlacement = newPlacement | (1<<(BOARD_SIZE-((y-miny)*(maxy-miny+1)+(x-minx))-1)); 
	snewPieces[pi++] = boardArray[bi]; 
      }  
    } 
  } 
  hashedPlacement = boardToIndex[newPlacement]; 
  if (rookVariant) { 
    B = hashRookPieces(snewPieces); 
  } else if (bishopVariant) { 
    B = hashBishopPieces(snewPieces); 
  } 
  N = ((hashedPlacement*BMAX+B) << 1) + currentPlayer; 
  SafeFree(snewPieces); 
  return N; 
} 
 
/* Returns whose turn it is based on the given position */ 
int getCurrTurn(POSITION position) { 
  return (position & 1); 
} 
 
/* Given a hashed value, this function returns a string 
   which represents a board */ 
char *unhashBoard(POSITION position) { 
  int A, B, currentPlayer; 
  char *pieces; 
  char *placement; 
  char *bA; 
  currentPlayer = getCurrTurn(position); 
  position = position >> 1; // Remove the bit indicating player turn 
  placement = SafeMalloc((BOARD_SIZE+2) * sizeof(char)); // +2 is for border 
  A = indexToBoard[position/BMAX] | (int)pow(2,BOARD_SIZE); 
  B = position%BMAX; 
  itoa(A,placement,2); 
  pieces = piecesunhashed[B]; 
  bA = FillBoardArray(pieces,placement); 
  SafeFree(placement); 
  return bA; 
} 
 
/* Given a placement array (string of 1's and 0's) and a string of pieces, will 
   return a board with the corresponding pieces placed according to the placeArray. */ 
char *FillBoardArray(char *pieceArray, char *placeArray) { 
  int c, r, sideLength, displayBoardSize; 
  char *bA; 
  sideLength = (int)pow(BOARD_SIZE,0.5)+2; // +2 is for border 
  displayBoardSize = (int)pow(sideLength,2); 
  bA = SafeMalloc((displayBoardSize + 1) * sizeof(char)); 
  placeArray++; //Ignore bit that indicates number of significant bits 
  for(c = 0; c < sideLength; c++){ 
    bA[c] = ' '; // Top border 
  } 
  for (r = 1; r < sideLength-1; r++) { 
    bA[r*sideLength] = ' '; // Left border 
    for(c = 1; c < sideLength-1; c++){ 
      if (*placeArray == '1') { 
	bA[r*sideLength+c] = *pieceArray; 
	pieceArray++; 
      } 
      else { 
	bA[r*sideLength+c] = ' '; // Right border 
      } 
      placeArray++; 
    } 
    bA[r*sideLength+c] = ' '; 
  } 
  for(c = 0; c < sideLength; c++){ 
    bA[displayBoardSize-sideLength+c] = ' '; // Bottom border 
  } 
  bA[displayBoardSize] = '\0'; 
  return bA; 
} 
 
/* Given a position N, a MOVELIST, and the placement of the king, 
   this function "creates" all of the available moves for the king. 
   POSTCONDITION: moves is updated with all the legal moves. */ 
void generateKingMoves(POSITION N, MOVELIST **moves, int place) { 
  int direction, sidelength; 
  char *boardArray = unhashBoard(N); 
  sidelength = pow(strlen(boardArray),0.5); 
  /*MOVE UP*/ 
  direction = place-sidelength; 
  if (testMove(direction,place,N) == TRUE) { 
    *moves = CreateMovelistNode(createMove(place,direction,sidelength), *moves); 
  } 
  /*MOVE DOWN*/ 
  direction = place+sidelength; 
  if (testMove(direction,place,N) == TRUE) { 
    *moves = CreateMovelistNode(createMove(place,direction,sidelength), *moves); 
  } 
  /*MOVE RIGHT*/ 
  direction = place+1; 
  if (testMove(direction,place,N) == TRUE) { 
    *moves = CreateMovelistNode(createMove(place,direction,sidelength), *moves); 
  } 
  /*MOVE LEFT*/ 
  direction = place-1; 
  if (testMove(direction,place,N) == TRUE) { 
    *moves = CreateMovelistNode(createMove(place,direction,sidelength), *moves); 
  } 
  /*MOVE UPPER-RIGHT*/ 
  direction = place-sidelength+1; 
  if (testMove(direction,place,N) == TRUE) { 
    *moves = CreateMovelistNode(createMove(place,direction,sidelength), *moves); 
  } 
  /*MOVE UPPER-LEFT*/ 
  direction = place-sidelength-1; 
  if (testMove(direction,place,N) == TRUE) { 
    *moves = CreateMovelistNode(createMove(place,direction,sidelength), *moves); 
  } 
  /*MOVE LOWER-RIGHT*/ 
  direction = place+sidelength+1; 
  if (testMove(direction,place,N) == TRUE) { 
    *moves = CreateMovelistNode(createMove(place,direction,sidelength), *moves); 
  } 
  /*MOVE LOWER-LEFT*/ 
  direction = place+sidelength-1; 
  if (testMove(direction,place,N) == TRUE) { 
    *moves = CreateMovelistNode(createMove(place,direction,sidelength), *moves); 
  } 
} 
 
/* Given a position N, a MOVELIST, and the placement of the bishop, 
   this function "creates" all of the available moves for the bishop. 
   POSTCONDITION: moves is updated with all the legal moves. */ 
void generateBishopMoves(POSITION N, MOVELIST **moves, int place) { 
  generateMovesDirection(N,moves,place,UL); 
  generateMovesDirection(N,moves,place,UR); 
  generateMovesDirection(N,moves,place,DL); 
  generateMovesDirection(N,moves,place,DR); 
} 
 
/* Given a position N, a MOVELIST, and the placement of the rook, 
   this function "creates" all of the available moves for the rook. 
   POSTCONDITION: moves is updated with all the legal moves. */ 
void generateRookMoves(POSITION N, MOVELIST **moves, int place) { 
  generateMovesDirection(N,moves,place,UP); 
  generateMovesDirection(N,moves,place,DOWN); 
  generateMovesDirection(N,moves,place,LEFT); 
  generateMovesDirection(N,moves,place,RIGHT); 
} 
 
/* Given a position N, a MOVELIST, the placement of a piece, and 
   a direction, this function will "create" all of the available/legal 
   moves of the piece at place in the given direction. 
   POSTCONDITION: moves is updated with all of the legal moves. */ 
void generateMovesDirection(POSITION N, MOVELIST **moves, int place, int direction) { 
  int i, iterations=0, sideLength, x, y, x_inc=0, y_inc=0; 
  char piece; 
  char *boardArray = unhashBoard(N); 
  sideLength = (int) sqrt(BOARD_SIZE)+2; 
  x = place % sideLength; 
  y = place / sideLength; 
  switch (direction){ 
  case UP:  
    y_inc = -1; 
    iterations = y; 
    break; 
  case DOWN:  
    y_inc = 1; 
    iterations = sideLength - 1 - y; 
    break; 
  case LEFT:  
    x_inc = -1; 
    iterations = x; 
    break; 
  case RIGHT:  
    x_inc = 1; 
    iterations = sideLength - 1 - x; 
    break; 
  case UL:  
    y_inc = -1; 
    x_inc = -1; 
    iterations = ((x<y)? x : y); 
    break; 
  case UR:  
    y_inc = -1; 
    x_inc = 1;  
    iterations = ((y<(sideLength - 1 - x))? y : sideLength - 1 - x); 
    break; 
  case DL:  
    y_inc = 1; 
    x_inc = -1; 
    iterations = ((x<(sideLength - 1 - y))? x : sideLength - 1 - y); 
    break; 
  case DR:  
    y_inc = 1; 
    x_inc = 1; 
    iterations = ((x>y)? x : y); 
    break; 
  } 
  for(i=0; i<iterations; i++){ 
    x += x_inc; 
    y += y_inc; 
    piece = boardArray[y*sideLength+x]; 
    if(piece != ' ') 
      return; 
    else { 
      if (testMove(y*sideLength+x,place,N)) { 
	*moves = CreateMovelistNode(createMove(place,y*sideLength+x,sideLength), *moves); 
      } 
    } 
  } 
} 
 
/* Returns TRUE if the given piece belongs to currentPlayer, 
   and FALSE otherwise. */ 
BOOLEAN isSameTeam(char piece, int currentPlayer) { 
  if (piece >= 'a' && piece <= 'z' && currentPlayer == PLAYER2_TURN) 
    return TRUE; 
  else if (piece >= 'A' && piece <= 'Z' && currentPlayer == PLAYER1_TURN) 
    return TRUE; 
  else  
    return FALSE; 
} 
 
/* Given the starting and ending indeces, will return 
   a MOVE representing this move. 
   PRECONDITION: The move is legal */ 
MOVE createMove(int init, int final, int sidelength) { 
  MOVE m; 
  int filei, ranki, filef, rankf; 
  filei = 10+init%sidelength; 
  filef = 10+final%sidelength; 
  ranki = sidelength-(init/sidelength); 
  rankf = sidelength-(final/sidelength); 
  m = (filei << 12) | (ranki << 8) | (filef << 4) | rankf; 
  return m; 
} 
 
/* Checks to see whether the move will create an illegal board -  
   Checks to see if it will result in an illegal placement, 
   or if the move will cause the player to be in check.  Returns 
   TRUE if the move is legal, and FALSE otherwise. */ 
BOOLEAN testMove(int newspot, int origspot, POSITION N) { 
  int sidelength, A; 
  char newpiece; 
  POSITION NEW_N; 
  char *bA = unhashBoard(N); 
  sidelength = pow(strlen(bA),0.5); 
  newpiece = bA[newspot]; 
  if (!isSameTeam(newpiece,getCurrTurn(N))) { 
    bA[newspot] = bA[origspot]; 
    bA[origspot] = ' '; 
    if (isLegalBoard(bA)) { 
      NEW_N = hashBoard(bA,getCurrTurn(N)); 
      A = indexToBoard[(NEW_N >> 1)/BMAX]; 
      if (!inCheck(NEW_N)) { 
	bA[origspot] = bA[newspot]; 
	bA[newspot] = newpiece; 
	return TRUE; 
      } 
    } 
    bA[origspot] = bA[newspot]; 
    bA[newspot] = newpiece; 
  } 
  return FALSE; 
} 
 
/* Determines if the player is currently in check based on 
   the given position.  Returns TRUE if the player is in check, 
   and FALSE otherwise. */ 
BOOLEAN inCheck(POSITION N) { 
  int i; 
  char piece; 
  int currentPlayer = getCurrTurn(N); 
  char *bA = unhashBoard(N); 
  for (i = 0; i < strlen(bA); i++) { 
    piece = bA[i]; 
    if (!isSameTeam(piece,currentPlayer)) { 
      if (piece >= 'a' && piece <= 'z') { 
	piece = piece - 'a' + 'A'; 
      } 
      switch (piece) { 
      case 'K':  
	if (kingCheck(bA, i, currentPlayer) == TRUE) { 
	  return TRUE; 
	} else { 
	  break;    				 
	} 
      case 'B': 
	if (bishopCheck(bA, i, currentPlayer) == TRUE) { 
	  return TRUE; 
	} else { 
	  break; 
	} 
      case 'R': 
	if (rookCheck(bA, i, currentPlayer) == TRUE) { 
	  return TRUE; 
	} else { 
	  break; 
	} 
      default:  
	break; 
      } 
    } 
  } 
  return FALSE; 
} 
 
/* Checks if the king is checking the opposing king based on the given 
   board, the placement of the king that might be checking, and the currentPlayer. */ 
BOOLEAN kingCheck(char *bA, int place, int currentPlayer) { 
  char king; 
  int sidelength; 
  king = (currentPlayer == PLAYER2_TURN) ? 'k' : 'K'; 
  sidelength = pow(strlen(bA),0.5); 
  return  ((bA[place-sidelength]==king) || 
	   (bA[place+sidelength]==king) || 
	   (bA[place+1]==king) || 
	   (bA[place-1]==king) || 
	   (bA[place-sidelength+1]==king) || 
	   (bA[place-sidelength-1]==king) || 
	   (bA[place+sidelength+1]==king) || 
	   (bA[place+sidelength-1]==king)) ? TRUE : FALSE; 
} 
 
/* Checks if the bishop is checking the opposing king based on the given 
   board, the placement of the bishop, and the currentPlayer. */ 
BOOLEAN bishopCheck(char *bA, int place, int currentPlayer) { 
  char king; 
  int sidelength; 
  king = (currentPlayer == PLAYER2_TURN) ? 'k' : 'K'; 
  sidelength = pow(strlen(bA),0.5); 
  return ((bA[place-sidelength+1]==king) || 
	  (bA[place-sidelength-1]==king) || 
	  (bA[place+sidelength+1]==king) || 
	  (bA[place+sidelength-1]==king)) ? TRUE : FALSE; 
} 
 
/* Checks if the rook is checking the opposing king based on the given 
   board, the placement of the rook, and the currentPlayer. */ 
BOOLEAN rookCheck(char *bA, int place, int currentPlayer) { 
  char king; 
  int sidelength; 
  king = (currentPlayer == PLAYER2_TURN) ? 'k' : 'K'; 
  sidelength = pow(strlen(bA),0.5); 
  return ((bA[place-sidelength]==king) || 
	  (bA[place+sidelength]==king) || 
	  (bA[place+1]==king) || 
	  (bA[place-1]==king)) ? TRUE : FALSE; 
} 
 
/* Checks if the king has any legal moves based on the given 
   position.  Returns TRUE if the king has a legal move, 
   and FALSE otherwise. */ 
BOOLEAN canMove(POSITION position) { 
  int currentPlayer = getCurrTurn(position); 
  MOVELIST *moves = NULL; 
  char *bA = unhashBoard(position); 
  char king = (currentPlayer == PLAYER1_TURN) ? 'K' : 'k'; 
  int length = strlen(bA); 
  int i, kingplace; 
  for(i = 0; i < length; i++) { 
    if (bA[i] == king) { 
      kingplace = i; 
      break; 
    } 
  } 
  generateKingMoves(position,&moves,i); 
  return !(moves == NULL); 
} 
 
/* Temporary work around so that we do not need to hash a board that is 
   not of size 9 - Our preprocessed arrays only deal with this size. 
   Same as isLegalPlacement, but takes a board instead of a placement. */ 
BOOLEAN isLegalBoard(char *bA) {
  int i, place, sideLength, length = strlen(bA); 
  char temp[length+1]; 
  for (i = 0; bA[i] != '\0'; i++) { 
    if (bA[i] == ' ') { 
      temp[i] = '0'; 
    } else { 
      temp[i] = '1'; 
    } 
  } 
  temp[i] = '\0';
  sideLength = (int) pow(length,0.5); 
  place = atobi(temp,2);
  return isLegalPlacement(place, sideLength);
} 
 
// $Log: not supported by cvs2svn $
// Revision 1.7  2005/12/27 10:57:50  hevanm
// almost eliminated the existance of gDatabase in all files, with some declarations commented earlier that need to be hunt down and deleted from the source file.
//
// Revision 1.6  2005/12/23 01:34:12  brianzimmer
// Final version of Tile Chess that adheres to the 10 point checklist for the Fall 2005 semester.
//
// Revision 1.5  2005/12/19 11:11:04  brianzimmer
// Cleaned up and commented final version of Tile Chess for the Fall 2005 semester.
// 
// Revision 1.4  2005/12/17 04:31:55  zwizeguy 
// Implemented rook/bishop variants, and misere option. 
// 
// Revision 1.5  2005/10/06 03:06:11  hevanm 
// Changed kDebugDetermineValue to be FALSE. 
// 
// Revision 1.4  2005/05/02 17:33:01  nizebulous 
// mtemplate.c: Added a comment letting people know to include gSymmetries 
// 	     in their getOption/setOption hash. 
// mttc.c: Edited to handle conflicting types.  Created a PLAYER type for 
//         gamesman.  mttc.c had a PLAYER type already, so I changed it. 
// analysis.c: Changed initialization of option variable in analyze() to -1. 
// db.c: Changed check in the getter functions (GetValueOfPosition and 
//       getRemoteness) to check if gMenuMode is Evaluated. 
// gameplay.c: Removed PlayAgainstComputer and PlayAgainstHuman.  Wrote PlayGame 
//             which is a generic version of the two that uses to PLAYER's. 
// gameplay.h: Created the necessary structs and types to have PLAYER's, both 
// 	    Human and Computer to be sent in to the PlayGame function. 
// gamesman.h: Really don't think I changed anything.... 
// globals.h: Also don't think I changed anything....both these I just looked at 
//            and possibly made some format changes. 
// textui.c: Redid the portion of the menu that allows you to choose opponents 
// 	  and then play a game.  Added computer vs. computer play.  Also, 
//           changed the analysis part of the menu so that analysis should 
// 	  work properly with symmetries (if it is in getOption/setOption hash). 
// 
// Revision 1.3  2005/03/10 02:06:47  ogren 
// Capitalized CVS keywords, moved Log to the bottom of the file - Elmer 
// 
