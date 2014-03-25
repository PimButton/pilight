/*
	Copyright (C) 2013 CurlyMo & Bram1337

	This file is part of pilight.

    pilight is free software: you can redistribute it and/or modify it under the
	terms of the GNU General Public License as published by the Free Software
	Foundation, either version 3 of the License, or (at your option) any later
	version.

    pilight is distributed in the hope that it will be useful, but WITHOUT ANY
	WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with pilight. If not, see	<http://www.gnu.org/licenses/>
*/

#ifndef _PROTOCOL_PIRMOTION_H_
#define _PROTOCOL_PIRMOTION_H_

struct protocol_t *pirmotion;

void pirmotionInit(void);
void pirmotionCreateMessage(int systemcode, int programcode, int state);
void pirmotionParseCode(void);
int pirmotionCreateCode(JsonNode *code);
void pirmotionCreateLow(int s, int e);
void pirmotionCreateMed(int s, int e);
void pirmotionCreateHigh(int s, int e);
void pirmotionClearCode(void);
void pirmotionCreateSystemCode(int systemcode);
void pirmotionCreateProgramCode(int programcode);
void pirmotionCreateState(int state);
void pirmotionCreateFooter(void);
void pirmotionPrintHelp(void);

#endif
