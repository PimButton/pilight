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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../pilight.h"
#include "common.h"
#include "log.h"
#include "protocol.h"
#include "hardware.h"
#include "binary.h"
#include "gc.h"
#include "pirmotion.h"

void pirmotionCreateMessage(int systemcode, int programcode, int state) {
	pirmotion->message = json_mkobject();
	json_append_member(pirmotion->message, "systemcode", json_mknumber(systemcode));
	json_append_member(pirmotion->message, "programcode", json_mknumber(programcode));
	if(state == 1) {
		json_append_member(pirmotion->message, "state", json_mkstring("on"));
	} else {
		json_append_member(pirmotion->message, "state", json_mkstring("off"));
	}
}

void pirmotionParseCode(void) { // This function parses received code. We need
								// to edit this part of the code
	int x = 0;

	/* Convert the one's and zero's into binary */
	for(x=0; x<pirmotion->rawlen; x+=4) {
		if(pirmotion->code[x+3] == 1 || pirmotion->code[x+0] == 1) {
			pirmotion->binary[x/4]=1;
		} else {
			pirmotion->binary[x/4]=0;
		}
	}

	int systemcode = binToDec(pirmotion->binary, 0, 4);
	int programcode = binToDec(pirmotion->binary, 5, 9);
	int check = pirmotion->binary[10];
	int state = pirmotion->binary[11];

	if(check != state) {
		pirmotionCreateMessage(systemcode, programcode, state);
	}
}

void pirmotionCreateLow(int s, int e) {
	int i;

	for(i=s;i<=e;i+=4) {
		pirmotion->raw[i]=pirmotion->plslen->length;
		pirmotion->raw[i+1]=(pirmotion->pulse*pirmotion->plslen->length);
		pirmotion->raw[i+2]=(pirmotion->pulse*pirmotion->plslen->length);
		pirmotion->raw[i+3]=pirmotion->plslen->length;
	}
}

void pirmotionCreateMed(int s, int e) {
	int i;

	for(i=s;i<=e;i+=4) {
		pirmotion->raw[i]=(pirmotion->pulse*pirmotion->plslen->length);
		pirmotion->raw[i+1]=pirmotion->plslen->length;
		pirmotion->raw[i+2]=(pirmotion->pulse*pirmotion->plslen->length);
		pirmotion->raw[i+3]=pirmotion->plslen->length;
	}
}

void pirmotionCreateHigh(int s, int e) {
	int i;

	for(i=s;i<=e;i+=4) {
		pirmotion->raw[i]=pirmotion->plslen->length;
		pirmotion->raw[i+1]=(pirmotion->pulse*pirmotion->plslen->length);
		pirmotion->raw[i+2]=pirmotion->plslen->length;
		pirmotion->raw[i+3]=(pirmotion->pulse*pirmotion->plslen->length);
	}
}

void pirmotionClearCode(void) {
	pirmotionCreateLow(0,47);
}

void pirmotionCreateSystemCode(int systemcode) {
	int binary[255];
	int length = 0;
	int i=0, x=0;

	length = decToBinRev(systemcode, binary);
	for(i=0;i<=length;i++) {
		if(binary[i]==1) {
			x=i*4;
			pirmotionCreateMed(x, x+3);
		}
	}
}

void pirmotionCreateProgramCode(int programcode) {
	int binary[255];
	int length = 0;
	int i=0, x=0;

	length = decToBinRev(programcode, binary);
	for(i=0;i<=length;i++) {
		if(binary[i]==1) {
			x=i*4;
			pirmotionCreateHigh(20+x, 20+x+3);
		}
	}
}

void pirmotionCreateState(int state) {
	if(state == 0) {
		pirmotionCreateHigh(40, 43);
	} else {
		pirmotionCreateHigh(44, 47);
	}
}

void pirmotionCreateFooter(void) {
	pirmotion->raw[48]=(pirmotion->plslen->length);
	pirmotion->raw[49]=(PULSE_DIV*pirmotion->plslen->length);
}

int pirmotionCreateCode(JsonNode *code) {
	int systemcode = -1;
	int programcode = -1;
	int state = -1;
	int tmp;

	json_find_number(code, "systemcode", &systemcode);
	json_find_number(code, "programcode", &programcode);
	if(json_find_number(code, "off", &tmp) == 0)
		state=0;
	else if(json_find_number(code, "on", &tmp) == 0)
		state=1;

	if(systemcode == -1 || programcode == -1 || state == -1) {
		logprintf(LOG_ERR, "pirmotion: insufficient number of arguments");
		return EXIT_FAILURE;
	} else if(systemcode > 31 || systemcode < 0) {
		logprintf(LOG_ERR, "pirmotion: invalid systemcode range");
		return EXIT_FAILURE;
	} else if(programcode > 31 || programcode < 0) {
		logprintf(LOG_ERR, "pirmotion: invalid programcode range");
		return EXIT_FAILURE;
	} else {
		pirmotionCreateMessage(systemcode, programcode, state);
		pirmotionClearCode();
		pirmotionCreateSystemCode(systemcode);
		pirmotionCreateProgramCode(programcode);
		pirmotionCreateState(state);
		pirmotionCreateFooter();
	}
	return EXIT_SUCCESS;
}

void pirmotionPrintHelp(void) {
	printf("\t -s --systemcode=systemcode\tcontrol a device with this systemcode\n");
	printf("\t -u --programcode=programcode\tcontrol a device with this programcode\n");
	printf("\t -t --on\t\t\tsend an on signal\n");
	printf("\t -f --off\t\t\tsend an off signal\n");
}

void pirmotionInit(void) {

	protocol_register(&pirmotion);
	protocol_set_id(pirmotion, "pirmotion");
	protocol_device_add(pirmotion, "pirmotion", "pirmotion Switches");
	protocol_plslen_add(pirmotion, 494); // 16796 / 3 = 494
	pirmotion->devtype = SWITCH;
	pirmotion->hwtype = RF433;
	pirmotion->pulse = 4;
	pirmotion->rawlen = 50;
	pirmotion->binlen = 12;

	options_add(&pirmotion->options, 's', "systemcode", OPTION_HAS_VALUE, CONFIG_ID, JSON_NUMBER, NULL, "^(3[012]?|[012][0-9]|[0-9]{1})$");
	options_add(&pirmotion->options, 'u', "programcode", OPTION_HAS_VALUE, CONFIG_ID, JSON_NUMBER, NULL, "^(3[012]?|[012][0-9]|[0-9]{1})$");
	options_add(&pirmotion->options, 't', "on", OPTION_NO_VALUE, CONFIG_STATE, JSON_STRING, NULL, NULL);
	options_add(&pirmotion->options, 'f', "off", OPTION_NO_VALUE, CONFIG_STATE, JSON_STRING, NULL, NULL);

	options_add(&pirmotion->options, 0, "gui-readonly", OPTION_HAS_VALUE, CONFIG_SETTING, JSON_NUMBER, (void *)0, "^[10]{1}$");

	pirmotion->parseCode=&pirmotionParseCode;
	pirmotion->createCode=&pirmotionCreateCode;
	pirmotion->printHelp=&pirmotionPrintHelp;
}
