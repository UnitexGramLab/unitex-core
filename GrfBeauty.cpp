/*
 * Unitex
 *
 * Copyright (C) 2001-2011 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "GrfBeauty.h"
#include "Error.h"


/**
 * This function takes a grf generated from a sequence list, and tries
 * to make it more beautiful for a human reader.
 */
void beautify(Grf* grf,Alphabet* alph) {
/* 1: fusion de boîtes équivalentes à la casse près
 * 2: fusion de boîtes pseudo-équivalentes à la casse près (mêmes transitions
 *    entrantes ou sortantes: à paramétrer par une option)
 * 3: fusion de deux boîtes A et B telles que A ne pointe que B et B n'est pointée que par A
 * 4: fusion de deux boîtes ayant les mêmes transitions
 * 5: si A et C sont reliées soit directement, soit par B, modifier B pour ajouter une ligne <E>
 * 6: si A, B et C sont toutes reliées à X, Y et Z, utiliser une boîte <E> intermédiaire pour
 *    réduire le nombre de transitions
 *
 * en dernier: positionner joliment les boîtes
 *  => il faudra détecter les plus grands sous-groupes
 */
}
