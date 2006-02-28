/************************************************************************
**
** NAME:	analysis.c
**
** DESCRIPTION:	Contains the majority of game analysis functionality.
**
** AUTHOR:	GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2005-01-11
**
** LICENSE:	This file is part of GAMESMAN,
**		The Finite, Two-person Perfect-Information Game Generator
**		Released under the GPL:
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program, in COPYING; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
**************************************************************************/

#include "gamesman.h"
#include "analysis.h"
#include "db.h"


/*
** Globals
*/

long		gTotalMoves = 0;
ANALYSIS	gAnalysis = {};

/*
** Local variables
*/
POSITION thePosition;
VALUE theValue;
POSITION winCount, loseCount, tieCount, unknownCount = 0;	// CHANGED from UINT (change back if no worky)
POSITION primitiveWins, primitiveLoses, primitiveTies = 0;
POSITION reachablePositions = 0;
POSITION totalPositions;
int  hashEfficiency = 0;
float averageFanout = 0;
REMOTENESS theRemoteness;	/*	For extended analysis including remoteness */
REMOTENESS theLargestRemoteness = 0;  // for keeping track of largest found remoteness.  For efficiency and a good statistic.

/*
** Code
*/

void PrintRawGameValues(BOOLEAN toFile)
{
    FILE *fp;
    char filename[80];
    POSITION i;
    VALUE value;
    
    if(toFile) {
        printf("File to save to: ");
        scanf("%s",filename);
	
        if((fp = fopen(filename, "w")) == NULL) {
            ExitStageRightErrorString("Couldn't open file, sorry.");
            exit(0);
        }
	printf("Writing to %s...", filename);
	fflush(stdout);
    } else
        fp = stdout;
    
    if (!toFile) printf("\n");
    fprintf(fp,"%s\n", kGameName);
    fprintf(fp,"Position,Value,Remoteness%s\n",
	    (!kPartizan && !gTwoBits) ? ",MexValue" : "");
    
    for(i=0 ; i<gNumberOfPositions ; i++)
      if((value = GetValueOfPosition((POSITION)i)) != undecided) {
            fprintf(fp,POSITION_FORMAT ",%c,%d",
		    i,
		    gValueLetter[value],
		    Remoteness((POSITION)i));
	    if(!kPartizan && !gTwoBits)
	      fprintf(fp,",%d\n",MexLoad((POSITION)i));
	    else
	      fprintf(fp,"\n");
      }
    
    if(toFile) {
      fclose(fp);
      printf("done\n");
    }
}

void PrintBadPositions(char c,int maxPositions, POSITIONLIST* badWinPositions, POSITIONLIST* badTiePositions, POSITIONLIST* badLosePositions)
{
    POSITIONLIST *ptr = NULL;
    BOOLEAN continueSearching = TRUE;
    POSITION thePosition;
    int j;
    char yesOrNo;
    
    if     (c == '1') ptr = badWinPositions;
    else if(c == '2') ptr = badTiePositions;
    else if(c == '3') ptr = badLosePositions;
    else BadElse("PrintBadPositions");
    
    j = 0;
    continueSearching = TRUE;
    do {
        for(j = 0 ; ptr != NULL && j < maxPositions ;j++) {
            thePosition = ptr->position;
            PrintPosition(thePosition, "Nobody", TRUE);
            ptr = ptr->next;
        }
        if(ptr != NULL) {
            printf("\nDo you want more? [Y/N] : ");
            scanf("%c",&yesOrNo);
            scanf("%c",&yesOrNo);
            continueSearching = (yesOrNo == 'y' || yesOrNo == 'Y');
        }
        else
            printf("\nThere are no more %s positions to list...\n",
		   c == '1' ? "winning" : c == '3' ? "losing" : "tieing");
    } while (continueSearching && ((ptr = ptr->next) != NULL));
}

void PrintMexValues(MEX mexValue, int maxPositions)
{
    BOOLEAN continueSearching = TRUE;
    POSITION thePosition;
    int j;
    char yesOrNo;
    
    j = 0;
    continueSearching = TRUE;
    do {
        for(j = 0 ; ((thePosition = GetNextPosition()) != kBadPosition) && j < maxPositions ;) {
            if (MexLoad(thePosition) == mexValue) {
                PrintPosition(thePosition, "Nobody", TRUE);
                j++;
            }
        }
        if(thePosition != kBadPosition) {
            printf("\nDo you want more? [Y/N] : ");
            scanf("%c",&yesOrNo);
            scanf("%c",&yesOrNo);
            continueSearching = (yesOrNo == 'y' || yesOrNo == 'Y');
        }
        else
            printf("\nThere are no more positions with Mex value %d to list...\n",
		   (int)mexValue);
    } while (continueSearching && (thePosition != kBadPosition));
    HitAnyKeyToContinue();
}

void PrintValuePositions(char c, int maxPositions)
{      
    BOOLEAN continueSearching = TRUE;
    POSITION thePosition;
    VALUE theValue;
    int j;
    char yesOrNo;
    
    j = 0;
    continueSearching = TRUE;
    do {
        for(j = 0 ; ((thePosition = GetNextPosition()) != kBadPosition) && j < maxPositions ;) {
            theValue = GetValueOfPosition(thePosition);
            if((theValue == win  && (c == 'w' || c == 'W')) || 
	       (theValue == lose && (c == 'l' || c == 'L')) ||
	       (theValue == tie  && (c == 't' || c == 'T'))) {
		PrintPosition(thePosition, "Nobody", TRUE);
		j++;
	    }
        }
        if(thePosition != kBadPosition) {
            printf("\nDo you want more? [Y/N] : ");
            scanf("%c",&yesOrNo);
            scanf("%c",&yesOrNo);
            continueSearching = (yesOrNo == 'y' || yesOrNo == 'Y');
        }
        else
            printf("\nThere are no more %s positions to list...\n",
		   c == 'w' || c == 'W' ? "winning" : c == 'l' || c == 'L' ? "losing" : "tieing");
    } while (continueSearching && (thePosition != kBadPosition));
    HitAnyKeyToContinue();
}

void PrintDetailedGameValueSummary()
{
   char *initialPositionValue = "";
   REMOTENESS currentRemoteness;

   switch(gAnalysis.InitialPositionValue)
   {
       case win:
           initialPositionValue = "Win";
           break;
       case lose:
           initialPositionValue = "Lose";
           break;
       case tie:
           initialPositionValue = "Tie";
           break;
       default:
           BadElse("PrintGameValueSummary [InitialPositionValue]");
   }
   printf("\n\n\t----- Detailed Summary of Game values -----\n\n");

   printf("\tRemoteness           Win         Lose          Tie\n");
   printf("\t-------------------------------------------------------\n");

   for(currentRemoteness = gAnalysis.LargestFoundRemoteness; currentRemoteness >= 0; currentRemoteness-=1) {
       if(gAnalysis.DetailedPositionSummary[currentRemoteness][0] == 0 && gAnalysis.DetailedPositionSummary[currentRemoteness][1] == 0
           && gAnalysis.DetailedPositionSummary[currentRemoteness][2] == 0) continue;

       printf("\t%10u    %10llu   %10llu   %10llu\n", currentRemoteness, gAnalysis.DetailedPositionSummary[currentRemoteness][0],
           gAnalysis.DetailedPositionSummary[currentRemoteness][1], gAnalysis.DetailedPositionSummary[currentRemoteness][2]);
   }

   printf("\t-------------------------------------------------------\n");
   printf("\tTotals        %10llu   %10llu   %10llu\n", gAnalysis.WinCount, gAnalysis.LoseCount, gAnalysis.TieCount);
   if (gAnalysis.Draws != 0)
      printf("\tDraws = %llu\tFringe0Edges = %llu\n", gAnalysis.Draws, gAnalysis.F0count);
   printf("\n\tTotal Positions Visited: %llu\n", gAnalysis.TotalPositions);
   

   return;
}


void PrintGameValueSummary()
{
    char *initialPositionValue = "";
    switch(gAnalysis.InitialPositionValue)
    {
        case win:
            initialPositionValue = "Win";
            break;
        case lose:
            initialPositionValue = "Lose";
            break;
        case tie:
            initialPositionValue = "Tie";
            break;
        default:
            BadElse("PrintGameValueSummary [InitialPositionValue]");
    }
    printf("\n\n\t----- Summary of Game values -----\n\n");
    
    printf("\tValue       Number       Total\n");
    printf("\t------------------------------\n");
    printf("\tLose      = %5llu out of %llu (%5llu primitive)\n",gAnalysis.LoseCount,gAnalysis.TotalPositions,gAnalysis.PrimitiveLoses);
    printf("\tWin       = %5llu out of %llu (%5llu primitive)\n",gAnalysis.WinCount,gAnalysis.TotalPositions, gAnalysis.PrimitiveWins);
    printf("\tTie       = %5llu out of %llu (%5llu primitive)\n",gAnalysis.TieCount,gAnalysis.TotalPositions,gAnalysis.PrimitiveTies);
	printf("\tDraw      = %5llu out of %llu\n",
		gAnalysis.Draws,  // ADDED to count draws
		gAnalysis.TotalPositions);
    printf("\tUnknown   = %5llu out of %llu (Sanity-check...should always be 0)\n",gAnalysis.UnknownCount,gAnalysis.TotalPositions);  
    printf("\tTOTAL     = %5llu out of %llu allocated (%5llu primitive)\n",
       gAnalysis.TotalPositions,
       gNumberOfPositions,
       gAnalysis.TotalPrimitives);
    
    printf("\tHash Efficiency                   = %6d\%%\n",gAnalysis.HashEfficiency);
    printf("\tTotal Moves                       = %5llu\n",gAnalysis.TotalMoves);
    printf("\tAvg. number of moves per position = %2f\n", gAnalysis.AverageFanout);
    printf("\tProbability of maintaining a %-5s= %2f\n", initialPositionValue,gAnalysis.InitialPositionProbability);
    
    return;
}



/** Analysis **/

VALUE AnalyzePosition(POSITION thePosition, VALUE theValue)
{
    if (theValue != undecided) {
        totalPositions++;
        if(theValue == win)  {
            winCount++;
            reachablePositions++;
            if ((theRemoteness = Remoteness(thePosition)) == 0) primitiveWins++;		// Stores remoteness on each call, saves data to array
            gAnalysis.DetailedPositionSummary[theRemoteness][0] += 1;
            if (theRemoteness > theLargestRemoteness) theLargestRemoteness = theRemoteness;  // Keeps track of the largest seen remoteness
        } else if(theValue == lose) {
            loseCount++;
            reachablePositions++;
            if ((theRemoteness = Remoteness(thePosition)) == 0) primitiveLoses++;
            gAnalysis.DetailedPositionSummary[theRemoteness][1] += 1;
            if (theRemoteness > theLargestRemoteness) theLargestRemoteness = theRemoteness;
        } else if(theValue == tie) {
            if ((theRemoteness = Remoteness(thePosition)) < REMOTENESS_MAX) 
            {
                tieCount++;
                gAnalysis.DetailedPositionSummary[theRemoteness][2] += 1;
                if (theRemoteness > theLargestRemoteness) theLargestRemoteness = theRemoteness;
                if (theRemoteness == 0) primitiveTies++;
            }
            reachablePositions++;
        } else {
            unknownCount++;
        }
    }

    return(theValue);
}
    
void AnalysisCollation()
{   
    hashEfficiency = (int)((((float)reachablePositions ) / (float)gNumberOfPositions) * 100.0); 
    averageFanout = (float)((float)gAnalysis.TotalMoves/(float)(reachablePositions - primitiveWins - primitiveLoses - primitiveTies));
    
    gAnalysis.InitialPositionValue = GetValueOfPosition(gInitialPosition);
    
    //gAnalysis.InitialPositionProbability = DetermineProbability(gInitialPosition,gAnalysis.InitialPositionValue);
    
    gAnalysis.HashEfficiency    = hashEfficiency;
    gAnalysis.AverageFanout     = averageFanout;
    gAnalysis.TotalPositions    = totalPositions;
    gAnalysis.WinCount          = winCount;
    gAnalysis.LoseCount         = loseCount;
    gAnalysis.TieCount          = tieCount;
    gAnalysis.Draws             = gAnalysis.TotalPositions - gAnalysis.WinCount - gAnalysis.LoseCount - gAnalysis.TieCount;
    gAnalysis.UnknownCount      = unknownCount;
    gAnalysis.PrimitiveWins     = primitiveWins;
    gAnalysis.PrimitiveLoses    = primitiveLoses;
    gAnalysis.PrimitiveTies     = primitiveTies;
    gAnalysis.NumberOfPositions = gNumberOfPositions;
    gAnalysis.TotalPrimitives   = gAnalysis.PrimitiveWins+gAnalysis.PrimitiveLoses+gAnalysis.PrimitiveTies;
	gAnalysis.LargestFoundRemoteness = theLargestRemoteness;  //ADDED
}

/* Determines the chance that you'll maintain your value if you *
 * randomly select a move given a position.                     */
float DetermineProbability(POSITION position, VALUE value)
{
    MOVELIST *ptr, *head;
    VALUE opposite_value=lose;
    VALUE primitive = Primitive(position);
    POSITION child;
    POSITION numChildren = 0;
    float probabilitySum = 0.0;
    
    switch(value)
    {
        case win:
            opposite_value = lose;
            break;
        case lose:
            opposite_value = win;
            break;
        case tie:
            opposite_value = tie;
            break;
        default:
            BadElse("DetermineProbability [next_level_value]");
    }
    
    
    if(primitive == value)
    {
        return 1.000;
    }
    else if (primitive == opposite_value)
    {
        return 0.0;
    }
    else if(Visited(position))
    {
        return 0.0;
    }
    else
    {
        MarkAsVisited(position);
        
        head = ptr = GenerateMoves(position);
        if(ptr == NULL) {return 0.0;}
        while(ptr != NULL)
        {
            child = DoMove(position, ptr->move);
           
            probabilitySum += DetermineProbability(child, opposite_value);
            
            numChildren++;
            ptr = ptr->next;
        }
        FreeMoveList(head);
       
        return (float)((float) probabilitySum / (float)numChildren);
       

    }
    
}


// Write variant statistic
void writeVarStat(char * statName, char * text, FILE *rowp)
{
    FILE * filep;
    //FILE * rawfilep ;
    char outFileName[256];
    
    sprintf(outFileName, "analysis/%s/var%d/%s", kDBName,getOption(),statName) ;
    
    filep = fopen(outFileName, "w");
    
    
    fprintf(filep,"<!-- AUTO CREATED, do //not modify-->\n");
    fprintf(filep,text);
    fprintf(filep,"\n");
    
    
    fprintf(rowp,"<td ALIGN = ""center""><!--#include virtual=\"%s\"--></td>\n",statName);
    
    
    fclose(filep);
    
    
}

void createAnalysisGameDir()
{
    char gameDirName[256];
    sprintf(gameDirName, "analysis/%s", kDBName);
    
    mkdir("analysis", 0755);
    mkdir(gameDirName, 0755);
    
}

void createAnalysisVarDir()
{
    char varDirName[256];
    sprintf(varDirName, "analysis/%s/var%d", kDBName,getOption());
    mkdir(varDirName, 0755) ;
}

BOOLEAN LoadAnalysis() {
    char gameFileName[256];
    char *line = NULL;
    int currentRemoteness;
    FILE *fp;

	createAnalysisGameDir();
    sprintf(gameFileName, "analysis/%s/%s_analysis.dat", kDBName,kDBName);
    
    if((fp = fopen(gameFileName, "rb")) == NULL) {
        printf("\nFailed to open analysis file for reading. A new one will be generated.");
        return FALSE;
    }
    printf("\nLoading Analysis Database for %s...", kGameName);
    /* read data from file */
    line = fgetline(fp);
    
    /* Check file version */
    if(line[0] != 2) {
        printf("\nError: Version mismatch. A new database will be generated.");
        return FALSE;
    }
    
    /* Read misc. info */
    line = fgetline(fp);
    gAnalysis.HashEfficiency = atoi(strtok(line, ","));
    gAnalysis.AverageFanout = atof(strtok(NULL, ",\n"));
    gAnalysis.NumberOfPositions = (POSITION) atol(strtok(NULL, ","));
    gAnalysis.TotalPositions = (POSITION) atol(strtok(NULL, ","));
    gAnalysis.TotalMoves = (POSITION) atol(strtok(NULL, ","));
    gAnalysis.TotalPrimitives = (POSITION) atol(strtok(NULL, ","));
    gAnalysis.WinCount = (POSITION) atol(strtok(NULL, ","));
    gAnalysis.LoseCount = (POSITION) atol(strtok(NULL, ","));
    gAnalysis.TieCount = (POSITION) atol(strtok(NULL, ","));
    gAnalysis.UnknownCount = (POSITION) atol(strtok(NULL, ","));
    gAnalysis.Draws = (POSITION) atol(strtok(NULL, ","));
    gAnalysis.PrimitiveWins = (POSITION) atol(strtok(NULL, ","));
    gAnalysis.PrimitiveLoses = (POSITION) atol(strtok(NULL, ","));
    gAnalysis.PrimitiveTies = (POSITION) atol(strtok(NULL, ","));
    gAnalysis.TimeToSolve = atoi(strtok(NULL, ","));
    gAnalysis.InitialPositionValue = atoi(strtok(NULL, ","));
    gAnalysis.InitialPositionProbability = atof(strtok(NULL, ","));
    gAnalysis.LargestFoundRemoteness = atoi(strtok(NULL, ","));
    gAnalysis.F0count = (POSITION) atol(strtok(NULL, ","));
    
    for(currentRemoteness = 0; currentRemoteness <= gAnalysis.LargestFoundRemoteness; currentRemoteness++) {
        line = fgetline(fp);
        gAnalysis.DetailedPositionSummary[currentRemoteness][0] = (POSITION) atol(strtok(line, ","));
        gAnalysis.DetailedPositionSummary[currentRemoteness][1] = (POSITION) atol(strtok(NULL, ","));
        gAnalysis.DetailedPositionSummary[currentRemoteness][2] = (POSITION) atol(strtok(NULL, ","));
    }
    printf("Done!");
    return TRUE;
}

/**
 *  Analysis database format:
 *  Header:
 *      First byte is version number (database format version 2 in this case)
 *      Second byte is '/n' to delimit header from data
 *  Other data:
 *      Misc. data will be on one line, delimited by commas. This includes
 *      total wins, loses, ties, etc.
 *  Remoteness Data:
 *      Each remoteness is on its own line. Data is stored in following order
 *      per line: win, lose, tie, with values delimited by a comma.
 */

void SaveAnalysis() {
    char gameFileName[256];
    createAnalysisGameDir();
    FILE *gamep;
    REMOTENESS currentRemoteness;
    
    sprintf(gameFileName, "analysis/%s/%s_analysis.dat", kDBName,kDBName);
    gamep = fopen(gameFileName, "wb");
    
    /* save data to file */
    fprintf(gamep, "%c\n", 2);
    
    fprintf(gamep, "%d,%f,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%u,%d,%f,%d,%llu\n",
            gAnalysis.HashEfficiency, gAnalysis.AverageFanout, gAnalysis.NumberOfPositions,
            gAnalysis.TotalPositions, gAnalysis.TotalMoves, gAnalysis.TotalPrimitives,
            gAnalysis.WinCount, gAnalysis.LoseCount, gAnalysis.TieCount,
            gAnalysis.UnknownCount, gAnalysis.Draws, gAnalysis.PrimitiveWins,
            gAnalysis.PrimitiveLoses, gAnalysis.PrimitiveTies, gAnalysis.TimeToSolve,
            gAnalysis.InitialPositionValue, gAnalysis.InitialPositionProbability, gAnalysis.LargestFoundRemoteness,
            gAnalysis.F0count);
    
    for(currentRemoteness = 0; currentRemoteness <= gAnalysis.LargestFoundRemoteness; currentRemoteness++) {
        if(gAnalysis.DetailedPositionSummary[currentRemoteness][0] == 0 
            && gAnalysis.DetailedPositionSummary[currentRemoteness][1] == 0
            && gAnalysis.DetailedPositionSummary[currentRemoteness][2] == 0) 
            continue;
           
        fprintf(gamep, "%llu,%llu,%llu\n",
                        gAnalysis.DetailedPositionSummary[currentRemoteness][0], 
                        gAnalysis.DetailedPositionSummary[currentRemoteness][1],
                        gAnalysis.DetailedPositionSummary[currentRemoteness][2]);
    }
    fclose(gamep);
}

void writeGameHTML()
{
    char gameFileName[256];
    FILE *gamep;
    
    
    STRING bgColor = "#000066";
    STRING fontColor = "#FFFFFF";
    STRING fontFace = "verdana";
    
    sprintf(gameFileName, "analysis/%s/%s.shtml", kDBName,kDBName);
    gamep = fopen(gameFileName, "w");
    
    fprintf(gamep, "<html><head>\n");
    fprintf(gamep, "<style>a:link, a:visited {color: %s\ntext-decoration: none;}\n\n", bgColor);
    fprintf(gamep, "a:hover, a:active {color: %s; text-decoration: none;}\ntd {color: %s}\n</style>\n", bgColor, fontColor);
    
    fprintf(gamep, "</head>\n");
    fprintf(gamep, "<body bgcolor=\"%s\">\n",bgColor);
    fprintf(gamep, "<font color = \"%s\" face = %s size = 2>", fontColor, fontFace);
    
    // a picture of the game
    fprintf(gamep, "<center>\n");
    fprintf(gamep, "<img src=\"../images/%s.gif\" width = 100 height = 100>", kDBName);
    fprintf(gamep, "</br></br>\n");
    fprintf(gamep, "</center>\n");
    
    // Game name, gamescrafter
    fprintf(gamep, "<center>");
    fprintf(gamep, "<h1><b>\n");
    fprintf(gamep, "%s\n", kGameName);
    fprintf(gamep, "</h1></b>\n");
    fprintf(gamep, "<h2><b>\n");
    fprintf(gamep, "Crafted by: %s", kAuthorName);
    fprintf(gamep, "</h2></b>\n");
    fprintf(gamep, "</center>");
    
    fprintf(gamep, "<!--#include virtual=\"%s_table.shtml\"-->\n", kDBName);
    
    fprintf(gamep, "</body>");
    fprintf(gamep, "</html>\n");
    
    fclose(gamep);
}


void createVarTable ()
{
    char tableFileName[256];
    FILE * tablep;
    int i;
    
    sprintf(tableFileName, "analysis/%s/%s_table.shtml", kDBName, kDBName);
    tablep = fopen(tableFileName, "w");
    
    fprintf(tablep,"<!-- AUTO CREATED, do not modify-->\n");
    fprintf(tablep,"<table align=""ABSCENTER"" BORDER =""1"" CELLSPACING=""0"" CELLPADDING=""5"">\n");
    fprintf(tablep,"<tr>\n");
    
    fprintf(tablep,"<td><b>Variant</b></td>\n");
    fprintf(tablep,"<td><b>Value</b></td>\n");
    fprintf(tablep,"<td><b>Wins</b></td>\n");
    fprintf(tablep,"<td><b>Loses</b></td>\n");
    fprintf(tablep,"<td><b>Ties</b></td>\n");
    fprintf(tablep,"<td><b>Primitive Wins</b></td>\n");
    fprintf(tablep,"<td><b>Primitive Loses</b></td>\n");
    fprintf(tablep,"<td><b>Primitive Ties</b></td>\n");
    fprintf(tablep,"<td><b>Reachable Positions</b></td>\n");
    fprintf(tablep,"<td><b>Total Positions</b></td>\n");
    fprintf(tablep,"<td><b>Hash Efficiency (%%)</b></td>\n");
    fprintf(tablep,"<td><b>Avg. Fanout</b></td>\n");
    fprintf(tablep,"<td><b>Timer(s)</b></td>\n");
    
    
    fprintf(tablep,"</tr>\n");
    
    for (i = 1; i <= NumberOfOptions(); i++) {
        fprintf(tablep,"<tr>\n");
        fprintf(tablep,"<!--#include virtual=\"var%d/row.shtml\"-->\n", i);
        fprintf(tablep,"</tr>\n");
    }
    
 fprintf(tablep,"</table>\n");
    fclose (tablep);
    
}

void writeVarHTML ()
{
    
    char text[256];
    FILE * rowp;
    char rowFileName[256];
    
    sprintf(rowFileName, "analysis/%s/var%d/row.shtml", kDBName,getOption());
    
    rowp = fopen(rowFileName, "w");
    
    /***********************************
    Variant Specific
    ************************************/
    
    
    fprintf(rowp,"<!-- AUTO CREATED, do not modify-->\n");
    
    sprintf(text, "%d",getOption());
    writeVarStat("option",text,rowp);
    
    writeVarStat("value", gValueString[(int)gValue], rowp);
    
    sprintf(text, "%5llu", gAnalysis.WinCount);
    writeVarStat("WinCount", text, rowp);
    
    sprintf(text, "%5llu", gAnalysis.LoseCount);
    writeVarStat("LoseCount", text, rowp);
    
    sprintf(text, "%5llu", gAnalysis.TieCount);
    writeVarStat("TieCount", text, rowp);
    
    sprintf(text, "%5llu", gAnalysis.PrimitiveWins);
    writeVarStat("Prim.WinCount", text, rowp);
    
    sprintf(text, "%5llu", gAnalysis.PrimitiveLoses);
    writeVarStat("Prim.LoseCount", text, rowp);
    
    sprintf(text, "%5llu", gAnalysis.PrimitiveTies);
    writeVarStat("Prim.TieCount", text, rowp);
    
    
    sprintf(text, "%5llu", gAnalysis.TotalPositions);
    writeVarStat("totalPositions", text , rowp);
    
    sprintf(text, "%5llu", gNumberOfPositions);
    writeVarStat("NumberOfPositions", text, rowp);
    
    sprintf(text, "%d", gAnalysis.HashEfficiency);
    writeVarStat("hashEfficiency", text, rowp);
    
    
    sprintf(text, "%2f", gAnalysis.AverageFanout);
    writeVarStat("AverageFanout", text, rowp);
    
    sprintf(text, "%d", gAnalysis.TimeToSolve);
    writeVarStat("TimeToSolve", text, rowp);
    
    
    
    fclose(rowp);
    
}


BOOLEAN CorruptedValuesP()
{
    MOVELIST *ptr, *head;
    VALUE parentValue, childValue;
    POSITION position, child;
    BOOLEAN parentIsWin, foundLosingChild, parentIsTie, foundTieingChild, corrupted;
    
    corrupted = FALSE;
    for(position=0 ; position<gNumberOfPositions ; position++) { /* for all positions */
        parentIsWin = FALSE;
        foundLosingChild = FALSE;
        parentIsTie = FALSE;
        foundTieingChild = FALSE;
        if((parentValue = GetValueOfPosition(position)) != undecided) { /* if valid position */
            if(Primitive(position) == undecided) { /* Not Primitive, children */
                head = ptr = GenerateMoves(position);
                while (ptr != NULL) {
                    child = DoMove(position,ptr->move);  /* Create the child */
                    childValue = GetValueOfPosition(child); /* Get its value */
		    
                    if (gGoAgain(position, ptr->move)) {
                        switch(childValue) {
                            case win: childValue = lose; break;
                            case lose: childValue = win; break;
                            default: break;
                        }
                    }
		    
                    if(parentValue == lose) {
                        if(childValue != win) {
                            corrupted = TRUE;
                            printf("Corruption: Losing Parent " POSITION_FORMAT " has %s child " POSITION_FORMAT ", shouldn't be losing\n",position,gValueString[childValue],child);
                        }
                    } else if (parentValue == win) {
                        parentIsWin = TRUE;
                        if(childValue == lose)
                            foundLosingChild = TRUE;
                    } else if (parentValue == tie) {
                        parentIsTie = TRUE;
                        if(childValue == lose) {
                            corrupted = TRUE;
                            printf("Corruption: Tieing Parent " POSITION_FORMAT " has Lose child " POSITION_FORMAT ", should be win\n",position,child);
                        } else if (childValue == tie)
                            foundTieingChild = TRUE;
                    } else 
                        BadElse("CorruptedValuesP");
                    ptr = ptr->next;                     /* Go to the next child */
                } /* while ptr != NULL (for all children) */
                FreeMoveList(head);
                if(parentIsWin && !foundLosingChild) {
                    corrupted = TRUE;
                    printf("Corruption: Winning Parent " POSITION_FORMAT " has no losing children, shouldn't be win\n",position);
                }
                if(parentIsTie && !foundTieingChild) {
                    corrupted = TRUE;
                    printf("Corruption: Tieing Parent " POSITION_FORMAT " has no tieing children, should be a lose\n",position);
                }
            } /* if not primitive */
        } /* if valid position */
    } /* for all positions */
    return(corrupted);
}

/*
** Analysis XML Output
*/

void writeXML(STATICMESSAGE msg)
{
    static FILE *xmlFile = 0;
    switch(msg)
    {
        case Init:
            xmlFile = prepareXMLFile();
            break;
        case Save:
            if(xmlFile != 0)
            {
                writeXMLData(xmlFile);
            }
            break;
        case Clean:
            if(xmlFile != 0)
            {
                closeXMLFile(xmlFile);
                xmlFile=0;
            }
            break;    
        default:
           /* BadElse? */
           break;
    }
}

FILE* prepareXMLFile()
{
  FILE * xmlFile;
  char xmlPath[256];
  
  sprintf(xmlPath, "analysis/xml/%s.xml", kDBName);
  
  mkdir("analysis/xml",0755);
  
  xmlFile = fopen(xmlPath,"w");
  fprintf(xmlFile,"<?xml version=\"1.0\"?>\n");
  fprintf(xmlFile,"<game name=\"%s\" author=\"%s\" shortname=\"%s\">\n", kGameName,kAuthorName,kDBName);
  return xmlFile;
}

void closeXMLFile(FILE* xmlFile)
{
    fprintf(xmlFile,"</game>\n");
    fclose(xmlFile);
}

void writeXMLData(FILE* xmlFile)
{
    fprintf(xmlFile,"    <variant hashcode=\"%d\">\n",getOption());
    fprintf(xmlFile,"        <value>%s</value>\n",gValueString[(int)gValue]);
    fprintf(xmlFile,"        <count>\n");
    fprintf(xmlFile,"            <win>%llu</win>\n",gAnalysis.WinCount);
    fprintf(xmlFile,"            <lose>%llu</lose>\n",gAnalysis.LoseCount);
    fprintf(xmlFile,"            <tie>%llu</tie>\n",gAnalysis.TieCount);
    fprintf(xmlFile,"        </count>\n");
    fprintf(xmlFile,"        <primitive>\n");
    fprintf(xmlFile,"            <win>%llu</win>\n",gAnalysis.PrimitiveWins);
    fprintf(xmlFile,"            <lose>%llu</lose>\n",gAnalysis.PrimitiveLoses);
    fprintf(xmlFile,"            <tie>%llu</tie>\n",gAnalysis.PrimitiveTies);
    fprintf(xmlFile,"        </primitive>\n");
    fprintf(xmlFile,"        <positionstats>\n");
    fprintf(xmlFile,"            <total>%llu</total>\n",gAnalysis.TotalPositions);
    fprintf(xmlFile,"            <hashtotal>%llu</hashtotal>\n",gNumberOfPositions);
    fprintf(xmlFile,"            <hashefficiency>%d</hashefficiency>\n",gAnalysis.HashEfficiency);
    fprintf(xmlFile,"            <fanout>%2f</fanout>\n",gAnalysis.AverageFanout);
    fprintf(xmlFile,"        </positionstats>\n");
    fprintf(xmlFile,"        <time>%d</time>\n",gAnalysis.TimeToSolve);
    fprintf(xmlFile,"    </variant>\n");
    fflush(xmlFile);
}

/*
** Percentage
*/

float PercentDone (STATICMESSAGE msg)
{
    static POSITION num_pos_seen = 0;
    float percent = 0;
    switch (msg)
    {
        case Update:
            num_pos_seen++;
            break;
        case Clean:
            num_pos_seen = 0;
            break;
        default:
            break;
    }
    percent = (float)num_pos_seen/(float)gNumberOfPositions * 100.0;

    return percent;
}


/************************************************************************
**
** NAME:        DatabaseCombVisualization
**
** DESCRIPTION: Print to stdout the Comb Visualization (described below)
**              which is essentially all the positions and holes of the DB,
**              encoded as positive and negative numbers respectively.
** 
** INPUTS:      none
**
************************************************************************/

void DatabaseCombVisualization()
{
  POSITION thePosition;
  BOOLEAN lastUndecided, thisUndecided;
  long streak = 0, longestUndecided = 0, longestDecided = 0, switches = 0;

  printf("\nThis is called a \"Database Comb Visualization\"\n");
  printf("because we go through the database and find the streaks of\n");
  printf("visited (i.e., known) positions and those the hash reserved\n");
  printf("space for but never used. Every known streak is represented as\n");
  printf("a POSITIVE number and every unknown streak is represented as\n");
  printf("a NEGATIVE number. Thus, a full database of size K will\n");
  printf("simply be printed as the positive number K. Likewise, a half-full database\n");
  printf("with every other position known and unknown will be a sequence of\n");
  printf("positive and negative ones: -1, 1, -1, 1, etc.\n");
  printf("---------------------------------------------------------------------------\n");

  lastUndecided = (GetValueOfPosition(0) == undecided); /* Handles 1st case */
  
  /* Can you say DAV? We should write an enumerator someday... */
  for(thePosition = 0; thePosition < gNumberOfPositions ; thePosition++) {

    thisUndecided = (GetValueOfPosition(thePosition) == undecided);
    if (lastUndecided == thisUndecided) {
      streak++;
      if ( thisUndecided && streak > longestUndecided) longestUndecided = streak;
      if (!thisUndecided && streak > longestDecided  ) longestDecided   = streak;
    }
    else {
      /* Streak of Undecideds prints as a negative # */
      /* Streak of Knowns     prints as a positive # */
      printf("%s%lu\n", (lastUndecided ? "-" : ""), streak);
      streak = 1; /* A new streak of 1 of a different parity */
      switches++;
    }
    lastUndecided = thisUndecided;
  }
  
  /* Must flush the last bookend one too */
  printf("%s%lu\n", (lastUndecided ? "-" : ""), streak);

  /* Print some stats */
  printf("\n\nLongest   Visited (positive #s) streak: %lu\n", longestDecided);
  printf("Longest UnVisited (negative #s) streak: %lu\n", longestUndecided);
  printf("Total switches we have (# of changes) : %lu\n", switches);

  HitAnyKeyToContinue();
}
