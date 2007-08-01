 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Universit� de Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#ifndef FrequencyH
#define FrequencyH

#include "Text_tokens.h"
#include <Judy.h>

typedef Pvoid_t judy;

/**
 * This structure is used to store information about the current
 * freq build. It is used to avoid giving too much parameters
 * to functions.
 */
struct freq_opt {
	int automata;
	int words_only;
	int token_limit;
	unsigned threshold;
	int sentence_only;
	int clength;
};

int print_freqtable(struct snt_files *snt, struct freq_opt);
judy create_freqtable( FILE *text,              
                       FILE *ind,
                       FILE *fst2,            
                       struct text_tokens *tok, 
                       struct freq_opt option   );


#endif

