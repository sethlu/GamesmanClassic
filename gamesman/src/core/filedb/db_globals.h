/************************************************************************
**
** NAME:	db_globals.h
**
** DESCRIPTION:	Gamescrafters Database Globals Header File
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

#ifndef GMCORE_DB_GLOBALS_H
#define GMCORE_DB_GLOBALS_H

typedef unsigned long long Position;

typedef unsigned long long db_offset;

typedef unsigned long long frame_id;
typedef unsigned long long page_id;

typedef char boolean;
#define TRUE 1
#define FALSE 0

//this is really arbitrary
#define PAGE_SIZE 4088 // 4K bytes, minus 8 bytes for 64-bit page_id

typedef struct buffer_page_struct {
  char mem[PAGE_SIZE];
  page_id id; //buffer id, id*rec_size = the index of the first record in this page
  //frame_id rec_index; //the "index" (POSITION hash number) associated with the first record
  //db_offset off;
  boolean valid;
}db_buffer_page;

#endif /* GMCORE_DB_GLOBALS_H */