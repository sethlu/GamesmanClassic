###################################################
# this is a template for tcl module creation
#
# created by Alex Kozlowski and Peterson Trethewey
####################################################


# GS_InitGameSpecific sets characteristics of the game that
# are inherent to the game, unalterable.  You can use this function
# to initialize data structures, but not to present any graphics.
# It is called FIRST, ONCE, only when the player
# starts playing your game, and before the player hits "New Game"
# At the very least, you must set the global variables kGameName
# and gInitialPosition in this function.

# set DIMENSION 4

### These depend on the constants on top

### FILL THIS IN
proc GS_InitGameSpecific  {} {

    # Authors Info
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Rach Liu, Bryon Ross, Jiong Shen, Tom Wang"
    set kTclAuthors "Rach Liu, Tom Wang"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"

    global CANVAS_WIDTH CANVAS_HEIGHT MIN_CANVAS_LENGTH NUM_EXTRA_GAPS

    # Copied from outside
    set CANVAS_WIDTH  500
    set CANVAS_HEIGHT 500

    if {$CANVAS_WIDTH > $CANVAS_HEIGHT} {
	set MIN_CANVAS_LENGTH $CANVAS_HEIGHT
    } else {
	set MIN_CANVAS_LENGTH $CANVAS_WIDTH
    }

    set NUM_EXTRA_GAPS 1

    global TIME_TO_ANIMATE_ONE_SLIDE NUM_FRAMES_PER_SEC NUM_FRAMES TIME_DELAY_BETWEEN_FRAMES

    # Useless
    set TIME_TO_ANIMATE_ONE_SLIDE 50
    set NUM_FRAMES_PER_SEC 10
    set NUM_FRAMES [expr $TIME_TO_ANIMATE_ONE_SLIDE * $NUM_FRAMES_PER_SEC]
    set TIME_DELAY_BETWEEN_FRAMES [expr 100 / $NUM_FRAMES_PER_SEC]

    # Overwrite NUM_FRAMES
    set NUM_FRAMES 30

    ### Set the name of the game
    
    global Dimension
    set Dimension 3    

    global kToMove kToWin
     
    set kToMove "Click on an arrow to place a piece."

    set kToWin "Connect $Dimension in a row in any direction to win." 

    global kGameName
    set kGameName "The game of Slide-N"
    
    ### Set the initial position of the board

    global gInitialPosition gPosition
    set gInitialPosition 0
    set gPosition $gInitialPosition

    global Board_Size
    set Board_Size [expr $Dimension * $Dimension]

    # For drawing the board
    global Gap Board_Length Piece_Radius
    set Gap [expr $MIN_CANVAS_LENGTH / 2 / ($Dimension + $NUM_EXTRA_GAPS)]
    set Board_Length [expr $MIN_CANVAS_LENGTH - 2 * $NUM_EXTRA_GAPS * $Gap]
    set Piece_Radius [expr $Gap / sqrt(2)]

    #global InitX
    #global InitY

    #global EndX
    #global EndY
}

# GS_NameOfPieces should return a list of 2 strings that represent
# your names for the "pieces".  If your game is some pathalogical game
# with no concept of a "piece", give a name to the game's sides.
# if the game is tic tac toe, this might be a single line: return [list x o]
# This function is called FIRST, ONCE, only when the player
# starts playing the game, and before he hits "New Game"
# The left player's piece should be the first item in the list.
# The right player's piece should be second.

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
    return [list red blue]
}

# GS_Initialize is where you can start drawing graphics.  
# Its argument, c, is a canvas.  Please draw only in this canvas.
# You could put an opening animation in this function that introduces the game
# or just draw an empty board.
# This function is called ONCE after GS_InitGameSpecific, and before the
# player hits "New Game"

proc GS_Initialize { c } {
    DrawBoard $c
} 

proc DrawBoard { c } {
    $c delete all

    global MIN_CANVAS_LENGTH
    global Dimension

    # Create giant backboard to hide things
    $c create rectangle 0 0 $MIN_CANVAS_LENGTH $MIN_CANVAS_LENGTH -fill honeydew -width 0 -tag BACK

    global Board_Length
    global Gap
    global NUM_EXTRA_GAPS

    # Draw lines from top left to bottom right
    for { set i 0 } { $i <= $Dimension } { incr i } {
	set x1 [expr $Board_Length / 2 - $Gap * $i]
	set y1 [expr $Gap * $i]

	set x2 [expr $x1 + $Board_Length / 2]
	set y2 [expr $y1 + $Board_Length / 2]

	$c create line $x1 $y1 $x2 $y2 -tag BOARD -width 5
    }
    
    # Draw lines from bottom right to top left
    for { set i 0 } { $i <= $Dimension } { incr i } {
	set x1 [expr $Board_Length / 2 + $Gap * $i]
	set y1 [expr $Gap * $i]

	set x2 [expr $x1 - $Board_Length / 2]
	set y2 [expr $y1 + $Board_Length / 2]

	$c create line $x1 $y1 $x2 $y2 -tag BOARD -width 5
    }

    $c raise BOARD BACK

    # Create all the pieces
    
    global Piece_Radius
    
    for { set i 0 } { $i < $Dimension } { incr i } {
 	set x_center [expr $Board_Length / 2 - $Gap * $i]
 	set y_center [expr $Gap * ($i + 1)] 
	
 	for { set j 0 } { $j < $Dimension } { incr j } {
 	    set x1 [expr $x_center - $Piece_Radius]
 	    set y1 [expr $y_center - $Piece_Radius]

 	    set x2 [expr $x_center + $Piece_Radius]
 	    set y2 [expr $y_center + $Piece_Radius]

 	    $c create oval $x1 $y1 $x2 $y2 -fill blue -tag [list PIECES BLUE_PIECES PIECE[expr $i * $Dimension + $j] BLUE[expr $i * $Dimension + $j]]
 	    $c create oval $x1 $y1 $x2 $y2 -fill red -tag [list PIECES RED_PIECES PIECE[expr $i * $Dimension + $j] RED[expr $i * $Dimension + $j]]

 	    set x_center [expr $x_center + $Gap]
 	    set y_center [expr $y_center + $Gap]
 	}
    }

    $c lower PIECES BACK

    # Create arrows

    set ARROW_XY_LEN [expr 1 * $Gap]
    set ARROW_GAP [expr 0.2 * $Gap]

    # Draw the arrows on the top right
    set x1 [expr $Board_Length / 2 - $Gap / 2 + $ARROW_GAP]
    set y1 [expr 0 - $Gap / 2 - $ARROW_GAP]

    for { set i 1 } { $i <= $Dimension } { incr i } {
	set x1 [expr $x1 + $Gap]
	set y1 [expr $y1 + $Gap]

	set x2 [expr $x1 + $ARROW_XY_LEN]
	set y2 [expr $y1 - $ARROW_XY_LEN]
	
	$c create line $x1 $y1 $x2 $y2 \
	    -arrow first -width [expr $Gap / 4] -fill black \
	    -arrowshape [list [expr $Gap / 4] [expr $Gap / 2] [expr $Gap / 4]] \
	    -tag [list ARROWS ARROW[expr $Dimension + $i]] \
	    -fill cyan
	
	$c bind ARROW[expr $Dimension + $i] <ButtonRelease-1> \
	    "myReturnFromHumanMove [expr $Dimension + $i]"
    }
        
    # Draw the arrows on the top left
    set x1 [expr $Board_Length / 2 + $Gap / 2 - $ARROW_GAP]
    set y1 [expr 0 - $Gap / 2 - $ARROW_GAP]

    for { set i 1 } { $i <= $Dimension } { incr i } {
	set x1 [expr $x1 - $Gap]
	set y1 [expr $y1 + $Gap]

	set x2 [expr $x1 - $ARROW_XY_LEN]
	set y2 [expr $y1 - $ARROW_XY_LEN]
	
	$c create line $x1 $y1 $x2 $y2 \
	    -arrow first -width [expr $Gap / 4] -fill black \
	    -arrowshape [list [expr $Gap / 4] [expr $Gap / 2] [expr $Gap / 4]] \
	    -tag [list ARROWS ARROW[expr $Dimension - $i + 1]] \
	    -fill cyan

	$c bind ARROW[expr $Dimension - $i + 1] <ButtonRelease-1> \
	    "myReturnFromHumanMove [expr $Dimension - $i + 1]" 
   }

    $c lower ARROWS all

    # Move everything to correct place
    global X_SHIFT Y_SHIFT
    set X_SHIFT [expr $NUM_EXTRA_GAPS * $Gap]
    set Y_SHIFT [expr 1.5 * $NUM_EXTRA_GAPS * $Gap]

    $c move BOARD $X_SHIFT $Y_SHIFT
    $c move PIECES $X_SHIFT $Y_SHIFT
    $c move ARROWS $X_SHIFT $Y_SHIFT
}

proc myReturnFromHumanMove { i } {
    # real code
    ReturnFromHumanMove $i

    # for debug
    # puts $i
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

proc Unhash { position } {
    global Board_Size

    set temp $position
    set retval []

    for { set i 0 } { $i < $Board_Size } { incr i } {
	lappend retval [expr $temp % 3]
	set temp [expr $temp / 3]
    }

    return $retval
}

proc GS_DrawPosition { c position } {
    
    # BTW too: don't make any assumptions about the state of the board.
    global CANVAS_WIDTH
    global CANVAS_HEIGHT

    set list_board [Unhash $position]

    DrawBoard $c

    # set temp_list $list_board

    set i 0

    #puts $list_board

    foreach item $list_board {

	# For debugging purposes
	# puts $item

	# Precondition: The board has been reset.

	if {$item == 0} { # This should not be necessary
	    $c lower BLUE$i all
	    $c lower RED$i all
	} elseif {$item == 1} {	    
	    $c raise RED$i BACK
	} else { #$item == 2
	    $c raise BLUE$i BACK
	}

	incr i
    }
}


# GS_NewGame should start playing the game. "let's play"  :)
# It's arguments are a canvas, c, where you should draw and
# the hashed starting position of the game.
# This is called just when the player hits "New Game"
# and before any moves are made.

proc GS_NewGame { c position } {
    # The default behavior of this function is just to draw the position
    # but if you want you can add a special behavior here like an animation
    GS_DrawPosition $c $position
}


# GS_WhoseMove takes a position and returns whose move it is.
# Your return value should be one of the items in the list returned
# by GS_NameOfPieces.
# This function is called just before every move.

proc GS_WhoseMove { position } {
    global Board_Size

    set temp $position

    for { set i 0 } { $i < $Board_Size } { incr i } {
	set temp [expr $temp / 3]
    }

    if {$temp == 0} {
	return x
    } elseif {$temp == 1} {
	return o
    } else {
	#puts "ERROR in GS_WhoseMove"
	return 0
    }
}


# GS_HandleMove draws (animates) a move being made.
# The user or the computer has just made a move, animate it or draw it
# or whatever.  Draw the piece moving if your game is a rearranger, or
# the piece appearing if it's a "dart board"

# By the way, if you animate, a function that will be useful for you is
# update idletasks.  You can call this to force the canvas to update if
# you make changes before tcl enters the event loop again.

proc GS_HandleMove { c oldPosition theMove newPosition } {
    global Gap Dimension
    global NUM_FRAMES TIME_DELAY_BETWEEN_FRAMES

#     puts "Enter HandleMove"

    GS_DrawPosition $c $oldPosition
    update idletasks
    
    set oldPositionUnhashed [Unhash $oldPosition]
    set newPositionUnhashed [Unhash $newPosition]

    set piecesSqueezed [Pieces_Squeezed $theMove $oldPositionUnhashed]

    DrawPieceBeforeMove $c $theMove $oldPosition
    update idletasks

    set squeezedPosUnhashed [Animate_Pieces_Squeezed $c $theMove $oldPositionUnhashed]

    # Put code for gravity here
    
    puts "Exit HandleMove"
    
#    after 1

    GS_DrawPosition $c $newPosition
}

proc Color_Piece { position } {
    global Board_Size

    set temp $position

    for { set i 0 } { $i < [expr $Board_Size] } { incr i } {
	set temp [expr $temp / 3]
    }

    if {$temp == 0} {
	return red
    } elseif {$temp == 1} {
	return blue
#    } else { # hopefully just testing
#	puts "Testing?"
#	return red
    }
}

# Draw the piece inserted from the move.
proc DrawPieceBeforeMove { c theMove position } {
    global Dimension Gap Board_Length X_SHIFT Y_SHIFT

    if {$theMove > $Dimension} {
	set i [expr $theMove - $Dimension - 1]
	set x_center [expr $Board_Length / 2 + $Gap * ($i + 1) + $X_SHIFT]
	set y_center [expr $Gap * $i + $Y_SHIFT]
    } else { # $theMove <= $Dimension
	# Check this against DrawBoard
	set i [expr $Dimension - $theMove]
	set x_center [expr $Board_Length / 2 - $Gap * ($i + 1) + $X_SHIFT]
	set y_center [expr $Gap * $i + $Y_SHIFT]
    }
    
    global Piece_Radius

    set x1 [expr $x_center - $Piece_Radius]
    set y1 [expr $y_center - $Piece_Radius]
    
    set x2 [expr $x_center + $Piece_Radius]
    set y2 [expr $y_center + $Piece_Radius]
    
    $c create oval $x1 $y1 $x2 $y2 -fill [Color_Piece $position] -tag NEW_PIECE_$theMove
    $c lower NEW_PIECE_$theMove BOARD
}

# Check if some piece will be squeezed by the new piece
proc Pieces_Squeezed { theMove oldPositionUnhashed } {
    global Dimension

    if { $theMove <= $Dimension } {
	set piece_to_check [expr ($Dimension - $theMove) * $Dimension]
    } else { # $theMove > $Dimension
	set piece_to_check [expr $theMove - $Dimension - 1]
    }

    if {[lindex $oldPositionUnhashed $piece_to_check] != 0} {
	return true
    } else {
	return false
    }
}

proc Animate_Pieces_Squeezed { c theMove oldPositionUnhashed } {
    global Dimension

    # Move every piece along the direction one square forward
    if { $theMove > $Dimension } {
	set myMove [expr $theMove - $Dimension - 1]
	return [Animate_Pieces_Squeezed_Helper $c $theMove $myMove $oldPositionUnhashed [expr $Dimension * $Dimension] $Dimension]
    } else { # $theMove <= $Dimension
	set myMove [expr $Dimension * ($Dimension - $theMove)]
	return [Animate_Pieces_Squeezed_Helper $c $theMove $myMove $oldPositionUnhashed [expr ($myMove / $Dimension + 1) * $Dimension] 1]
    }
}

proc Last { list } {
    return [lindex list [expr [llength $list] - 1]]
}

# Append all pieces that need to move
proc AppendPiecesToMove { c theMove myMove oldPositionUnhashed bound increment } {
    set l [list NEW_PIECE_$theMove]

    for { set i $myMove } { $i < $bound } { incr i $increment } {
	if { [lindex $oldPositionUnhashed $i] != 0 } {
	    lappend l PIECE$i
	} else { # No piece at the location i
	    break
	}
    }

    return $l
}

proc Animate_Pieces_Squeezed_Helper { c theMove myMove oldPositionUnhashed bound increment } {
    set l [AppendPiecesToMove $c $theMove $myMove $oldPositionUnhashed $bound $increment]

    global NUM_FRAMES TIME_DELAY_BETWEEN_FRAMES Gap Dimension

    set MOVE_Y_PER_FRAME [expr 1.0 * $Gap / $NUM_FRAMES]

    puts $MOVE_Y_PER_FRAME

    if {$theMove > $Dimension} {
	set MOVE_X_PER_FRAME [expr -1 * $MOVE_Y_PER_FRAME]
    } else { # $theMove <= $Dimension
	set MOVE_X_PER_FRAME $MOVE_Y_PER_FRAME
    }

    for { set i 0 } { $i < $NUM_FRAMES } { incr i } {
#	after $TIME_DELAY_BETWEEN_FRAMES
	# puts "Outside loop"

	foreach item $l {	
	    $c move $item $MOVE_X_PER_FRAME $MOVE_Y_PER_FRAME
	    # puts $item
	}

	update idletasks
    }

    # Fix this...
    return 0 
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
# We provide a procedure called MoveValueToColor that takes in moveType and a moveValue and returns the correct color.
# For example [MoveValueToColor "value" "Tie"] would return yellow and [MoveValueToColor "all" "anything"] would return cyan.

proc GS_ShowMoves { c moveType position moveList } {
    # Seems like we have to call this ourselves

    #Debug purpose
    global i

    set i 0

    #puts $moveList

    foreach item $moveList {
	set i [lindex $item 0]
	set value [lindex $item 1]
	set color cyan

	if {$moveType == "value"} {

	    #puts $i
	    #puts $value

	    if {$value == "Tie"} {
		set color yellow
	    } elseif {$value == "Lose"} {
		set color green
	    } else {
		set color DarkRed
	    }
	}

	$c itemconfig ARROW$i -fill $color
    }

    $c raise ARROWS all
}



# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.

proc GS_HideMoves { c moveType position moveList} {
    $c lower ARROWS all
}



# GS_HandleUndo draws a move undoing itself sort of.
# the names of the arguments explain themselves but just to clarify...
# The game was in position A, a player makes move M bringing the game to position B
# then an undo is called
# currentPosition is the B
# theMoveToUndo is the M
# positionAfterUndo is the A

# By default this function just calls GS_DrawPosition, but you certainly don't need to keep that.

proc GS_HandleUndo { c currentPosition theMoveToUndo positionAfterUndo} {
    GS_DrawPosition $c $positionAfterUndo
}





# GS_GetGameSpecificOptions is not quite ready, don't worry about it .
proc GS_GetGameSpecificOptions { } {
}




# GS_GameOver is called the moment the game is finished ( won, lost or tied)
# you could use this function to draw the line striking out the winning row in tic tac toe for instance
# or you could congratulate the winner or do nothing if you want.

proc GS_GameOver { c position gameValue nameOfWinningPiece nameOfWinner lastMove } {
    GS_DrawPosition $c $position
    puts $nameOfWinner
}


# GS_UndoGameOver is called then the player hits undo after the game is finished.
# this is provided so that you may undo the drawing you did in GS_GameOver if you drew something.
# for instance, if you drew a line crossing out the winning row in tic tac toe, this is where you should delete the line.

# note: GS_HandleUndo is called regardless of whether the move undoes the end of the game, so IF you choose to do nothing in
# GS_GameOver, you needn't do anything here either.

proc GS_UndoGameOver { c position } {
}
