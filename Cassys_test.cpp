/*
 * Unitex
 *
 * Copyright (C) 2001-2012 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 *
 */
/*
 * Cassys_test.c
 *
 *  Created on: 4 oct. 2012
 *      Author: David Nott, Nathalie Friburger (nathalie.friburger@univ-tours.fr)
 */

#include "Cassys_test.h"
#include "Unicode.h"
#include "StringParsing.h"
#include "Pattern.h"

namespace unitex {

int main_Cassys_test(int argc, char* argv[]){

	const unichar P_OPENING_BRACKET[]={'{',0};

	unichar *text;
	char char_text[]="ola ! {Bonjour}";
	text = (unichar*) malloc(sizeof(unichar)*(strlen(char_text)+1));
	if(text == NULL){
		fatal_error("malloc");
	}

	u_strcpy(text,char_text);


	unichar result[128];
	parse_string(text,result,P_OPENING_BRACKET);


	u_printf("%S\n",result);





	return 0;
}

}
