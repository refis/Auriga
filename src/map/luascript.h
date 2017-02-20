/*
 * Copyright (C) 2002-2007  Auriga
 *
 * This file is part of Auriga.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef _LUASCRIPT_H_
#define _LUASCRIPT_H_

#include "lua.h"

lua_State *L;

extern int lua_respawn_id;
enum {
	L_RUN  = 0,
	L_STOP,
	L_INPUT,
	L_MENU,
	L_CLOSE,
	L_END
};

int luascript_run_function(const char *name,int char_id,const char *format,...);
void luascript_resume(struct map_session_data *sd,const char *format,...);

void luascript_reload(void);

int do_init_luascript(void);
int do_final_luascript(void);

#endif
