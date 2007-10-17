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

#include "String_hash2.h"
struct sentence_node {
	struct sentence_tran *trans;
};
struct sentence_tran {
	int word;
	int eti;
	int pos;	// position on text
	struct sentence_node *arr;
	struct sentenec_tran *next;
};
struct sentence_stack {
	struct sentence_tran *trans;
	struct sentence_stack *forward;
	struct sentence_stack *backward;
};
class sentenceKr {
	pageHandle nodes;
	pageHandle transitions;
	pageHandle stack;
	struct sentence_stack *stack_tail;
	FILE *senF;
public:
	sentenceKr()
	{
		nodes.setSz(sizeof(struct sentence_node)*4096,
			sizeof(struct sentence_node));
		transitions.setSz(
			sizeof(struct sentence_tran)*4096
			,sizeof(struct sentenc_tran));
		transitions.setSz(
			sizeof(struct sentence_stack)*4096
			,sizeof(struct sentenc_stack));

		stack_tail = getStackPage();
	};
	~sentenceKr(){};
	struct sentence_stack *getStackPage()
	{
		struct sentence_stack *hp,*tp,*cp;
		for( i = 0; i < 4096;i++)
		{
			cp = (struct sentence_stack *)stack.get();
			cp->forward = 0;
			cp->backward = 0;
			cp->trans = 0;
			if(i){
				tp->forward = cp;
				cp->backward = tp;
				tp = cp;
			}else{
				hp = cp;
				tp = cp;
			}
		}
		return(hp);
	}
	// construction sentence auto
	struct sentence_tran *insert(int word,int eti,int pos)
	{
		struct sentence_tran **t = &curNode->transitions;
		while(*t) t = &t->next;
		*t = (struct sentence_tran *)transition.get();
	};
	link(struct sentence_node *dsN,struct sentence_node *deN,
		struct sentence_node *ssN, struct sentenec_node *seN);
	struct sentence_tran *getNextId()
	{
		if(stack_tail->trans->next){
			struct sentence_tran *getTr= stack_tail->trans->next;
			if(!stack_tail->forward){
				stack_tail->forward = getStackPage();
				stack_tail->forward->backward = stack_tail->forward;
			}
			stack_tail = stack_tail->forward;
			stack_tail->trans = getTr->arr->trans;
			return(getTr);
		}
		stack_tail = stack_tail->backward;
		if(!stack_tail) // finish all path in the current sentence
			return((struct sentence_tran *)-1);
		return(getnextId());
	};
	void saveSentenceFile(){
		
	}
	void loadSentence()
	{
	}
};
