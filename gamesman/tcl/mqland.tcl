
####################################################
# this is a template for tcl module creation
#
# created by Alex Kozlowski and Peterson Trethewey
####################################################


# GS_InitGameSpecific sets characteristics of the game that
# are inherent to the game, unalterable.  You can use this fucntion
# to initialize data structures, but not to present any graphics.
# It is called FIRST, ONCE, only when the player
# starts playing your game, and before the player hits "New Game"
# At the very least, you must set the global variables kGameName
# and gInitialPosition in this function.

proc GS_InitGameSpecific {} {
    
    ### We added this
    global width height boardWidth boardHeight
    set boardWidth 500 
    set boardHeight 500
    set width 4
    set height 4

    global slotSize cellPadding megaCellPadding arrowWidth
    set slotSize(w) [expr $boardWidth / $width]
    set slotSize(h) [expr $boardHeight / $height]
    set cellPadding 10
    set megaCellPadding [expr 3 * $cellPadding]
    set arrowWidth [expr $slotSize(w) / 8]

    global xColor oColor
    set xColor blue
    set oColor red

    ### Set the name of the game
    
    global kGameName
    set kGameName "Queensland"
    
    ### Set the initial position of the board

    global gInitialPosition gPosition
    set gInitialPosition 0
    set gPosition $gInitialPosition

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString ""
    set kMisereString ""

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    global gMisereGame
    if {!$gMisereGame} {
	SetToWinString "To Win: (fill in)"
    } else {
	SetToWinString "To Win: (fill in)"
    }
    
    ### Edit this string

    SetToMoveString "To Move: (fill in)"
	    
    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Gamescrafters Team!"
    set kTclAuthors "(Fill this in)"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"
}

proc UnhashPosition {postition} {


}


proc orderArrows {c i j} {
    global width height 
    set maxtag [expr ($width-1)*($width-1)+($height-1)*($height-1)]
    for {set p $maxtag} {$p >= 1} {set p [expr $p - 1]} {
	$c raise "i $i j $j p $p"
    }
}

# GS_NameOfPieces should return a list of 2 strings that represent
# your names for the "pieces".  If your game is some pathalogical game
# with no concept of a "piece", give a name to the game's sides.
# if the game is tic tac toe, this might be a single line: return [list x o]
# This function is called FIRST, ONCE, only when the player
# starts playing the game, and before he hits "New Game"

proc GS_NameOfPieces {} {

    return [list x o]

}

# GS_ColorOfPlayers should return a list of two strings, 
# each representing the color of a player.
# If a specific color appears uniquely on one player's pieces,
# it might be a good choice for that player's color.
# In impartial games, both players may share the same color.
# If the game is tic tac toe, this might be the line 
# return [list blue red]
# If the game is nim, this might be the line
# return [list green green]
# This function is called FIRST, ONCE, only when the player
# starts playing the game, and before he clicks "New Game"
# The left player's color should be the first item in the list.
# The right player's color should be second.

proc GS_ColorOfPlayers {} {
    global xColor oColor
    set xColor blue
    set oColor red
    return [list $xColor $oColor]
    
}

# Setup the rules frame
# Adds widgets to the rules frame that will allow the user to 
# select the variant of this game to play. The options 
# selected by the user should be stored in a set of global
# variables.
# This procedure must initialize the global variables to some
# valid game variant.
# The rules frame must include a standard/misere setting.
# Args: rulesFrame (Frame) - The rules frame to which widgets
# should be added
# Modifies: the rules frame and its global variables
# Returns: nothing

proc GS_SetupRulesFrame { rulesFrame } {

    set standardRule \
	[list \
	     "What would you like your winning condition to be:" \
	     "Standard" \
	     "Misere" \
	    ]

    # List of all rules, in some order
    set ruleset [list $standardRule]

    # Declare and initialize rule globals
    global gMisereGame
    set gMisereGame 0

    # List of all rule globals, in same order as rule list
    set ruleSettingGlobalNames [list "gMisereGame"]

    global kLabelFont
    set ruleNum 0
    foreach rule $ruleset {
	frame $rulesFrame.rule$ruleNum -borderwidth 2 -relief raised
	pack $rulesFrame.rule$ruleNum  -fill both -expand 1
	message $rulesFrame.rule$ruleNum.label -text [lindex $rule 0] -font $kLabelFont
	pack $rulesFrame.rule$ruleNum.label -side left
	set rulePartNum 0
	foreach rulePart [lrange $rule 1 end] {
	    radiobutton $rulesFrame.rule$ruleNum.p$rulePartNum -text $rulePart -variable [lindex $ruleSettingGlobalNames $ruleNum] -value $rulePartNum -highlightthickness 0 -font $kLabelFont
	    pack $rulesFrame.rule$ruleNum.p$rulePartNum -side left -expand 1 -fill both
	    incr rulePartNum
	}
	incr ruleNum
    } 
}

# Get the game option specified by the rules frame
# Returns the option of the variant of the game specified by the 
# global variables used by the rules frame
# Args: none
# Modifies: nothing
# Returns: option (Integer) - the option of the game as specified by 
# getOption and setOption in the module's C code

proc GS_GetOption { } {
    global gMisereGame
    set option 1
    set option [expr $option + (1-$gMisereGame)]
    return $option
}

# Modify the rules frame to match the given options
# Modifies the global variables used by the rules frame to match the 
# given game option.
# This procedure only needs to support options that can be selected 
# using the rules frame.
# Args: option (Integer) -  the option of the game as specified by 
# getOption and setOption in the module's C code
# Modifies: the global variables used by the rules frame
# Returns: nothing

proc GS_SetOption { option } {
    global gMisereGame
    set option [expr $option - 1]
    set gMisereGame [expr 1-($option%2)]
}

# GS_Initialize is where you can start drawing graphics.  
# Its argument, c, is a canvas.  Please draw only in this canvas.
# You could put an opening animation in this function that introduces the game
# or just draw an empty board.
# This function is called ONCE after GS_InitGameSpecific, and before the
# player hits "New Game"

proc GS_Initialize { c } {

    #global constants
    global witdh height boardWidth boardHeight
    set boardWidth 500 
    set boardHeight 500
    set width 4
    set height 4

    global slotSize cellPadding megaCellPadding arrowWidth
    set slotSize(w) [expr $boardWidth / $width]
    set slotSize(h) [expr $boardHeight / $height]
    set cellPadding 10
    set megaCellPadding [expr 3 * $cellPadding]
    set arrowWidth [expr $slotSize(w) / 8]

    global xColor oColor
    set xColor blue
    set oColor red
  
    global xPieces oPeices
    global placeMoves
    global slideStartLocs arrows
    global background

    # you may want to start by setting the size of the canvas; this line isn't cecessary
    $c configure -width $boardWidth -height $boardHeight 

    #draw the pieces
    for {set i 0} {$i < $width} {incr i} {
	for {set j 0} { $j < $height} {incr j} {
	    set xPieces($i,$j) [$c create oval \
				    [expr $i * $slotSize(w) + $cellPadding] \
				    [expr ($j+1) * $slotSize(h) - $cellPadding] \
				    [expr ($i+1) * $slotSize(w) - $cellPadding] \
				    [expr $j * $slotSize(h) + $cellPadding] \
				    -fill $xColor -tags [list xPieces]]
	    set oPieces($i,$j) [$c create oval \
				    [expr $i * $slotSize(w) + $cellPadding] \
				    [expr ($j+1) * $slotSize(h) - $cellPadding] \
				    [expr ($i+1) * $slotSize(w) - $cellPadding] \
				    [expr $j * $slotSize(h) + $cellPadding] \
				    -fill $oColor -tags [list oPieces]]
	}
    }

    #draw small circles for place moves
    for {set i 0} {$i < $width} {incr i} {
	for {set j 0} {$j < $height} {incr j} {
	    set placeMoves($i,$j) [$c create oval \
				       [expr $i * $slotSize(w) + $megaCellPadding] \
				       [expr ($j+1) * $slotSize(h) - $megaCellPadding] \
				       [expr ($i+1) * $slotSize(w) - $megaCellPadding] \
				       [expr $j * $slotSize(h) + $megaCellPadding] \
				       -fill pink]
	}
    } 
   
    #draw small circles for slide start locations and 
    #arrows for slide direction on slide moves
    for {set i 0} {$i < $width} {incr i} {
	for {set j 0} {$j < $height} {incr j} {
	    set slideStartLocs($i,$j) [$c create oval \
				       [expr $i * $slotSize(w) + $megaCellPadding] \
				       [expr ($j+1) * $slotSize(h) - $megaCellPadding] \
				       [expr ($i+1) * $slotSize(w) - $megaCellPadding] \
				       [expr $j * $slotSize(h) + $megaCellPadding] \
				       -fill pink]
	}
    } 
    for {set i 0} {$i < $width} {incr i} {
	for {set j 0} {$j < $height} {incr j} {
	    for {set k 0} {$k < $width} {incr k} {
		for {set l 0} {$l < $height} {incr l} {
		    if {([expr $k - $i] == 0 && [expr $l - $j] != 0) || \
			    ([expr $k - $i] != 0 && [expr $l - $j] == 0) || \
			    ([expr abs ([expr $k - $i])] == [expr abs ([expr $l - $j])] && [expr $k - $i] != 0)} {
			set arrows($i,$j,$k,$l) [$c create line \
						     [expr $i * $slotSize(w) + $slotSize(w) / 2] \
						     [expr $j * $slotSize(h) + $slotSize(h) / 2] \
						     [expr $k * $slotSize(w) + $slotSize(w) / 2] \
						     [expr $l * $slotSize(h) + $slotSize(h) / 2] \
						     -width $arrowWidth -arrow last \
						     -arrowshape [list [expr 2 * $arrowWidth] [expr 2 * $arrowWidth] $arrowWidth] \
						     -fill green -tags [list "i $i j $j p [expr ($i-$k)*($i-$k) + ($j-$l)*($j-$l)]"]]
		    }
		}
	    }
	    orderArrows $c $i $j
	}
    }

    #draw the background board and lines
    set background [$c create rectangle 0 0 $boardWidth $boardHeight -fill gray]
    for {set i 1} {$i < $width} {incr i} {
	$c create line \
	    [expr $i * $slotSize(w)] 0 \
	    [expr $i * $slotSize(w)] $boardHeight \
	    -tags [list lines]
    }
    for {set j 1} {$j < $height} {incr j} {
	$c create line \
	    0 [expr $j * $slotSize(h)]  \
	    $boardWidth [expr $j * $slotSize(h)] \
	    -tags [list lines]
    }
    
    #raise the backround so that the pieces and moves aren't visible
    $c raise $background
    $c raise lines
}

proc GS_Deinitialize { c } {
    $c delete all
}

# GS_DrawPosition this draws the board in an arbitrary position.
# It's arguments are a canvas, c, where you should draw and the
# (hashed) position.  For example, if your game is a rearranger,
# here is where you would draw pieces on the board in their correct positions.
# Imagine that this function is called when the player
# loads a saved game, and you must quickly return the board to its saved
# state.  It probably shouldn't animate, but it can if you want.

# BY THE WAY: Before you go any further, I recommend writing a tcl function that 
# UNhashes You'll thank yourself later.
# Don't bother writing tcl that hashes, that's never necessary.

proc GS_DrawPosition { c position } {
    
    ### TODO: Fill this in
    
}


# GS_NewGame should start playing the game. "let's play"  :)
# It's arguments are a canvas, c, where you should draw and
# the hashed starting position of the game.
# This is called just when the player hits "New Game"
# and before any moves are made.

proc GS_NewGame { c position } {
    # TODO: The default behavior of this funciton is just to draw the position
    # but if you want you can add a special behaivior here like an animation
    GS_DrawPosition $c $position
}

# GS_WhoseMove takes a position and returns whose move it is.
# Your return value should be one of the items in the list returned
# by GS_NameOfPieces.
# This function is called just before every move.
# Optional Procedure

proc GS_WhoseMove { position } {
    return ""    
}

# GS_HandleMove draws (animates) a move being made.
# The user or the computer has just made a move, animate it or draw it
# or whatever.  Draw the piece moving if your game is a rearranger, or
# the piece appearing if it's a "dart board"

# By the way, if you animate, a function that will be useful for you is
# update idletasks.  You can call this to force the canvas to update if
# you make changes before tcl enters the event loop again.

proc GS_HandleMove { c oldPosition theMove newPosition } {

	### TODO: Fill this in
    
}

# GS_ShowMoves draws the move indicator (be it an arrow or a dot, whatever the
# player clicks to make the move)  It is also the function that handles coloring
# of the moves according to value. It is called by gamesman just before the player
# is prompted for a move.

# Arguments:
# c = the canvas to draw in as usual
# moveType = a string which is either value, moves or best according to which radio button is down
# position = the current hashed position
# moveList = a list of available moves to the player.  These moves are represented as numbers (same as in C)

# The code snippet herein may be helpful but is not necessary to do it that way.
# We provide a procedure called MoveTypeToColor that takes in moveType and
# returns the correct color.

proc GS_ShowMoves { c moveType position moveList } {

	### TODO: Fill this in
	
}

# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.

proc GS_HideMoves { c moveType position moveList} {

    ### TODO: Fill this in

}

# GS_HandleUndo draws a move undoing itself sortof.
# the names of the arguments explain themsleves but just to clarify...
# The game was in position A, a player makes move M bringing the game to position B
# then an undo is called
# currentPosition is the B
# theMoveToUndo is the M
# positionAfterUndo is the A

# By default this function just calls GS_DrawPosition, but you certainly don't need to keep that.

proc GS_HandleUndo { c currentPosition theMoveToUndo positionAfterUndo} {

	### TODO if needed
    GS_DrawPosition c positionAfterUndo
}

# GS_GetGameSpecificOptions is not quite ready, don't worry about it .
proc GS_GetGameSpecificOptions { } {
}

# GS_GameOver is called the moment the game is finished ( won, lost or tied)
# you could use this function to draw the line striking out the winning row in tic tac toe for instance
# or you could congratulate the winner or do nothing if you want.

proc GS_GameOver { c position gameValue nameOfWinningPiece nameOfWinner } {

	### TODO if needed
	
}

# GS_UndoGameOver is called then the player hits undo after the game is finished.
# this is provided so that you may undo the drawing you did in GS_GameOver if you drew something.
# for instance, if you drew a line crossing out the winning row in tic tac toe, this is where you sould delete the line.

# note: GS_HandleUndo is called regardless of whether the move undoes the end of the game, so IF you choose to do nothing in
# GS_GameOver, you needn't do anything here either.

proc GS_UndoGameOver { c position } {

	### TODO if needed

}
