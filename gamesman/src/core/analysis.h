#ifndef GMCORE_DB_H
#include "db.h"
#endif

#ifndef GMCORE_ANALYSIS_H
#define GMCORE_ANALYSIS_H
/*
#ifndef ORIGINAL_PUT_VALUE_PTR
VALUE   (*original_put_value) (POSITION pos, VALUE val);
#define ORIGINAL_PUT_VALUE_PTR
#endif
*/


/* Functions to output sets of data */

void	PrintRawGameValues		(BOOLEAN toFile);
void	PrintBadPositions		(char value, int maxPos, POSITIONLIST* badWinPos,
					 POSITIONLIST* badTiePos, POSITIONLIST* badLosePos);
void	PrintMexValues			(MEX value, int maxpos);
void	PrintValuePositions		(char value, int maxPos);
void	PrintGameValueSummary		();
void    PrintDetailedGameValueSummary();

/* Analysis output */

void	analyze				();
VALUE   AnalyzePosition(POSITION thePosition, VALUE value);
void    AnalysisCollation();
float   DetermineProbability    (POSITION position, VALUE value);
void	writeVarStat			(STRING statName, STRING text, FILE* out);
void	DatabaseCombVisualization	();

/* Analysis save/load */
BOOLEAN LoadAnalysis();
void    SaveAnalysis();

/* Analysis misc */

void	createAnalysisGameDir		();
void	createAnalysisVarDir		();
void	writeGameHTML			();
void	createVarTable			();
void	writeVarHTML			();
BOOLEAN	CorruptedValuesP		();

float	PercentDone			(STATICMESSAGE msg);

/* Analysis XML Support */

void	writeXML			(STATICMESSAGE msg);
void	prepareXMLFile			();
void 	prepareXMLVarFile		();
void	closeXMLFile			();
void	closeXMLVarFile			();
void	writeXMLData			();
void	writeXMLVarData			();

/* Analysis Data Structure */

typedef struct analysis_info
{
  POSITION NumberOfPositions;
  POSITION TotalPositions;
  POSITION TotalMoves;
  POSITION TotalPrimitives;
  POSITION WinCount;
  POSITION LoseCount;
  POSITION TieCount;
  POSITION UnknownCount;
  POSITION Draws;
  POSITION PrimitiveWins;
  POSITION PrimitiveLoses;
  POSITION PrimitiveTies;
  POSITION F0EdgeCount;
  POSITION F0NodeCount;
  POSITION F0DrawEdgeCount;
  VALUE InitialPositionValue;
  REMOTENESS LargestFoundRemoteness;
  unsigned int  TimeToSolve;
  int HashEfficiency;
  float AverageFanout;
  float InitialPositionProbability;
  POSITION DetailedPositionSummary[REMOTENESS_MAX+1][4];		/* Table for counting wins(1) and losses(2) and ties(3) 
															*at each remoteness between 0 and REMOTENESS_MAX-1   */
} ANALYSIS;

extern ANALYSIS gAnalysis;

#endif /* GMCORE_ANALYSIS_H */

