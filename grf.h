 /*
  * Unitex
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef _GRF_H_
#define _GRF_H_

#include <stdio.h>
#include "Unicode.h"

#define GRF_BOX_INIT   0
#define GRF_BOX_FINAL  1



struct grf_box;

typedef struct grf_trans {
  int to;
  grf_trans * next;
} grf_trans;


typedef struct grf_box {
  unichar * label;
  int x, y;
  int terminal;
  grf_trans * trans;
} grf_box;


typedef struct grf_header {
  unichar ** header;
  int nb;
  int size;
} grf_header;



typedef struct grf_t {
  grf_header * headers;
  int nb_boxes;
  grf_box ** boxes;
} grf_t;




grf_t * grf_load(FILE * f);
grf_t * grf_load(char * filename);

grf_t * new_grf(int nb_boxes);
void grf_delete(grf_t * grf);

//void grf_add_trans(grf_t * grf, int from, int to);

//void grf_dump(grf_t * grf, FILE * out);


int grf_is_acyclic(grf_t * grf);

void grf_remove_epsilons(grf_t * grf);
void grf_cleanup(grf_t * grf);

int grf_check_labels(grf_t * grf);


void grf_reorder_by_tab(grf_t * grf, int * tab);

void grf_calc_ids(grf_t * grf, int * ids);

#endif
