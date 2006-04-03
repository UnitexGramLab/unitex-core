 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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



//
//
//
//	 comparer deux automate 
//	first automate is text automates
//	seconde automate is motif automates
#include "Fst2.h"

class motif_auto_sur_texte_autos {
	int startAutoNum;
	int curAuto;
	Automate_fst2* Motif_Automate;
	Automate_fst2* Text_Automate;

#define	PATH_QUEUE_MAX	256
	struct trace_queue {
		int path;
		int eti;
	}pathEtiQ[PATH_QUEUE_MAX];
	int pathEtiQidx;

#define MAX_SOUS_AUTO_DEPTH	2048
	struct stackAuto {
		int aId;	// return number of state number
		int next;
	} CautoQueue[MAX_SOUS_AUTO_DEPTH];
	int CautoDepth;

public:
	motif_auto_sur_texte_autos()
	{
	};
	~motif_auto_sur_texte_autos()
	{
	};

	void locate_pattern_at_Autos(int mode,int output_mode)
	{
		startAutoNum = 	curAuto = 1;
		CautoDepth = 0;
		CautoQueue[CautoDepth].aId = curAuto;
		CautoQueue[CautoDepth].next = 0;
		
		pathEtiQidx = 0;
		pathEtiQ[pathEtiQidx].path = curAuto;
		pathEtiQ[pathEtiQidx].eti = 0;
		pathEtiQidx++;
		findCycleSubGraph(Motif_Automate->debut_graphe_fst2[curAuto],0,0);
		pathEtiQidx--;
		if(pathEtiQidx)exitMessage("error in program");
	}
	void findCycleSubGraph(int autoNo, int pos,int depth)
	{
	int saveAuto;
	int skipCnt = 0;
	int i;
	int nEtat;
	

	if( pathEtiQidx > PATH_QUEUE_MAX)	return;

	if( Motif_Automate->etat[autoNo]->control & FST2_FINAL_STATE_BIT_MASK) {	// terminal node 
		if(curAuto != startAutoNum){		// check continue  condition
			skipCnt = 0;	// find next state
			for(i = CautoDepth;i>=0; --i){
				if(CautoQueue[i].aId == -1) 
					skipCnt++;
				else {
					if(skipCnt) 
						skipCnt--;
					else
						break;
				}
			}
			// ?
			if( i == 0) exitMessage("not want state arrive");	

			curAuto = CautoQueue[i].aId;
			nEtat = CautoQueue[i].next;
			CautoDepth++;
			CautoQueue[CautoDepth].aId = -1;
			CautoQueue[CautoDepth].next = 0;

			pathEtiQ[pathEtiQidx].path = nEtat;
			pathEtiQ[pathEtiQidx].eti = 0;
			pathEtiQidx++;
			findCycleSubGraph(nEtat,pos,depth+1);
			pathEtiQidx--;
			CautoDepth--;
		} else {	// stop condition
			afficher_match_fst2(0,L"");
		}
	}
	for(struct transition_fst *sui = Motif_Automate->etat[autoNo]->transitions;
	sui != 0 ; sui = sui->suivant){
		if(sui->etiquette & FILE_PATH_MARK ) {	// handling sub call
			CautoDepth++;
			CautoQueue[CautoDepth].aId = curAuto;
			CautoQueue[CautoDepth].next = sui->arr;
			saveAuto = curAuto;
			curAuto = sui->etiquette & SUB_ID_MASK;

			pathEtiQ[pathEtiQidx].path = sui->etiquette;
			pathEtiQ[pathEtiQidx].eti = 0;
			
			++pathEtiQidx;
			findCycleSubGraph(Motif_Automate->debut_graphe_fst2[curAuto],pos,depth+1);
			--pathEtiQidx;			
			curAuto = saveAuto;
			--CautoDepth;
			continue;
		}
		// verifiy next condition 
		for( struct transition_fst *tsui = Text_Automate->etat[pos]->transitions;
		tsui != 0; tsui = tsui->suivant){
			if(matchVerify(sui->etiquette,tsui->etiquette)){
				// forward 
				pathEtiQ[pathEtiQidx].path = sui->arr;
				pathEtiQ[pathEtiQidx].eti = sui->etiquette;
				++pathEtiQidx;

				findCycleSubGraph(sui->arr,tsui->arr,depth+1);
				pathEtiQidx--;
			}

		}
	}
	}
	
	int matchVerify(int etiOrg,int etiText){
		return(1);
	}

};
