 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <stdlib.h>

#include "qsort.h"
#include "stack.h"
#include "grf.h"
#include "Error.h"

#define MAX(x,y)  ((x > y) ? x : y)

#define UNDEF (-1)

#define WHITE  (0)
#define GRAY   (1)
#define BLACK  (2)



static unichar epsilon[] = { '<', 'E', '>', 0 };



grf_header * grf_header_new() {

  grf_header * header = (grf_header *) malloc(sizeof(grf_header));

  header->size = 32;
  header->header = (unichar **) malloc(header->size * sizeof(unichar *));
  header->nb = 0;

  return header;
}


void grf_header_delete(grf_header * header) {
  for (int i = 0; i < header->nb; i++) {
    free(header->header[i]);
  }
  free(header->header);
  free(header);
}


void grf_header_add(grf_header * header, unichar * str) {

  if (header->nb == header->size) {
    header->size = header->size * 2;
    header->header = (unichar **) realloc(header->header, header->size * sizeof(unichar *));
  }

  header->header[header->nb++] = u_strdup(str);
}



grf_box * grf_box_new() {

  grf_box * box = (grf_box *) malloc(sizeof(grf_box));

  box->label = NULL;
  box->x = box->y = box->terminal = 0;
  box->trans = NULL;

  return box;
}



void grf_box_delete(grf_box * box) {

  free(box->label);

  grf_trans * trans;
  while ((trans = box->trans) != NULL) { box->trans = trans->next; free(trans); }

  free(box);
}



grf_t * grf_new(int nb_boxes) {

  grf_t * grf = (grf_t *) malloc(sizeof(grf_t));

  grf->nb_boxes = nb_boxes;
  grf->boxes = (grf_box **) malloc(nb_boxes * sizeof(grf_box *));

  for (int i = 0; i < nb_boxes; i++) {
    grf->boxes[i] = NULL;
  }

  return grf;
}



void grf_delete(grf_t * grf) {

  for (int i = 0; i < grf->nb_boxes; i++) { if (grf->boxes[i]) { grf_box_delete(grf->boxes[i]); } }

  if (grf->headers) { grf_header_delete(grf->headers); }
  free(grf->boxes);
  free(grf);
}



void grf_add_trans(grf_t * grf, int from, int to) {

  grf_trans * trans;

  if ((grf->boxes[from]->trans == NULL) || (to < grf->boxes[from]->trans->to)) {
    trans = (grf_trans *) malloc(sizeof(grf_trans));
    trans->to   = to;
    trans->next = grf->boxes[from]->trans;
    grf->boxes[from]->trans = trans;
    return;
  }

  if (to == grf->boxes[from]->trans->to) { return; }

  for (trans = grf->boxes[from]->trans; trans->next && (trans->next->to < to); trans = trans->next);

  if ((trans->next != NULL) && (to == trans->next->to)) { return; }

  grf_trans * nouvo = (grf_trans *) malloc(sizeof(grf_trans));
  nouvo->to   = to;
  nouvo->next = trans->next;
  trans->next = nouvo;
}




grf_t * grf_load(FILE * f) {


  int bufsize = 1024;
  unichar * buf = (unichar *) malloc(bufsize * sizeof(unichar));


  grf_header * header = grf_header_new();

  u_fgets(buf, bufsize, f);

  do { // look for '#'

    grf_header_add(header, buf);

    if (u_fgets(buf, bufsize, f) == EOF) { error("unexpected EOF"); free(buf); return NULL; }

  } while (*buf != '#');



  /* read num of boxes */

  u_fgets(buf, bufsize, f);

  if (! u_is_digit(*buf)) {
    error("parse error\n");
    free(buf);
    return NULL;
  }

  int nb_boxes = u_parse_int(buf, NULL);

  grf_t * grf = grf_new(nb_boxes);


  grf->headers = header;


  /* read box decriptions */

  for (int i = 0; i < nb_boxes; i++) {

    grf_box * box = grf->boxes[i] = grf_box_new();

    int len;
    unichar * p = buf;

    if ((len = u_fgets(p, bufsize, f)) == EOF) { error("unexpected EOF\n"); goto error; }

    while ((len == (bufsize - 1)) && (buf[len - 1] != '\n')) {  /* make sure we read all the line */

      bufsize = 2 * bufsize;
      buf = (unichar *) realloc(buf, bufsize * sizeof(unichar));
      p = buf + len;
      len = len + u_fgets(p, bufsize - len, f);
    }



    /* read label */

    if (*buf != '"') { error("character '\"' expected\n"); goto error; }

    for (p = buf + 1; *p != '"'; p++) {
      if (*p == '\\') { p++; }
      if (*p == 0) { error("error: label doesn't end?\n"); goto error; }
    }

    *p = 0;

    box->label = u_strdup(buf + 1);

    /* read coords of the box */

    p++;
    
    while (*p && (! u_is_digit(*p))) { p++; }
    box->x = u_parse_int(p, &p);

    while (*p && (! u_is_digit(*p))) { p++; }
    box->y = u_parse_int(p, &p);

    while (*p && (! u_is_digit(*p))) { p++; }
    if (*p == 0) {
      error("unexpected EOL (box #%d)\n", i);
      goto error;
    }
    

    int nb_trans = u_parse_int(p, &p);

    for (int j = 0; j < nb_trans; j++) {

      while (*p && (! u_is_digit(*p))) { p++; }
      if (*p == 0) {
	error("unexpected EOL (box #%d)\n", i);
	goto error;
      }

      int to = u_parse_int(p, &p);
      grf_add_trans(grf, i, to);

      if (to == GRF_BOX_FINAL) { box->terminal = 1; }

    }
  }

  free(buf);

  return grf;

error:

  free(buf);
  grf_delete(grf);
  return NULL;
}




grf_t * grf_load(char * fname) {

  FILE * f;

  if ((f = u_fopen(fname, U_READ)) == NULL) {
    error("cannot open %s\n", fname);
    return NULL;
  }

  grf_t * grf = grf_load(f);

  fclose(f);

  return grf;
}



void grf_reorder(grf_t * grf, const int * newnum) {

  grf_box ** oldboxes = grf->boxes;

  grf->boxes = (grf_box **) malloc(grf->nb_boxes * sizeof(grf_box *));


  int nb = 0;

  for (int i = 0; i < grf->nb_boxes; i++) {

    if (newnum[i] != UNDEF) {

      nb = MAX(nb, newnum[i] + 1);

      grf_box * box = grf->boxes[newnum[i]] = oldboxes[i];

      grf_trans * trans = box->trans;
      box->trans = NULL;

      while (trans) {

	if (newnum[trans->to] != UNDEF) { grf_add_trans(grf, newnum[i], newnum[trans->to]); }

	grf_trans * tmp = trans;
	trans = trans->next;
	free(tmp);
      }

    } else { grf_box_delete(grf->boxes[i]); }
  }

  if (nb != grf->nb_boxes) {
    grf->boxes = (grf_box **) realloc(grf->boxes, nb * sizeof(grf_box *));
    grf->nb_boxes = nb;
  }

  free(oldboxes);
}



/* remake grf according to tab ...
 * if tab[i] == 0, delete box[i] and all transition pointing from/to it
 * else keep the box
 */



void grf_remake(grf_t * grf, char * tab) {

  int nb = 0;

  /* compute new ordering */

  int* newnum=(int*)malloc(sizeof(int)*grf->nb_boxes);

  for (int i = 0; i < grf->nb_boxes; i++) {
    if (tab[i]) {
      newnum[i] = nb++;
    } else {
      newnum[i] = -1;
    }
  }


  grf_box ** boxes = (grf_box **) malloc(nb * sizeof(grf_box *));

  for (int i = 0; i < grf->nb_boxes; i++) {

    if (newnum[i] != -1) {

      grf_box * box = boxes[newnum[i]] = grf->boxes[i];

      /* delete transitions pointing to a former box and renumber the others */

      grf_trans * tmp;

      while ((box->trans != NULL) && (newnum[box->trans->to] == -1)) {
	tmp        = box->trans;
	box->trans = box->trans->next;
	free(tmp);
      }


      for (grf_trans * trans = box->trans; trans; trans = trans->next) {

	trans->to = newnum[trans->to];

	while ((trans->next != NULL) && (newnum[trans->next->to] == -1)) {
	  tmp = trans->next;
	  trans->next = trans->next->next;
	  free(tmp);
	}
      }

    } else { grf_box_delete(grf->boxes[i]); }
  }

  free(grf->boxes);
  grf->boxes = boxes;
  grf->nb_boxes = nb;
free(newnum);
}



/*
 * return true if terminal box is accessible from box i
 */

//TODO: use stack

static int is_useful(grf_t * grf, char * color, char * useful, int i) {

  if (color[i] == BLACK) { return useful[i]; }
  if (color[i] == GRAY)  { error("error: is_useful called on a cyclic graph.\n"); return useful[i]; }

  color[i] = GRAY;

  if (i == GRF_BOX_FINAL) { useful[i] = 1; }

  for (grf_trans * trans = grf->boxes[i]->trans; trans; trans = trans->next) {

    if (is_useful(grf, color, useful, trans->to)) {
      useful[i] = 1;
    }
  }

  color[i] = BLACK;

  return useful[i];
}




/*
 * emonde (prune?) grf
 */

void grf_cleanup(grf_t * grf) {

  char* color=(char*)malloc(sizeof(char)*grf->nb_boxes);
  char* control=(char*)malloc(sizeof(char)*grf->nb_boxes);

  for (int i = 0; i < grf->nb_boxes; i++) { color[i] = WHITE; control[i] = 0; }

  is_useful(grf, color, control, GRF_BOX_INIT); // color[] give us accessible box and control[] the useful one


  /* make control[] give useful AND accessible box */

  for (int i = 0; i < grf->nb_boxes; i++) { control[i] = ((color[i] != WHITE) && (control[i])); }

  grf_remake(grf, control);
free(color);
free(control);
}




//TODO: use stack

static int check_for_acyclicity(grf_t * grf, char * color, int boxno) {

  stack_type_int * stack = stack_new_int();

  stack_push_int(stack, GRF_BOX_INIT);

  while (! stack_empty_int(stack)) {

    int i = stack_head_int(stack);
    
    if (color[i] == BLACK) { stack_pop_int(stack); continue; }
    // if (color[i] == GRAY)  { return false; }

    color[i] = GRAY;

    grf_trans * trans = grf->boxes[i]->trans;
    
    while ((trans) && (color[trans->to] == BLACK)) { trans = trans->next; }

    if (trans == NULL) { color[i] = BLACK; stack_pop_int(stack); }

    while (trans) {
      if (color[trans->to] == GRAY) { return false; }
      if (color[trans->to] == WHITE) { stack_push_int(stack,trans->to); }
      trans = trans->next;
    }
  }

  stack_delete_int(stack);

  return 1;
}



int grf_is_acyclic(grf_t * grf) {

  char* color=(char*)malloc(sizeof(char)*grf->nb_boxes);

  for (int i = 0; i < grf->nb_boxes; i++) { color[i] = WHITE; }

  int k=check_for_acyclicity(grf, color, GRF_BOX_INIT);
  free(color);
  return k;
}







static void insert_transitions(grf_t * grf, int from, int to) {

  for (grf_trans * trans = grf->boxes[to]->trans; trans; trans = trans->next) {

    if (u_strcmp(grf->boxes[trans->to]->label, epsilon) == 0) {
      insert_transitions(grf, from, trans->to);
    } else {
      grf_add_trans(grf, from, trans->to);
    }
  }
}




/* delete epsilon transitions (all boxes labeled with '<E>') except for the init one
 * grf must be acyclic.
 */


void grf_remove_epsilons(grf_t * grf) {

  char* tab=(char*)malloc(sizeof(char)*grf->nb_boxes);


  int nb  = 0;

  for (int i = 0; i < grf->nb_boxes; i++) {

    if ((i == GRF_BOX_INIT) || (i == GRF_BOX_FINAL) || (u_strcmp(grf->boxes[i]->label, epsilon) != 0)) {

      tab[i] = 1; nb++;

      for (grf_trans * trans = grf->boxes[i]->trans; trans; trans = trans->next) {

      	if (! u_strcmp(grf->boxes[trans->to]->label, epsilon)) { insert_transitions(grf, i, trans->to); }
      }

    } else { tab[i] = 0; }
  }

  if (nb == grf->nb_boxes) { /* no <E> box */
    free(tab);
    return;
  }

  grf_remake(grf, tab);
  free(tab);
}



static int is_good_label(unichar * label) {

  unichar * p;

  if ((label == NULL)) {
    error("error: check label: no label\n");
    return 0;
  }

  if (*label == 0) {
    error("error: check label: label is empty\n");
    return 0;
  }


  if (*label == '{') { /* {__,__.__} */

    p = label + 1;

    while (*p !=  ',') { // look for ','
      if (*p == 0) { error("error: malformed label: ',' is missing.\n"); return 0; }
      p++;
    }
    
    while (*p != '.') {
      if (*p == 0) { error("error: malformed label: '.' is missing.\n"); return 0; }      
      p++;
    }

    while (*p != '}') {
      if (*p == 0) { error("error: malformed label: '}' is missing.\n"); return 0; }      
      p++;
    }

    p++;
    if (*p != 0) { error("error: malformed label: label should end with '}'.\n"); return 0; }

  } else { // no spaces

    for (p = label; *p; p++) {

      if ((*p == ' ') || (*p == '\t') || (*p == '\n')) {

	error("error: malformed label: spaces are disallowed in ??? labels.\n");
	return 0;
      }
    }
  }

  return 1;
}



int grf_check_labels(grf_t * grf) {

  for (int i = 0; i < grf->nb_boxes; i++) {

    if (i == GRF_BOX_FINAL) { continue; }


    if (! is_good_label(grf->boxes[i]->label)) {

      error("bad label in box #%d\n", i);
      return -1;
    }
  }

  return 0; 
}





/************************************************************************************
 *
 * minimisationffs...
 *
 ************************************************************************************/




void grf_calc_heights(grf_t * grf, int * heights) {

  
  for (int i = 0; i < grf->nb_boxes; i++) { heights[i] = UNDEF; }

  heights[GRF_BOX_FINAL] = 0;


  stack_type_int * stack = stack_new_int();

  stack_push_int(stack,GRF_BOX_INIT);

  while (! stack_empty_int(stack)) {

    int i = (int) stack_head_int(stack);

    if (heights[i] != UNDEF) { stack_pop_int(stack); continue; }


    grf_trans * trans = grf->boxes[i]->trans;

    while ((trans) && (heights[trans->to] != UNDEF)) {

      heights[i] = MAX(heights[i], heights[trans->to] + 1);
      trans = trans->next;
    }


    if (trans == NULL) { // we have heights[i]

      stack_pop_int(stack);

    } else {            // try again next time ...

      heights[i] = UNDEF;

      while (trans) {
	if (heights[trans->to] == UNDEF) { stack_push_int(stack, trans->to); }
	trans = trans->next;
      }
    }
  }

  stack_delete_int(stack);
}



/*
 * reorder boxes according to tab:
 *      newplace(boxe[i]) <= newplace(boxe[j]) <=> tab[i] <= tab[j] (don't touch ini and final boxes)
 *
 * useful for reordering boxes by their height.
 */



static int compareint(void * _a, void * _b, void * data) {

  int a = *((int *) _a);
  int b = *((int *) _b);

  return (a - b);
}


static int comparebox(void * _a, void * _b, void * data) {

  int a     = *((int *) _a);
  int b     = *((int *) _b);
  int * tab = (int *) data;

  return (tab[a] - tab[b]);
}




/* reorder the box according to the tab and reorder the tab accordingly
 *
 * ie:  if tab[i] < tab[j] then the box i will be placed before the box j
 *
 */


void grf_reorder_by_tab(grf_t * grf, int * tab) {

  int* newpos=(int*)malloc(sizeof(int)*grf->nb_boxes);

  for (int i = 0; i  < grf->nb_boxes; i++) {
    newpos[i] = i;
  }

  qsort(newpos + 2, grf->nb_boxes - 2, sizeof(int), comparebox, tab);


  /* we have to reorder according to newpos[] */

  int* newnum=(int*)malloc(sizeof(int)*grf->nb_boxes);

  for (int i = 0; i < grf->nb_boxes; i++) {
    newnum[newpos[i]] = i;
  }

  grf_reorder(grf, newnum);


  // reorder the tab accordingly

  qsort(tab + 2, grf->nb_boxes - 2, sizeof(int), compareint, NULL);
  free(newpos);
  free(newnum);
}







static int is_equiv(grf_t * grf, int i, int j, int * id) {

  int nb = 0;

  grf_trans * transi, * transj;

  for (transi = grf->boxes[i]->trans; transi; transi = transi->next, nb++) {

    for (transj = grf->boxes[j]->trans;

	 transj
	   && (transi->to != transj->to) 
	   && ((id[transi->to] != id[transj->to]) || (u_strcmp(grf->boxes[transi->to]->label, grf->boxes[transj->to]->label) != 0));

	 transj = transj->next);

    if (transj == NULL) { return 0; }


  }


  for (grf_trans * trans = grf->boxes[j]->trans; trans; trans = trans->next, nb--);

  return (nb == 0);
}



static void calc_ids(grf_t * grf, int min, int max, int * ids, int * NEWID) {

  for (int i = min; i <= max; i++) {

    if (ids[i] != UNDEF) { continue; }

    ids[i] = (*NEWID)++;

    for (int j = i + 1; j <= max; j++) {
      
      if ((ids[j] == UNDEF) && (is_equiv(grf, i, j, ids))) { ids[j] = ids[i];  }

    }
  }

}



void grf_calc_ids(grf_t * grf, int * ids) {


  int* heights=(int*)malloc(sizeof(int)*grf->nb_boxes);

  grf_calc_heights(grf, heights);

  grf_reorder_by_tab(grf, heights);

  for (int i = 0; i < grf->nb_boxes; i++) { ids[i] = UNDEF; }

  int NEWID = 0;

  ids[GRF_BOX_INIT]  = NEWID++;
  ids[GRF_BOX_FINAL] = NEWID++;

  int i = 2;
  int min;

  while (i < grf->nb_boxes) {

    min = i;

    while ((i < grf->nb_boxes) && (heights[min] == heights[i])) { i++; }

    calc_ids(grf, min, i - 1, ids, & NEWID);
  }
  free(heights);
}
