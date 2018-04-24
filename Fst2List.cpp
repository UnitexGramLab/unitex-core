/*
 * Unitex
 *
 * Copyright (C) 2001-2018 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <locale.h>
#include "Unicode.h"
#include "Fst2.h"
#include "AbstractFst2Load.h"
#include "Alphabet.h"
#include "Copyright.h"
#include "File.h"
#include "Error.h"
#include "Transitions.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char
    * usage_Fst2List =
        "Usage:\n"
          "Fst2List [-o outFile][-p s/f/d][-[a/t] s/m] [-m] [-d] [-f s/a][-s[0s] \"Str\"] [-r[s/l] \"Str\"] [-l line#] [-i subname]* [-c SS=0xxxx]* fname\n"
          " fname : name of the input file name with extension \".fst2\"\r\n"
          " -S : write path on standard output\r\n"
          " -o outFile : if this option and -S not exist, save paths at \"file\"lst.txt\r\n"
          " -[a/t] s/m : mode de automata or transducteur, s=single initial, m = multi-inital\r\n"
          "              by default \"-a s\"\r\n"
          " -l line#  :  max number of line to save[decimal].\r\n"
          " -i sname : stop call of the exploitation at this sub-graphe \"sname\"\r\n"
          " -p s/f/d : mode de extrait word, s=each sub graphs, f=full path(default),\r\n"
          "            d= debugging. default is 'f'\r\n"
          " -c SS=0xXXXX: change the symbol string between symbols < and >,\"<SS>\" \r\n"
          "                to a unicode character(0xXXXX)\r\n"
          " -s \"L[,R]\" : use two strings L, R as the separator each item\r\n"
          "                   default null\r\n"
          " -s0 \"Str\" : if transductor mode,set \"str\" as the separator between input and out\r\n"
          "                   default null\r\n"
          " -f  a/s :  if the mode is transductor,the format of output line i0i1SOS1(:s) or i0S0i1S1(:a),i0,i1: input, S0,S1:out\r\n"
          "       default value is \'s\'\r\n"
          " -ss \"stop\" : set \"str\" as the mark of stop exploitation at \"<stop>\" \r\n"
          "                    default null\r\n"
          " -m : mode special for description with alphabet\r\n"
          " -d : disable loop check: faster execution at the cost of information about loops\r\n"
          " -v : verbose mode  default null\r\n"
          " -r[s/l/x] \"L[,R]\"  : present recusive path(c0|...|cn) by Lc0|..|cnR : default null\r\n"
          " -V/--only-verify-arguments: only verify arguments syntax and exit\r\n"
          " -h/--help: this help\r\n";

static void usage() {
  display_copyright_notice();
  u_printf(usage_Fst2List);
}

static char *getUtoChar(char charBuffOut[], unichar *s) {
  int i;
  for (i = 0; (i < 1024) && s[i]; i++)
    charBuffOut[i] = (char) s[i];
  charBuffOut[i] = 0;
  return (charBuffOut);
}
enum ModeOut {
  PR_SEPARATION, PR_TOGETHER
}; // make word and output separated or together
enum printOutType {
  GRAPH, FULL, FST2LIST_DEBUG
};
enum initialType {
  SINGLE, MULTI
}; // 'multi' is used for graph with several starting state
enum presentCycleValue {
  SYMBOL, LABEL, STOP
};
enum autoType {
  AUTOMODE, TRANMODE
};

static unichar *uascToNum(unichar *uasc, int *val);
#define FILE_PATH_MARK  0x10000000
#define LOOP_PATH_MARK  0x20000000
#define STOP_PATH_MARK  0x40000000
#define PATHID_MASK    0x1FFFFFFF
#define SUB_ID_MASK    0x0fffffff
#define CTL_MASK    0xe0000000
#define LOOP_NODE_MARK  0x80
#define DIS_LINE_LIMIT_MAX  4096

#define MAX_CHANGE_SYMBOL_SIZE 32
#define MAGIC_OUT_STDOUT "<WRITE_U_STDOUT>"

static int changeStrToVal(int &changeStrToIdx, unichar changeStrTo[][MAX_CHANGE_SYMBOL_SIZE], char *src) {
  char *wp = src;

  int ptLoc = 0;
  int i;

  i = 1;
  changeStrTo[changeStrToIdx][i++] = (unichar) '<';
  for (wp = src; i < MAX_CHANGE_SYMBOL_SIZE && (*wp); i++) {
    if (*wp == (unichar) '=')
      break;
    changeStrTo[changeStrToIdx][i] = (unsigned short) *wp++;
  }
  if (*wp != (unichar) '=')
    return (1);
  if (i > (MAX_CHANGE_SYMBOL_SIZE - 2)) {
    u_printf("the name of the variable is too long %s", src);
    return 1;
  }
  changeStrTo[changeStrToIdx][i++] = (unichar) '>';
  changeStrTo[changeStrToIdx][i++] = (unichar) '\0';
  ptLoc = i;
  if (!*wp) {
    usage();
    return 1;
  }
  for (wp++; i < MAX_CHANGE_SYMBOL_SIZE && (*wp); i++) {
    changeStrTo[changeStrToIdx][i] = (unsigned short) *wp++;
  }
  if (*wp != (unichar) '\0') {
    return (1);
  }
  changeStrTo[changeStrToIdx][i++] = (unichar) '\0';
  uascToNum(&changeStrTo[changeStrToIdx][ptLoc], &i);
  changeStrTo[changeStrToIdx][0] = (unsigned short) i;

  char charBuffOut[1024];
  u_printf("Change symbol %s --> %x\n", getUtoChar(charBuffOut,&changeStrTo[changeStrToIdx][1]),changeStrTo[changeStrToIdx][0]);

  changeStrToIdx++;
  return (0);
}
static unichar u_null_string[] = { (unichar) '\0', (unichar) '\0' };
static unichar u_epsilon_string[] = { (unichar) '<', (unichar) 'E',(unichar) '>', (unichar) '\0' };

static const char *StrMemLack = "allocation of memory for cycle data failed";

/**
 * struct to keep track 
 * of the path and tag
 * in a queue
 */
struct pathAndTag {
  int autoNo;        // automaton's number
  int stateNo;       // state's number
  int tag;
};

//
//
//

class CFstApp {

public:

  struct fst2 *a;
  struct FST2_free_info fst2_free;
  U_FILE* foutput;
  ModeOut prMode;
  autoType automateMode;
  int listOut;
  int verboseMode;
  int enableLoopCheck;
  //  int control_char; // control the output for control_chars <>
#define  PATH_QUEUE_MAX  1024
  struct pathAndTag pathTagQ[PATH_QUEUE_MAX];
  int pathTagQidx;
#define TAGQ_MAX    1024

  //
  // print out all path in the .fst2
  //    first : check and mark all close paths in each sub-automaton
  //        print out to the file [filename]LstAuto.txt
  //   second : print out all open paths
  //        at [filename]L.txt
  //
  void loadGraph(int &changeStrToIdx, unichar changeStrTo[][MAX_CHANGE_SYMBOL_SIZE], char *fname);
  int exploreSubAuto(int startSubAutoNum);
  int getWordsFromGraph(int &changeStrToIdx, unichar changeStrTo[][MAX_CHANGE_SYMBOL_SIZE], char *fst2_file_name);
  int findCycleSubGraph(int autoNo, int autodep, int testState, int depthState);
  int outWordsOfGraph(int depth);

  // the stack which keeps the path and called sub graph
  struct stackAuto {
    int aId;
    int next;
  } calledAutoQ[2048];
  int calledAutoDepth;

  void CqueuePathPr(U_FILE *f);

  int *ignoreTable;  // 1 where the automaton is ignored, else 0
  int *numOfIgnore;

  int outLineLimit;
  int numberOfOutLine;
  int count_in_line; // number of out per line


  int totalPath;
  int totalLoop; // number of loops present in the automaton
  int stopPath;
  int errPath;

  presentCycleValue recursiveMode;
  printOutType display_control;
  initialType traitAuto; // single or multi initial state
  int niveau_traite_mot;
  int depthDebug;

  unichar *saveSep, *sepL; // for input and output
  unichar *sepR; // for input and output
  unichar *sep1; // for each input/output
  unichar *stopSignal;
  unichar *saveEntre, *entreGO, *entreGF;
  VersatileEncodingConfig vec;
  char ofdirName[1024];
  char ofExt[16];
  char ofnameOnly[512];        // output file name
  char defaultIgnoreName[512]; // input file name
  void fileNameSet(char *ifn, char *ofn) {
    char tmp[512];
    remove_path(ifn, tmp);
    remove_extension(tmp, defaultIgnoreName);
    if (!ofn) {
      get_path(ifn, ofdirName);
      remove_path(ifn, tmp);
      remove_extension(tmp, ofnameOnly);
      strcpy(ofExt, ".txt");
    } else {
      get_path(ofn, ofdirName);
      remove_path(ofn, tmp);
      remove_extension(tmp, ofnameOnly);
      get_extension(tmp, ofExt);
    }
    if (ofnameOnly[0] == 0) {
      fatal_error("ofile name not correct");
    }
  }

  /**
   * builds the output file's name
   */
  void makeOfileName(char *des, const char *fn, const char *ext) {
    strcpy(des, ofdirName);
    if (fn)
      strcat(des, fn);
    else
      strcat(des, ofnameOnly);
    if (ext)
      strcat(des, ext);
    else
      strcat(des, ofExt);
  }

  CFstApp() :
    a(0), fst2_free(FST2_free_info_init), foutput(0),
        prMode(PR_SEPARATION), automateMode(AUTOMODE), listOut(0),
        verboseMode(0), enableLoopCheck(1), pathTagQidx(0),

        calledAutoDepth(0),

        ignoreTable(0), numOfIgnore(0),

        outLineLimit(0x10000000), numberOfOutLine(0), count_in_line(0),

        totalPath(0), totalLoop(0), stopPath(0), errPath(0),

        recursiveMode(STOP), display_control(FULL),
        traitAuto(SINGLE),
        niveau_traite_mot(1), // unit of box is word
        depthDebug(0),

        saveSep(u_null_string), sepL(u_null_string),
        sepR(u_null_string), sep1(u_null_string), stopSignal(
            u_null_string), saveEntre(u_null_string), entreGO(
            u_null_string), entreGF(u_null_string),

        autoStackMap(NULL), mapOfCallHead(NULL), mapOfCallTail(NULL),

        cycInfos(NULL),

        headCyc(0), cyclePathCnt(0), headCycNodes(0), cycNodeCnt(0),

        ePtrCnt(0), tPtrCnt(0), EOutCnt(0), SOutCnt(0),

        arretSubListIdx(0) {
    initCallIdMap();
  }
  ;
  ~CFstApp() {
    arretExpoDel();
    cleanCyclePath();
    free_abstract_Fst2(a, &fst2_free);
    if (saveSep != u_null_string)
      delete saveSep;
    if (sep1 != u_null_string)
      delete sep1;
    if (stopSignal != u_null_string)
      delete stopSignal;
    if (saveEntre != u_null_string)
      delete saveEntre;
    if (ignoreTable)
      delete[] ignoreTable;
    if (numOfIgnore)
      delete[] numOfIgnore;
    deleteCallIdMap();
  }
  ;
  void CleanPathCounter() {
    totalPath = totalLoop = errPath = stopPath = 0;
  }
  //
  //    identify sub calls with transition and call id
  //

  void CloseOutput() {
    if ((foutput != NULL) && (foutput != U_STDOUT)) {
      u_fclose(foutput);
    }
    foutput = NULL;
  }

  /**
   * Map associating the transitions and their ID
   */
  struct callStackMapSt {
    Transition* tran;
    int autoId;
  }*autoStackMap;
  struct callIdMap {
    int cnt;
    Transition** list;
    struct callIdMap *next;
  }*mapOfCallHead, *mapOfCallTail;

  /**
   * return the ID of the transition called
   */
  int callIdentifyId(struct callStackMapSt *cmap, int count) {
    int id = 0;
    int i;
    struct callIdMap *fPtr;
    fPtr = mapOfCallHead;
    while (fPtr) {
      if (fPtr->cnt == count) {
        for (i = 0; i < count; i++) {
          if (fPtr->list[i] != cmap[i].tran) {
            break;
          }
        }
        if (i == count) {
          return id;
        }
      }
      id++;
      fPtr = fPtr->next;
    }
    fPtr = new struct callIdMap;
    fPtr->cnt = count;
    fPtr->next = 0;
    fPtr->list = new Transition*[count];
    for (i = 0; i < count; i++) {
      fPtr->list[i] = cmap[i].tran;
    }

    if (mapOfCallTail) {
      mapOfCallTail->next = fPtr;
      mapOfCallTail = fPtr;
    } else {
      mapOfCallTail = mapOfCallHead = fPtr;
    }
    return id;
  }
  void initCallIdMap() {
    autoStackMap = new struct callStackMapSt[1024];
    mapOfCallHead = mapOfCallTail = 0;
  }
  void deleteCallIdMap() {
    delete[] autoStackMap;
    while (mapOfCallHead) {
      mapOfCallTail = mapOfCallHead;
      mapOfCallHead = mapOfCallHead->next;
      delete[] mapOfCallTail->list;
      delete mapOfCallTail;
    }
  }

  //
  // for level
  //
  //
  //  save all cycle identify
  //
  struct cyclePathMark {
    int index; // nomber of identify
    struct pathAndTag *pathTagQueue;
    int pathCnt;
    int flag;
    struct cyclePathMark *next;
  };
  //
  //  save all node which has the cycle path
  //  the path from initial to the node is used to identify the node
  //  in sub graph
  //
  struct linkCycle {
    struct cyclePathMark *cyc;
    struct linkCycle *next;
  }*cycInfos;

  /**
   * structure to hold
   * cycle informations
   */
  struct cycleNodeId {
    int index;
    int autoNo;
    int stateNo;
    int tag;
    struct linkCycle *cycInfos;
    int flag;
    struct cycleNodeId *next;
  };
  struct cyclePathMark *headCyc;
  int cyclePathCnt;
  struct cycleNodeId *headCycNodes;
  int cycNodeCnt;

  unichar *getLabelNumber(unichar*aa, int numOfPath, int &flag, int curidx, int setflag) {
    struct cycleNodeId *cnode = headCycNodes;
    int i;
    int searchState = pathTagQ[curidx].stateNo & PATHID_MASK;
    int searchStateAuto = pathTagQ[curidx].autoNo;
    int searchTag = pathTagQ[curidx].tag;
    while (cnode) {
      if ((searchStateAuto == cnode->autoNo) && (searchState == cnode->stateNo) && (searchTag == cnode->tag)) {
        break;
      }
      cnode = cnode->next;
    }
    if (!cnode) {
      error("%d/%d stack\n", numOfPath, curidx);
      for (i = 0; i < pathTagQidx; i++) {
        error("%d : (%08x:%08x) : %08x\n", i, pathTagQ[i].autoNo, pathTagQ[i].stateNo, pathTagQ[i].tag);
      }

      CqueuePathPr(U_STDERR);
      fatal_error("eu~ak\n");
    }
    if (setflag) {
      flag = cnode->flag;
      if (!cnode->flag) {
        cnode->flag = 1;
      }
    }

    //unichar aa[64];
    u_sprintf(aa, "Loc%d", cnode->index);
    return ((unichar *) aa);
  }

  void setIdentifyValue(int offset, int cntNode) {
    struct cycleNodeId **cnode = &headCycNodes;

    int cycStateNo = pathTagQ[cntNode - 1].stateNo & PATHID_MASK;
    int cycStateAutoNo = pathTagQ[cntNode - 1].autoNo;
    int cycStateTag = pathTagQ[cntNode - 1].tag;
    while (*cnode) {
      if (((*cnode)->autoNo == cycStateAutoNo) && ((*cnode)->stateNo == cycStateNo) && ((*cnode)->tag == cycStateTag)) {
        break;
      }
      cnode = &((*cnode)->next);
    }
    if (!*cnode) {
      *cnode = new struct cycleNodeId;
      if (!(*cnode)) {
        fatal_error(StrMemLack);
      }
      (*cnode)->next = 0;
      (*cnode)->cycInfos = 0;
      (*cnode)->index = cycNodeCnt++;
      (*cnode)->autoNo = cycStateAutoNo;
      (*cnode)->stateNo = cycStateNo;
      (*cnode)->tag = cycStateTag;
      (*cnode)->flag = 0;
    }

    struct cyclePathMark *pCyc = getLoopId(offset);
    struct linkCycle **alc = &((*cnode)->cycInfos);
    while (*alc) {
      if (pCyc->index == (*alc)->cyc->index) {
        return;
      }
      if ((*alc)->cyc->index < pCyc->index) {
        break;
      }
      alc = &((*alc)->next);
    }
    *alc = new struct linkCycle;
    if (!(*alc)) {
      fatal_error(StrMemLack);
    }
    (*alc)->next = 0;
    (*alc)->cyc = pCyc;
  }

  struct cyclePathMark *getLoopId(int offset) {
    struct cyclePathMark **h = &headCyc;
    int numOfPath;
    int i, j;
    offset++;
    numOfPath = pathTagQidx - offset;
    while (*h) {
      if ((*h)->pathCnt == numOfPath) {
        for (i = 0; i < numOfPath; i++) {
          if ((pathTagQ[offset].autoNo
              == (*h)->pathTagQueue[i].autoNo)
              && ((pathTagQ[offset].stateNo & PATHID_MASK)
                  == (*h)->pathTagQueue[i].stateNo)
              && (pathTagQ[offset].tag
                  == (*h)->pathTagQueue[i].tag)) {
            break;
          }
        }
        if (i != numOfPath) { // find first the position in cycle ring
          for (j = 0; j < numOfPath; j++) {
            if (((pathTagQ[offset + j].stateNo & PATHID_MASK)
                != (*h)->pathTagQueue[i].stateNo)
                || (pathTagQ[offset + j].tag
                    != (*h)->pathTagQueue[i].tag)
                || (pathTagQ[offset + j].autoNo
                    != (*h)->pathTagQueue[i].autoNo)) {
              break;
            }
            if (++i >= numOfPath) {
              i = 0;
            }
          }
          if (j == numOfPath) {
            return ((*h));
          }
        }
      }
      if ((*h)->pathCnt < numOfPath) {
        break;
      }
      h = &((*h)->next);
    }
    struct cyclePathMark *tmp = new struct cyclePathMark;
    tmp->flag = 0;
    tmp->index = 0;
    tmp->pathCnt = 0;
    tmp->next = *h;
    *h = tmp;
    if (!(*h)) {
      fatal_error(StrMemLack);
    }
    (*h)->pathTagQueue = new struct pathAndTag[numOfPath];
    if (!((*h)->pathTagQueue)) {
      fatal_error(StrMemLack);
    }
    for (i = 0; i < numOfPath; i++) {
      (*h)->pathTagQueue[i].stateNo = pathTagQ[i + offset].stateNo
          & PATHID_MASK;
      (*h)->pathTagQueue[i].tag = pathTagQ[i + offset].tag;
      (*h)->pathTagQueue[i].autoNo = pathTagQ[i + offset].autoNo;

    }
    (*h)->pathCnt = numOfPath;
    (*h)->index = cyclePathCnt++;

    return ((*h));
  }

  void cleanCyclePath() {
    struct cycleNodeId *cnode = headCycNodes;
    struct cycleNodeId *tnode;
    struct linkCycle *inf, *tnf;
    struct cyclePathMark *tc, *cp;
    int i;
    while (cnode) {
      tnode = cnode->next;
      inf = cnode->cycInfos;
      while (inf) {
        tnf = inf->next;
        delete inf;
        inf = tnf;
      }
      delete cnode;
      cnode = tnode;
    }
    headCycNodes = 0;
    cycNodeCnt = 0;
    cp = headCyc;
    while (cp) {
      tc = cp->next;
      delete cp->pathTagQueue;
      delete cp;
      cp = tc;
    }
    headCyc = 0;
    cyclePathCnt = 0;
    if (a != NULL) {
      for (i = 0; i < a->number_of_states; i++) {
        a->states[i]->control &= 0x7f; // clean to mark recursive. 0x7f=0b0111_1111
      }
    }
  }
  void prCycleNode() {
    struct cycleNodeId *cnode = headCycNodes;
    u_fprintf(foutput, "cycle Nodes %d\n", cyclePathCnt);
    while (cnode) {
      u_fprintf(foutput, "%d (%d:%d)\n", cnode->index, cnode->autoNo,
          cnode->stateNo);
      cnode = cnode->next;
    }
  }

  int WasCycleNode(int cauto, int cstate) {
    struct cycleNodeId **cnode = &headCycNodes;

    while (*cnode) {
      u_printf("cnode : %d", (*cnode)->autoNo);
      if (((*cnode)->autoNo == cauto) && ((*cnode)->stateNo == cstate)) {
        return 1;
      }
      cnode = &((*cnode)->next);
    }
    return 0;
  }

  /**
   * detect a recursive call
   * increment 'totalLoop' if a loop is found
   */
  int IsCyclePath(int depth) {
    int scanner; 
    int curId = pathTagQ[pathTagQidx - 1].stateNo & PATHID_MASK;
    int curAutoId = pathTagQ[pathTagQidx - 1].autoNo;

    for (scanner = 0; scanner < pathTagQidx - 1; scanner++) {
      if (((pathTagQ[scanner].stateNo & PATHID_MASK) == curId)
          && (pathTagQ[scanner].autoNo == curAutoId)) { // find recursive path
        switch (recursiveMode) {
        case LABEL:
          if (listOut) {
            pathTagQ[pathTagQidx - 1].stateNo |= STOP_PATH_MARK;
            outWordsOfGraph(scanner);
          } else {
            //          saveNodeIndex(scanner);
            setIdentifyValue(scanner, pathTagQidx);
          }
          break;
        case SYMBOL:
          if (!listOut) {
            setIdentifyValue(scanner, pathTagQidx);
          }
          break;
        case STOP:
          if (listOut) {
            pathTagQ[scanner].stateNo |= LOOP_PATH_MARK;
            pathTagQ[pathTagQidx - 1].stateNo |= LOOP_PATH_MARK
                | STOP_PATH_MARK;
            outWordsOfGraph(depth);
            pathTagQ[scanner].stateNo &= ~LOOP_PATH_MARK;
          }
          break;
        default:
          fatal_error("illegal execution");
        }
        totalLoop++;
        return (1);
      }
    }
    return (0);
  }

  unichar EBuff[128];
  unichar TBuff[128];
  int ePtrCnt;
  int tPtrCnt;
  unichar EOUTLINE[4096];
  unichar SOUTLINE[4096];
  int EOutCnt;
  int SOutCnt;

  void resetBuffs() {
    ePtrCnt = tPtrCnt = EOutCnt = SOutCnt = 0;
  }

  int outOneWord(unichar *suf) {
    int i;
    int setOut;
    unichar *wp;
    EBuff[ePtrCnt] = TBuff[tPtrCnt] = 0;
    if (!ePtrCnt && !tPtrCnt && !suf) {
      return 0;
    }
    if (suf) {
      setOut = 0;
      //printf("%d %d %d %d \n",ePtrCnt,tPtrCnt,*suf,count_in_line);
      if (ePtrCnt || tPtrCnt || *suf || (count_in_line == 0)) {
        setOut = 1;
        if (prMode == PR_SEPARATION) {
          wp = sepL;
          while (*wp) {
            EOUTLINE[EOutCnt++] = *wp;
            if (automateMode == TRANMODE) {
              SOUTLINE[SOutCnt++] = *wp;
            }
            wp++;
          }
          for (i = 0; i < ePtrCnt; i++)
            EOUTLINE[EOutCnt++] = EBuff[i];
          if (automateMode == TRANMODE) {
            for (i = 0; i < tPtrCnt; i++) {
              SOUTLINE[SOutCnt++] = TBuff[i];
            }
          }
          wp = sepR;
          while (*wp) {
            if (ePtrCnt) {
              EOUTLINE[EOutCnt++] = *wp;
            }
            if (automateMode == TRANMODE) {
              SOUTLINE[SOutCnt++] = *wp;
            }
            wp++;
          }
        } else {
          wp = sepL;
          while (*wp) {
            EOUTLINE[EOutCnt++] = *wp++;
          }
          for (i = 0; i < ePtrCnt; i++) {
            EOUTLINE[EOutCnt++] = EBuff[i];
          }
          wp = saveSep;
          while (*wp) {
            EOUTLINE[EOutCnt++] = *wp++;
          }
          if (automateMode == TRANMODE) {
            for (i = 0; i < tPtrCnt; i++) {
              EOUTLINE[EOutCnt++] = TBuff[i];
            }
          }
          wp = sepR;
          while (*wp) {
            EOUTLINE[EOutCnt++] = *wp++;
          }
        }
      } // condition de out
      if ((recursiveMode == LABEL) && setOut) {
        if ((automateMode == TRANMODE) && (prMode == PR_SEPARATION)) {
          wp = entreGO;
          while (*wp) {
            SOUTLINE[SOutCnt++] = *wp++;
          }
          wp = suf;
          while (*wp) {
            SOUTLINE[SOutCnt++] = *wp++;
          }
        } else {
          wp = entreGO;
          while (*wp) {
            EOUTLINE[EOutCnt++] = *wp++;
          }
          wp = suf;
          while (*wp) {
            EOUTLINE[EOutCnt++] = *wp++;
          }
        }
      }
      EOUTLINE[EOutCnt] = 0;
      SOUTLINE[SOutCnt] = 0;
      u_fputs(EOUTLINE, foutput);
      if ((automateMode == TRANMODE) && SOutCnt) {
        u_fprintf(foutput, "%S%S", saveSep, SOUTLINE);
      }
      if (display_control == FST2LIST_DEBUG) {
        CqueuePathPr(foutput);
      }
      u_fprintf(foutput, "\n");
      numberOfOutLine++;
      EOutCnt = SOutCnt = 0;
    } else { // suf == 0
      if (ePtrCnt || tPtrCnt) {
        if (prMode == PR_SEPARATION) {
          wp = sepL;
          while (*wp) {
            EOUTLINE[EOutCnt++] = *wp;
            if (automateMode == TRANMODE) {
              SOUTLINE[SOutCnt++] = *wp;
            }
            wp++;
          }
          for (i = 0; i < ePtrCnt; i++) {
            EOUTLINE[EOutCnt++] = EBuff[i];
          }
          if (automateMode == TRANMODE) {
            for (i = 0; i < tPtrCnt; i++) {
              SOUTLINE[SOutCnt++] = TBuff[i];
            }
          }
          //        if(recursiveMode == LABEL){
          //          wp = entreGO;while(*wp)  EOUTLINE[EOutCnt++] = *wp++;
          //          }
          wp = sepR;
          while (*wp) {
            EOUTLINE[EOutCnt++] = *wp;
            if (automateMode == TRANMODE) {
              SOUTLINE[SOutCnt++] = *wp;
            }
            wp++;
          }
        } else {
          wp = sepL;
          while (*wp)
            EOUTLINE[EOutCnt++] = *wp++;
          for (i = 0; i < ePtrCnt; i++)
            EOUTLINE[EOutCnt++] = EBuff[i];
          wp = saveSep;
          while (*wp)
            EOUTLINE[EOutCnt++] = *wp++;
          if (automateMode == TRANMODE) {
            for (i = 0; i < tPtrCnt; i++) {
              EOUTLINE[EOutCnt++] = TBuff[i];
            }
          }
          if (recursiveMode == LABEL) {
            wp = entreGO;
            while (*wp) {
              EOUTLINE[EOutCnt++] = *wp++;
            }
          }
          wp = sepR;
          while (*wp) {
            EOUTLINE[EOutCnt++] = *wp++;
          }
        }
        count_in_line++;
      }
    }

    ePtrCnt = tPtrCnt = 0;

    if (outLineLimit <= numberOfOutLine) {
      return 1;
    }
    return 0;
  }

  void dummyWordOut() {
    unichar *wp;
    if ((recursiveMode == LABEL) && !count_in_line) {
      wp = sepL;
      while (*wp)
        EOUTLINE[EOutCnt++] = *wp++;
      wp = saveSep;
      while (*wp)
        EOUTLINE[EOutCnt++] = *wp++;
      wp = sepR;
      while (*wp)
        EOUTLINE[EOutCnt++] = *wp++;
      wp = entreGO;
      while (*wp)
        EOUTLINE[EOutCnt++] = *wp++;
    }
    EOUTLINE[EOutCnt++] = 0;
    u_fprintf(foutput, "%S\n", EOUTLINE);
    numberOfOutLine++;
  }
  void putInt(int flag, int v) {
    if (v < 10) {
      EBuff[ePtrCnt] = v % 10 + (unichar) '0';
      if (flag) {
        TBuff[tPtrCnt++] = EBuff[ePtrCnt];
      }
      ePtrCnt++;
      return;
    }
    putInt(flag, v / 10);
    EBuff[ePtrCnt] = v % 10 + (unichar) '0';
    if (flag) {
      TBuff[tPtrCnt++] = EBuff[ePtrCnt];
    }
    ePtrCnt++;
  }
  int prOutCycle() {
    struct cyclePathMark *h = headCyc;
    int i;
    unichar *wp;
    //        unichar *wwp;
    Fst2Tag Tag;
    while (h) {
      u_fprintf(foutput, "C%d%S", h->index, entreGO);
      ePtrCnt = tPtrCnt = 0;
      for (i = 0; i < h->pathCnt; i++) {
        //        putInt(0,h->pathTagQueue[i].path);
        Tag = a->tags[h->pathTagQueue[i].tag];
        wp = (unichar *) Tag->input;

        if (u_strcmp(wp, u_epsilon_string)) {
          //          wwp = saveSep;while(*wwp) EBuff[ePtrCnt++] = *wwp++;
          while (*wp) {
            EBuff[ePtrCnt++] = *wp++;
          }
        }
        wp = (unichar *) Tag->output;
        if ((automateMode == TRANMODE) && wp && u_strcmp(wp,
            u_epsilon_string)) {
          //            wwp = saveSep;while(*wwp) EBuff[ePtrCnt++] = *wwp++;
          while (*wp) {
            TBuff[tPtrCnt++] = *wp++;
          }
        }
      }
      if (outOneWord(entreGF) != 0) {
        return 1;
      }
      h = h->next;
    }
    return 0;
  }

  int prOutCycleAtNode(int autoNum, int nodeNum) {
    struct cyclePathMark *h = headCyc;
    int i, st, ed;
    int tmp;
    unichar *wp;
    Fst2Tag Tag;
    if (autoNum) {
      st = a->initial_states[autoNum];
      ed = (autoNum == a->number_of_graphs) ? a->number_of_states
          : a->initial_states[autoNum + 1];
    } else {
      st = nodeNum;
      ed = st + 1;
    }

    while (h) {
      tmp = h->pathTagQueue[h->pathCnt - 1].stateNo;
      if (tmp & FILE_PATH_MARK) {
        tmp = a->initial_states[tmp & SUB_ID_MASK];
      }
      if ((tmp < st) || (tmp >= ed)) {
        h = h->next;
        continue;
      }

      ePtrCnt = tPtrCnt = 0;
      EBuff[ePtrCnt++] = (unichar) 'C';
      putInt(0, h->index);
      EBuff[ePtrCnt++] = (unichar) ':';

      for (i = 0; i < h->pathCnt; i++) {
        tmp = h->pathTagQueue[i].stateNo;
        if (tmp & FILE_PATH_MARK) {
          tmp = a->initial_states[tmp & SUB_ID_MASK];
        }
        if ((tmp < st) || (tmp >= ed)) {
          break;
        }

        Tag = a->tags[h->pathTagQueue[i].tag];
        wp = (unichar *) Tag->input;
        if (u_strcmp(wp, u_epsilon_string) && *wp) {
          while (*wp) {
            EBuff[ePtrCnt++] = *wp++;
          }
        }
        wp = (unichar *) Tag->output;
        if ((automateMode == TRANMODE) && wp && u_strcmp(wp,
            u_epsilon_string) && *wp) {
          while (*wp) {
            TBuff[tPtrCnt++] = *wp++;
          }
        }
      }
      if (i == h->pathCnt) {
        if (outOneWord(u_null_string) != 0) {
          return 1;
        }
      } else
        resetBuffs();
      h = h->next;
    }
    return 0;
  }
  //
  //
  //
  void prSubGrapheCycle() {
    int i;
    for (i = 1; i <= a->number_of_graphs; i++) {
      char charBuffOut[1024];
      if (a->states[a->initial_states[i]]->control & LOOP_NODE_MARK) {
        error("the sub-graph %s has cycle path\n", getUtoChar(
            charBuffOut, a->graph_names[i]));
      }
    }
  }

#define MAX_IGNORE_SUB_GRAPH  256
  unichar *arretSubList[MAX_IGNORE_SUB_GRAPH];
  int arretSubListIdx;

  void arretExpoList(char *src) {
    char *cp = src;
    unichar *wp;
    if (arretSubListIdx == MAX_IGNORE_SUB_GRAPH) {
      u_printf("too many ignored sub-graph name ignore%s\n", src);
      return;
    }
    wp = new unichar[strlen(src) + 1];
    arretSubList[arretSubListIdx++] = wp;
    while (*cp)
      *wp++ = (unichar) (*cp++ & 0xff);
    *wp = (unichar) '\0';
    if (verboseMode) {
      char charBuffOut[1024];
      u_printf("IGNORE %s\n", getUtoChar(charBuffOut,
          arretSubList[arretSubListIdx - 1]));
    }
  }
  char fileLine[1024];
  void arretExpoListFile(char *src) {
    int i;

    U_FILE* uf = u_fopen(ASCII, src, U_READ);
    if (!uf) {
      fatal_error("Cannot open file %s\n", src);
    }
    while (af_fgets(fileLine, 256, uf->f)) {
      if (fileLine[0] == ' ') {
        continue;
      }
      for (i = 0; (i < 128) && (fileLine[i] != ' ') && (fileLine[i]
          != '.') && (fileLine[i] != 0) && (fileLine[i] != 0xa)
          && (fileLine[i] != 0xd); i++) {
        EBuff[i] = (unichar) (fileLine[i] & 0xff);
      }
      EBuff[i++] = 0;

      if (arretSubListIdx == MAX_IGNORE_SUB_GRAPH) {
        u_printf("too many ignored sub-graph, name ignore %s\n", src);
        return;
      }
      arretSubList[arretSubListIdx] = new unichar[i];
      for (i = 0; EBuff[i]; i++) {
        arretSubList[arretSubListIdx][i] = EBuff[i];
      }
      arretSubList[arretSubListIdx][i] = 0;
      if (verboseMode) {
        char charBuffOut[1024];
        u_printf("IGNORE %s\n", getUtoChar(charBuffOut,arretSubList[arretSubListIdx]));
      }
      arretSubListIdx++;
    }
    u_fclose(uf);
  }

  void arretExpoDel() {
    for (int i = 0; i < arretSubListIdx; i++){
      delete arretSubList[i];
    }
  }

  /**
   * prints the automaton stack
   * state queue and automaton queue
   */
  void prAutoStack(int depStack) {
    int i;
    u_printf("===== AutoQueue\n");
    for (i = 0; i <= calledAutoDepth; i++) {
      u_printf("%d :: %d :: %d\n", i, calledAutoQ[i].aId,
          calledAutoQ[i].next);
    }
    u_printf("===== stateQueue\n");
    for (i = 0; i < pathTagQidx; i++) {
      u_printf("%d (%d ::%d)%d\n", i, pathTagQ[i].autoNo,
          pathTagQ[i].stateNo, pathTagQ[i].tag);
    }
    u_printf("===== AutoStack\n");
    Transition *k;
    for (i = 0; i < depStack; i++) {
      k = autoStackMap[i].tran;
      u_printf("%d %d(%d ::%d)\n", i, autoStackMap[i].autoId,
          k->state_number, k->tag_number);
    }
  }

  void prAutoStackOnly() {
    for (int i = 0; i < pathTagQidx; i++)
      u_fprintf(foutput, "%d (%d ::%d)%d\n", i, pathTagQ[i].autoNo,
          pathTagQ[i].stateNo, pathTagQ[i].tag);
  }

private:
  /* prevent GCC warning */

  CFstApp(const CFstApp&) :
    a(0), fst2_free(FST2_free_info_init), foutput(0),
        prMode(PR_SEPARATION), automateMode(AUTOMODE), listOut(0),
        verboseMode(0), pathTagQidx(0),

        calledAutoDepth(0),

        ignoreTable(0), numOfIgnore(0),

        outLineLimit(0x10000000), numberOfOutLine(0), count_in_line(0),

        totalPath(0), totalLoop(0), stopPath(0), errPath(0),

        recursiveMode(STOP), display_control(FULL),
        traitAuto(SINGLE),
        niveau_traite_mot(1), // unit of box is word
        depthDebug(0),

        saveSep(u_null_string), sepL(u_null_string),
        sepR(u_null_string), sep1(u_null_string), stopSignal(
            u_null_string), saveEntre(u_null_string), entreGO(
            u_null_string), entreGF(u_null_string),

        autoStackMap(NULL), mapOfCallHead(NULL), mapOfCallTail(NULL),

        cycInfos(NULL),

        headCyc(0), cyclePathCnt(0), headCycNodes(0), cycNodeCnt(0),

        ePtrCnt(0), tPtrCnt(0), EOutCnt(0), SOutCnt(0),

        arretSubListIdx(0) {
    fatal_error("Unexpected copy constructor for CFstApp\n");
  }

  CFstApp& operator =(const CFstApp&) {
    fatal_error("Unexpected = operator for CFstApp\n");
    return *this;
  }
}; // end of fstApp


void CFstApp::loadGraph(int& changeStrToIdx, unichar changeStrTo[][MAX_CHANGE_SYMBOL_SIZE], char *fname) {
  int i_1, j_1;
  Transition *strans;

  a = load_abstract_fst2(&vec, fname, 1, &fst2_free);
  if (a == NULL) {
    fatal_error("Cannot load graph file %s\n", fname);
  }

  for (i_1 = 0; i_1 < a->number_of_states; i_1++) {
    strans = a->states[i_1]->transitions;
    if (a->states[i_1]->control & 0x80) {
      fatal_error("Not null control bit");
    }
    a->states[i_1]->control &= 0x7f; // clean to mark recursive. 0x7f=0b0111_1111
    while (strans) {
      if (strans->tag_number < 0) {   // transition invokes subgraph
        strans->tag_number = FILE_PATH_MARK | -strans->tag_number;
      }
      strans = strans->next;
    }
  }

  ignoreTable = new int[a->number_of_graphs + 1];
  numOfIgnore = new int[a->number_of_graphs + 1];
  for (i_1 = 1; i_1 <= a->number_of_graphs; i_1++) {
    ignoreTable[i_1] = 0;
    numOfIgnore[i_1] = 0;
  }

  if (arretSubListIdx) {

    for (i_1 = 0; i_1 < arretSubListIdx; i_1++) {
      for (j_1 = 1; j_1 <= a->number_of_graphs; j_1++) {
        if (!u_strcmp((unichar *) a->graph_names[j_1], arretSubList[i_1])) {
          break;
        }
      }
      if (j_1 > a->number_of_graphs) {
        char charBuffOut[1024];
        u_printf("Warning : Not exist the sub-graph %s\n", getUtoChar(charBuffOut, arretSubList[i_1]));
        continue;
      }
      char charBuffOut[1024];
      u_printf("%s %d graph ignore the exploitation\n", getUtoChar(charBuffOut, a->graph_names[j_1]), j_1);

      ignoreTable[j_1] = 1;
    }
  }
  if (stopSignal) {

    for (i_1 = 0; i_1 < a->number_of_tags; i_1++) {
      if (u_strcmp((unichar *) a->tags[i_1]->input, stopSignal)) {
        continue;
      }
      for (j_1 = 0; j_1 < a->number_of_states; j_1++) {
        strans = a->states[j_1]->transitions;
        while (strans) {
          if (strans->tag_number == i_1) {
            strans->tag_number |= STOP_PATH_MARK;
          }
          strans = strans->next;
        }

      }
    }
  }
  if (changeStrToIdx) {
    unichar *wp;
    int i, j, k, l, m;
    unichar temp[256];
    for (i = 0; i < a->number_of_tags; i++) {
      wp = (unichar *) a->tags[i]->input;
      for (j = 0; wp[j]; j++) {
        temp[j] = wp[j];
      }
      temp[j] = 0;
      for (j = 0; j < changeStrToIdx; j++) {
        wp = changeStrTo[j] + 1;
        for (k = 0; temp[k]; k++) {
          for (l = 0; wp[l]; l++) {
            if (!temp[k + l] || (temp[k + l] != wp[l])) {
              break;
            }
          }
          if (wp[l]) {
            continue;
          }
          temp[k] = changeStrTo[j][0];
          l--;
          for (m = k + 1; temp[m + l]; m++) {
            temp[m] = temp[m + l];
          }
          temp[m] = 0;
        }
      }
      wp = (unichar *) a->tags[i]->input;
      if (u_strcmp(wp, temp)) {
        char charBuffOut1[1024];
        char charBuffOut2[1024];
        u_printf("%dth index, %s==>%s\n", i, getUtoChar(charBuffOut1,wp), getUtoChar(charBuffOut2, temp));
        for (j = 0; temp[j]; j++) {
          *wp++ = temp[j];
        }
        *wp = 0;
      }

    }

  }
}

/**
 * the main function to extract the words from the automaton
 */
int CFstApp::getWordsFromGraph(int &changeStrToIdx, unichar changeStrTo[][MAX_CHANGE_SYMBOL_SIZE], char *fname) {
  int i;
  char *dp;
  char ofNameTmp[1024];
  char tmpchar[1024];
  char ttpchar[1024];
  // load fst2 file
  loadGraph(changeStrToIdx, changeStrTo, fname);
  CleanPathCounter();
  ofNameTmp[0] = 0;
  switch (display_control) {
  case GRAPH: {
    if (enableLoopCheck) {
      listOut = 0;
      niveau_traite_mot = 1;
      exploreSubAuto(1); // mark loop path start nodes
      prSubGrapheCycle();
    }

    if (recursiveMode == LABEL) {
      error("warning:ignore the option -rl\r\n");
    }
    recursiveMode = SYMBOL;
    strcpy(tmpchar, ofnameOnly);
    strcat(tmpchar, "autolst");
    makeOfileName(ofNameTmp, tmpchar, ".txt");

    if (strcmp(ofnameOnly,MAGIC_OUT_STDOUT)==0) {
      foutput = U_STDOUT;
    } else {
      foutput = u_fopen(&vec, ofNameTmp, U_WRITE);
    }

    if (!foutput) {
      fatal_error("Cannot open file %s\n", ofNameTmp);
    }

    listOut = 1;

    for (i = 1; i <= a->number_of_graphs; i++) {
      u_fprintf(foutput, "[%d th automata %S]\n", i, a->graph_names[i]);
      //      printf("[%d th automata %s]\n",i,getUtoChar(a->nom_graphe[i]));

      if (exploreSubAuto(i) != 0) {
        CloseOutput();
        return 1;
      }

      if (prOutCycleAtNode(i, 0) != 0) {
        CloseOutput();
        return 1;
      }
      u_fprintf(
          foutput,
          " automaton %S, %d paths, %d path stopped by cycle, %d path overflows\n",
          a->graph_names[i], totalPath, totalLoop, errPath);
      CleanPathCounter();
    }

    if (recursiveMode == SYMBOL) {
      if (prOutCycle() != 0) {
        CloseOutput();
        return 1;
      }
    }
    CloseOutput();

    break;
  }
  case FST2LIST_DEBUG:
    break;
  case FULL:
    switch (traitAuto) {
    case SINGLE: {
      if (enableLoopCheck) {
        listOut = 0;
        exploreSubAuto(1); // mark loop path start nodes
        prSubGrapheCycle();
      }
      makeOfileName(ofNameTmp, 0, 0);
      if (strcmp(ofnameOnly, MAGIC_OUT_STDOUT) == 0) {
        foutput = U_STDOUT;
      }
      else {
        foutput = u_fopen(&vec, ofNameTmp, U_WRITE);
      }

      if (!foutput) {
        fatal_error("Cannot open file %s\n", ofNameTmp);
      }
      listOut = 1;
      exploreSubAuto(1);
      if (verboseMode) {
        char charBuffOut[1024];
        u_printf(
            " automaton %s : %d paths, %d path stopped by cycle, %d path overflows\n",
            getUtoChar(charBuffOut, a->graph_names[1]), totalPath,
            totalLoop, errPath);
        if (stopPath) {
          for (int inx = 1; inx <= a->number_of_graphs; inx++) {
            if (numOfIgnore[inx]) {
              u_printf(" Sub call [%s] %d\n", getUtoChar(
                  charBuffOut, a->graph_names[inx]),
                  numOfIgnore[inx]);
              numOfIgnore[inx] = 0;
            }
          }
        }
      }
      if (recursiveMode == SYMBOL) {
        if (prOutCycle() != 0) {
          CloseOutput();
          return 1;
        }
      }
      CloseOutput();
    }
      break;
    case MULTI: // the first graph has only the names of the initials graphs
    {
      U_FILE* listFile;
      unichar *wp;
      Transition *trans;
      strcpy(tmpchar, ofnameOnly);
      strcat(tmpchar, "lst");
      makeOfileName(ofNameTmp, tmpchar, ".txt");
      listFile = u_fopen(ASCII, ofNameTmp, U_WRITE);
      if (!(listFile)) {
        fatal_error("list file open error");
      }
      i = 0;

      for (trans = a->states[0]->transitions; trans != 0; trans = trans->next) {
        if (!(trans->tag_number & FILE_PATH_MARK)) {
          continue;
        }
        ignoreTable[trans->tag_number & SUB_ID_MASK] = 1;
        i++;
      }
      u_fprintf(listFile, " %d\n", i);
      for (trans = a->states[0]->transitions; trans != 0; trans = trans->next) {
        if (!(trans->tag_number & FILE_PATH_MARK)) {
          continue;
        }
        cleanCyclePath();

        wp = (unichar *) a->graph_names[trans->tag_number & SUB_ID_MASK];
        dp = tmpchar;

        while (*wp) {
          *dp++ = (char) (*wp & 0xff);
          wp++;
        }
        *dp++ = '\0';
        makeOfileName(ofNameTmp, tmpchar, 0);
        remove_path(ofNameTmp, ttpchar);
        u_fprintf(listFile, "%s\r\n", ttpchar);


        if (strcmp(ofnameOnly, MAGIC_OUT_STDOUT) == 0) {
          foutput = U_STDOUT;
        }
        else {
          foutput = u_fopen(&vec, ofNameTmp, U_WRITE);
        }

        if (!foutput) {
          fatal_error("Cannot open file %s\n", ofNameTmp);
        }
        if (enableLoopCheck) {
          listOut = 0; // output disable
          exploreSubAuto(trans->tag_number & SUB_ID_MASK);
          prSubGrapheCycle();
          CleanPathCounter();
        }
        listOut = 1; // output enable
        exploreSubAuto(trans->tag_number & SUB_ID_MASK);

        if (recursiveMode == SYMBOL) {
          if (prOutCycle() != 0) {
            CloseOutput();
            return 1;
          }
        }
        if (verboseMode) {
          char charBuffOut[1024];
          u_printf(" automaton %s: %d paths, %d path stopped by cycle, %d path overflows \n",getUtoChar(charBuffOut, a->graph_names[trans->tag_number & SUB_ID_MASK]), totalPath, totalLoop, errPath);

          if (stopPath) {
            for (int inx = 1; inx <= a->number_of_graphs; inx++) {
              if (numOfIgnore[inx]) {
                u_printf(" sub-call[%s] %d\n", getUtoChar(charBuffOut, a->graph_names[inx]),numOfIgnore[inx]);
                numOfIgnore[inx] = 0;
              }
            }
          }
        }

        CloseOutput();
      }
      u_fclose(listFile);
    }
      break;
    } // end switch 'traitauto'
  } // end switch 'display_control'
  return 0;
}

/**
 * explores all the sub graph calls in the automaton
 * starting at 'startAutoNo'
 */
int CFstApp::exploreSubAuto(int startAutoNo) {
  Transition startCallTr;
  //if(listOut) prCycleNode();
  startCallTr.tag_number = startAutoNo | FILE_PATH_MARK;
  startCallTr.state_number = 0;
  numberOfOutLine = 0; // reset output lines

  autoStackMap[0].tran = &startCallTr;
  int callSubGraphId = callIdentifyId(autoStackMap, 1);
  autoStackMap[0].autoId = callSubGraphId;

  calledAutoDepth = 0;
  calledAutoQ[calledAutoDepth].aId = callSubGraphId;
  calledAutoQ[calledAutoDepth].next = 0;
  pathTagQidx = 0;
  pathTagQ[pathTagQidx].autoNo = callSubGraphId;
  pathTagQ[pathTagQidx].stateNo = a->initial_states[startAutoNo];
  pathTagQ[pathTagQidx].tag = 0;
  pathTagQidx++;
  if (findCycleSubGraph(callSubGraphId, 1, a->initial_states[startAutoNo], 0) != 0) {
    return 1;
  }
  pathTagQidx--;
  if (pathTagQidx) {
    fatal_error("error in program");
  }
  return 0;
}

//
//  find cycle path by node and call
//
int CFstApp::findCycleSubGraph(int automatonNo, int autoDepth, int stateNo, int stateDepth) {
  int skipCnt = 0;
  int i;
  int tmp;
  int nextState;
  int callId;
  int scanner;
  //  prAutoStack(autoDepth);

  if (listOut && WasCycleNode(automatonNo, stateNo)) {
    pathTagQ[pathTagQidx - 1].stateNo |= LOOP_PATH_MARK;
  }
  if (pathTagQidx >= PATH_QUEUE_MAX) {
    pathTagQidx = PATH_QUEUE_MAX - 1;
    if (listOut) {
      errPath++;
      pathTagQ[pathTagQidx].stateNo = STOP_PATH_MARK;
      pathTagQ[pathTagQidx].tag = 0;
      pathTagQidx++;
      if (outWordsOfGraph(pathTagQidx) != 0) {
        return 1;
      }
      pathTagQidx--;
    } else {
      error("Warning: too many calls\n");
    }
    return 0;
  }
  if (IsCyclePath(stateDepth)) {
    if (recursiveMode == STOP) {
      return 0;
    }
    //    if(!listOut){
    //            a->state[stateNo]->controle |= LOOP_NODE_MARK;
    //    }
    return 0;
  }

  if (is_final_state(a->states[stateNo])) { // terminal node
    if (autoDepth != 1) { // check continue condition
      skipCnt = 0; // find next state
      for (i = calledAutoDepth; i >= 0; --i) {
        if (calledAutoQ[i].aId == -1) {
          skipCnt++;
        }
        else {
          if (skipCnt != 0) {
            skipCnt--;
          }
          else {
            break;
          }
        }
      }

      // ?
      if (i == 0) {
        error("unwanted state happened");
        return 1;
      }
      int tauto = calledAutoQ[i].aId;
      nextState = calledAutoQ[i].next;
      calledAutoDepth++;
      calledAutoQ[calledAutoDepth].aId = -1;
      calledAutoQ[calledAutoDepth].next = 0;

      pathTagQ[pathTagQidx].stateNo = nextState;
      pathTagQ[pathTagQidx].tag = 0;
      pathTagQ[pathTagQidx].autoNo = tauto;
      pathTagQidx++;
      if (findCycleSubGraph(tauto, autoDepth - 1, nextState, stateDepth + 1) != 0) {
        return 1;
      }
      pathTagQidx--;
      calledAutoDepth--;
    } else { // stop condition
      if (listOut) {
        totalPath++;
        pathTagQ[pathTagQidx].stateNo = STOP_PATH_MARK;
        pathTagQ[pathTagQidx].tag = 0;
        pathTagQ[pathTagQidx].autoNo = automatonNo;
        pathTagQidx++;
        if (outWordsOfGraph(pathTagQidx) != 0) {
          return 1;
        }
        pathTagQidx--;
      }
    }
  } // end if terminal node

  for (Transition *trans = a->states[stateNo]->transitions; trans != 0; trans = trans->next) {
    if (trans->tag_number & STOP_PATH_MARK) {
      if (listOut) {
        totalPath++;
        pathTagQ[pathTagQidx].autoNo = automatonNo;
        pathTagQ[pathTagQidx].stateNo = STOP_PATH_MARK;
        pathTagQ[pathTagQidx].tag = trans->tag_number & ~STOP_PATH_MARK;
        pathTagQidx++;
        if (outWordsOfGraph(pathTagQidx) != 0) {
          return 1;
        }
        pathTagQidx--;
      }
      continue;
    }
    if (display_control == GRAPH) {
      if (listOut) {
        pathTagQ[pathTagQidx].autoNo = automatonNo;
        pathTagQ[pathTagQidx].stateNo = trans->state_number;
        pathTagQ[pathTagQidx].tag = trans->tag_number;
        pathTagQidx++;
        if (findCycleSubGraph(automatonNo, autoDepth, trans->state_number, stateDepth + 1) != 0) {
          return 1;
        }
        pathTagQidx--;
      }
      continue;
    }
    if (trans->tag_number & FILE_PATH_MARK) { // handling sub graph call
      if (ignoreTable[trans->tag_number & SUB_ID_MASK]) {
        // find stop condition path
        if (listOut) {
          totalPath++;
          stopPath++;

          numOfIgnore[trans->tag_number & SUB_ID_MASK]++;

          pathTagQ[pathTagQidx].autoNo = automatonNo;
          pathTagQ[pathTagQidx].tag = trans->tag_number;
          pathTagQ[pathTagQidx].stateNo = STOP_PATH_MARK;
          pathTagQidx++;
          if (outWordsOfGraph(pathTagQidx) != 0) {
            return 1;
          }
          pathTagQidx--;
        }
        continue;
      }
      //
      //    find cycle call
      //
      tmp = trans->tag_number & SUB_ID_MASK;
      for (scanner = 0; scanner < autoDepth; scanner++) {
        if ((autoStackMap[scanner].tran->tag_number == trans->tag_number) && (trans->state_number == autoStackMap[scanner].tran->state_number)) {
          break;
        }
      }
      autoStackMap[autoDepth].tran = trans;
      if (scanner == autoDepth) {
        callId = callIdentifyId(autoStackMap, autoDepth + 1);
      } else { // find recursive call
        pathTagQ[pathTagQidx].tag = 0;
        pathTagQ[pathTagQidx].autoNo = autoStackMap[scanner].autoId;
        pathTagQ[pathTagQidx].stateNo = a->initial_states[tmp] | LOOP_PATH_MARK;

        ++pathTagQidx;
        if (!IsCyclePath(stateDepth)) {
          fatal_error("recursive find fail");
        }
        --pathTagQidx;
        continue;
      }
      pathTagQ[pathTagQidx].tag = 0;
      pathTagQ[pathTagQidx].autoNo = callId;
      pathTagQ[pathTagQidx].stateNo = a->initial_states[tmp];
      ++pathTagQidx;
      autoStackMap[autoDepth].autoId = callId;

      calledAutoDepth++;
      calledAutoQ[calledAutoDepth].aId = callId;
      calledAutoQ[calledAutoDepth].next = trans->state_number;
      if (findCycleSubGraph(callId, autoDepth + 1, a->initial_states[tmp], stateDepth + 1) != 0) {
        return 1;
      }
      --pathTagQidx;
      --calledAutoDepth;
      continue;
    }
    pathTagQ[pathTagQidx].stateNo = trans->state_number;
    pathTagQ[pathTagQidx].tag = trans->tag_number;
    pathTagQ[pathTagQidx].autoNo = automatonNo;
    ++pathTagQidx;
    if (findCycleSubGraph(automatonNo, autoDepth, trans->state_number, stateDepth + 1) != 0) {
      return 1;
    }
    pathTagQidx--;
  } // end for each transition
  return 0;
}

//
//  for debugging, display all stack
//
void CFstApp::CqueuePathPr(U_FILE *f) {
  int pidx = -1;
  int i;
  u_fprintf(f, "#");
  for (i = 0; i < pathTagQidx; i++) {
    if (pathTagQ[i].autoNo != pidx) { // skip the same value
      pidx = pathTagQ[i].autoNo;
      u_fprintf(f, "%S>", a->graph_names[pidx]);
    }
  }
}

unichar * uascToNum(unichar *uasc, int *val) {
  unichar *wp = uasc;
  int base = 10;
  int sum = 0;
  if ((*wp == (unichar) '0') && ((*(wp + 1) == (unichar) 'x') || (*(wp + 1)
      == (unichar) 'X'))) {
    base = 16;
    wp += 2;
  }
  do {
    if ((*wp >= (unichar) '0') && (*wp <= (unichar) '9')) {
      sum = sum * base + *wp - (unichar) '0';
      wp++;
    } else if ((base == 16) && (*wp >= (unichar) 'a') && (*wp
        <= (unichar) 'f')) {
      sum = sum * base + *wp - (unichar) 'a' + 10;
      wp++;
    } else if ((base == 16) && (*wp >= (unichar) 'A') && (*wp
        <= (unichar) 'F')) {
      sum = sum * base + *wp - (unichar) 'A' + 10;
      wp++;
    } else {
      break;
    }
  } while (*wp);
  *val = sum;
  return (wp);
}

/**
 * prints the current path
 */
int CFstApp::outWordsOfGraph(int depth) {
  int s;
  Fst2Tag Tag;
  unichar *sp;
  unichar *wp;
  unichar *ep;
  unichar *tp;
  unichar *chp;
  int indicateFirstUsed;
  int i;
  int markCtlChar, markPreCtlChar;
  depthDebug = pathTagQidx;
  EOutCnt = SOutCnt = 0;
  ePtrCnt = tPtrCnt = 0;
  unichar aaBuffer_for_getLabelNumber[64];

  //  fini = (tagQ[tagQidx - 1] & (FILE_PATH_MARK | LOOP_PATH_MARK)) ?
  //    tagQ[tagQidx -1 ]:0;
  //
  //  elimine the value signified repete
  //
  markCtlChar = markPreCtlChar = 0;
  indicateFirstUsed = 0;
  count_in_line = 0;
  //prAutoStackOnly();
  for (s = 0; s < pathTagQidx; s++) {
    EBuff[ePtrCnt] = TBuff[tPtrCnt] = 0;
    if (!pathTagQ[s].tag) {
      ep = tp = u_null_string;
    } else if (pathTagQ[s].tag & FILE_PATH_MARK) {
      ep = (display_control == GRAPH) ? (unichar *) a->graph_names[pathTagQ[s].tag & SUB_ID_MASK] : u_null_string;
      tp = u_null_string;
    } else {
      Tag = a->tags[pathTagQ[s].tag & SUB_ID_MASK];
      ep = (u_strcmp(Tag->input, u_epsilon_string)) ? 
              Tag->input : u_null_string;
      if (Tag->output != NULL) {
        tp = (u_strcmp(Tag->output, u_epsilon_string)) ? 
                Tag->output : u_null_string;
      } else {
        tp = u_null_string;
      }
    }
    //wprintf(L"{%d,%x,%x,%s,%s}",s,pathTagQ[s].stateNo,pathTagQ[s].tag,ep,tp);
    markCtlChar = 0;
    if (!(pathTagQ[s].stateNo & STOP_PATH_MARK) && !niveau_traite_mot && (*ep == '<')) {
      chp = ep + 1;
      while (*chp) {
        chp++;
      }
      --chp;
      if (*chp == (unichar) '>') {
        markCtlChar = 1;
        //               if(ePtrCnt || tPtrCnt) outOneWord(0);
        //               else if(control_char) outOneWord(0);
        //                   control_char = 1;
        //             while(*ep) EBuff[ePtrCnt++] = *ep++;
        //           while(*tp) TBuff[tPtrCnt++] = *tp++;
        //           continue;
      }

    }
    //wprintf(L"\n");

    if (pathTagQ[s].stateNo & LOOP_PATH_MARK) {
      if (recursiveMode == LABEL) {
        if (*ep || *tp) { // current
          if (tPtrCnt) {
            if (outOneWord(0) != 0) {
              return 1;
            }
          }
          if (markPreCtlChar && *ep) {
            if (outOneWord(0) != 0) {
              return 1;
            }
          }
          while (*ep)
            EBuff[ePtrCnt++] = *ep++;
          if (automateMode == TRANMODE) {
            while (*tp) {
              TBuff[tPtrCnt++] = *tp++;
            }
          }

          if (niveau_traite_mot) {
            if (ePtrCnt || tPtrCnt) {
              if (outOneWord(0) != 0) {
                return 1;
              }
            }
          }
          //                    else {
          //                  if(control_char) outOneWord(0);
          //                }

        }
        if (pathTagQ[s].stateNo & STOP_PATH_MARK) {
          sp = getLabelNumber(aaBuffer_for_getLabelNumber, depth,indicateFirstUsed, s, 0);
          if (outOneWord(sp) != 0) {
            return 1;
          }
          break;
        }
        sp = getLabelNumber(aaBuffer_for_getLabelNumber, s,indicateFirstUsed, s, 1);
        if (!indicateFirstUsed) { // first print out
          if (outOneWord(sp) != 0) {
            return 1;
          }
        } else {
          resetBuffs();
        }
        while (*sp)
          EOUTLINE[EOutCnt++] = *sp++;
        wp = entreGF;
        while (*wp)
          EOUTLINE[EOutCnt++] = *wp++;
        markPreCtlChar = markCtlChar;
        continue;
      } else if (recursiveMode == SYMBOL) { // SYMBOL
        if (tPtrCnt && niveau_traite_mot) {
          if (outOneWord(0) != 0) {
            return 1;
          }
        }
        wp = entreGO;
        while (*wp) {
          if (automateMode == TRANMODE) {
            TBuff[tPtrCnt++] = *wp;
          }
          EBuff[ePtrCnt++] = *wp;
          wp++;
        }
        while (*ep)
          EBuff[ePtrCnt++] = *ep++;
        EBuff[ePtrCnt++] = (unichar) '|';
        if (automateMode == TRANMODE) {
          while (*tp)
            TBuff[tPtrCnt++] = *tp++;
          TBuff[tPtrCnt++] = (unichar) '|';
        }
        struct cyclePathMark *h = headCyc;
        int findId = pathTagQ[s].stateNo & PATHID_MASK;
        while (h) {
          for (i = 0; i < h->pathCnt; i++) {
            if (h->pathTagQueue[i].stateNo == findId) {
              break;
            }
          }
          if (i != h->pathCnt) {
            if (automateMode == TRANMODE) {
              EBuff[ePtrCnt++] = (unichar) 'C';
              TBuff[tPtrCnt++] = (unichar) 'C';
              putInt(1, h->index);
              EBuff[ePtrCnt++] = (unichar) '|';
              TBuff[tPtrCnt++] = (unichar) '|';
            } else {
              EBuff[ePtrCnt++] = (unichar) 'C';
              putInt(0, h->index);
              EBuff[ePtrCnt++] = (unichar) '|';
            }
          }
          h = h->next;
        }
        if (automateMode == TRANMODE) {
          --tPtrCnt;
        }
        --ePtrCnt;
        wp = entreGF;
        while (*wp) {
          if (automateMode == TRANMODE) {
            TBuff[tPtrCnt++] = *wp;
          }
          EBuff[ePtrCnt++] = *wp;
          wp++;
        }
        if (outOneWord(0) != 0) {
          return 1;
        }

        continue;
      } else { // STOP
        if (outOneWord(0) != 0) {
          return 1;
        }
        if (!(pathTagQ[s].stateNo & STOP_PATH_MARK)) {
          // mark the stop
          wp = entreGO;
          while (*wp) {
            EOUTLINE[EOutCnt++] = *wp;
            if ((automateMode == TRANMODE) && (prMode
                == PR_SEPARATION)) {
              SOUTLINE[SOutCnt++] = *wp;
            }
            wp++;
          }
        }
        while (*ep) {
          EBuff[ePtrCnt++] = *ep++;
        }
        if (automateMode == TRANMODE) {
          while (*tp) {
            TBuff[tPtrCnt++] = *tp++;
          }
        }
        if (pathTagQ[s].stateNo & STOP_PATH_MARK) {
          wp = entreGF;
          while (*wp) {
            if (automateMode == TRANMODE) {
              TBuff[tPtrCnt++] = *wp;
              if (prMode == PR_SEPARATION) {
                EBuff[ePtrCnt++] = *wp;
              }
            } else {
              EBuff[ePtrCnt++] = *wp;
            }

            wp++;
          }
          if (outOneWord(u_null_string) != 0) {
            return 1;
          }
        } else {
          if (niveau_traite_mot) {
            if (ePtrCnt || tPtrCnt) {
              if (outOneWord(0) != 0) {
                return 1;
              }
            }
          } else {
            if (markPreCtlChar) {
              if (outOneWord(0) != 0) {
                return 1;
              }
            }
          }
        }
        markPreCtlChar = markCtlChar;
        continue;
      }
    }

    if (pathTagQ[s].stateNo & STOP_PATH_MARK) {
      //printf("stop %d\n",s);
      if (markPreCtlChar && markCtlChar) {
        if (outOneWord(0) != 0) {
          return 1;
        }
      }
      if ((automateMode == TRANMODE) && *tp) { // current
        if (tPtrCnt) {
          if (outOneWord(0) != 0) {
            return 1;
          }
        }
        while (*tp) {
          TBuff[tPtrCnt++] = *tp++;
        }
      }
      if (pathTagQ[s].tag & FILE_PATH_MARK) {
        if (outOneWord((unichar *) a->graph_names[pathTagQ[s].tag
            & SUB_ID_MASK]) != 0) {
          return 1;
        }
      } else {
        if (outOneWord(u_null_string) != 0) {
          return 1;
        }
      }
      break;
    }
    if (pathTagQ[s].tag & FILE_PATH_MARK) {
      if (tPtrCnt || (markPreCtlChar && *ep)) {
        if (outOneWord(0) != 0) {
          return 1;
        }
      }

      switch (display_control) {
      case GRAPH:
        EBuff[ePtrCnt++] = (unichar) '{';
        while (*ep)
          EBuff[ePtrCnt++] = *ep++;
        EBuff[ePtrCnt++] = (unichar) '}';
        if (outOneWord(0) != 0) {
          return 1;
        }
        break;
      case FST2LIST_DEBUG:
      case FULL:
        fatal_error("???");
      }
      markPreCtlChar = markCtlChar;
      continue;
    }

    // make a pair of (entre, sorti)
    if ((*ep == 0) && (*tp == 0)) {
      continue;
    }

    if (tPtrCnt || (markPreCtlChar && *ep)) {
      if (outOneWord(0) != 0) {
        return 1;
      }
    }
    while (*ep)
      EBuff[ePtrCnt++] = *ep++;
    if (automateMode == TRANMODE) {
      while (*tp) {
        TBuff[tPtrCnt++] = *tp++;
      }
    }
    if (niveau_traite_mot) {
      if (ePtrCnt || tPtrCnt) {
        if (outOneWord(0) != 0) {
          return 1;
        }
      }
    }
    //         else {
    //      if(control_char) outOneWord(0);
    //    }
    markPreCtlChar = markCtlChar;
  }
  return 0;
}

//
//
//
//

// TODO(jhondoe) Use UnitexGetOpt instead argc, argv
// FIXME(jhondoe) Use malloc to allocate chars' memory
// FIXME(jhondoe) Full of possible memory leaks: aa.saveEntre, wp2...
int main_Fst2List(int argc, char* const argv[]) {
  char* ofilename = 0;
  int iargIndex = 1;

  unichar changeStrTo[16][MAX_CHANGE_SYMBOL_SIZE];
  int changeStrToIdx;

  CFstApp aa;

  changeStrToIdx = 0;

  char* wp     = NULL;
  unichar* wp2 = NULL;
  unichar* wp3 = NULL;

  VersatileEncodingConfig vec = VEC_DEFAULT;
  bool only_verify_arguments = false;
  while (iargIndex < argc) {
    if (*argv[iargIndex] != '-')
      break;
    switch (argv[iargIndex][1]) {
    case 'f':
      iargIndex++;
      switch (argv[iargIndex][0]) {
        case 's':
          aa.prMode = PR_SEPARATION;
          break;
        case 'a':
          aa.prMode = PR_TOGETHER;
          break;
        default:
          error("Invalid arguments: rerun with --help\n");
          return USAGE_ERROR_CODE;
      }
      break;
    case 'd':
      aa.enableLoopCheck = false;
    case 'S':
      ofilename = new char[strlen(MAGIC_OUT_STDOUT) + 1];
      strcpy(ofilename, MAGIC_OUT_STDOUT);
      break;
    case 'o':
      iargIndex++;
      ofilename = new char[strlen(argv[iargIndex]) + 1];
      strcpy(ofilename, argv[iargIndex]);
      break;
    case 'l':
      iargIndex++;
      aa.outLineLimit = atoi(argv[iargIndex]);
      break;
    case 'i':
      iargIndex++; // stop the exploitation
      aa.arretExpoList(argv[iargIndex]);
      break;
    case 'I':
      iargIndex++; // stop the exploitation
      aa.arretExpoListFile(argv[iargIndex]);
      break;
    case 'p':
      iargIndex++;
      switch (argv[iargIndex][0]) {
      case 's':
        aa.display_control = GRAPH;
        break;
      case 'f':
        aa.display_control = FULL;
        break;
      case 'd':
        aa.display_control = FST2LIST_DEBUG;
        break;
      default:
        error("Invalid arguments: rerun with --help\n");
        return USAGE_ERROR_CODE;
      }
      break;
    case 'a':
    case 't':
      aa.automateMode = (argv[iargIndex][1] == 't') ? TRANMODE : AUTOMODE;
      iargIndex++;
      switch (argv[iargIndex][0]) {
      case 's':
        aa.traitAuto = SINGLE;
        break;
      case 'm':
        aa.traitAuto = MULTI;
        break;
      default:
        error("Invalid arguments: rerun with --help\n");
        return USAGE_ERROR_CODE;
      }
      break;
    case 'v':
      aa.verboseMode = 1;
      break;
    case 'm':
      aa.niveau_traite_mot = 0;
      break;
    case 'r':
      switch (argv[iargIndex][2]) {
      case 's':
        aa.recursiveMode = SYMBOL;
        break;
      case 'l':
        aa.recursiveMode = LABEL;
        break;
      case 'x':
        aa.recursiveMode = STOP;
        break;
      default:
        error("Invalid arguments: rerun with --help\n");
        return USAGE_ERROR_CODE;
      }
      iargIndex++;
      aa.saveEntre = new unichar[strlen(argv[iargIndex]) + 1];
      wp = argv[iargIndex];
      wp2 = aa.saveEntre;
      wp3 = 0;
      while (*wp) {
        if ((*wp < 0x20) || (*wp > 0x7e)) {
          error("Use a separator in ASC code\r\n");
          return DEFAULT_ERROR_CODE;
        }
        if (*wp == '\\')
          wp++;
        else if (*wp == ',')
          wp3 = wp2;
        if (*wp != '"')
          *wp2++ = (unichar) *wp++;
        else
          wp++;
      }
      *wp2 = 0;

      aa.entreGO = aa.saveEntre;
      if (wp3) {
        *wp3++ = 0;
        aa.entreGF = wp3;
      } else
        aa.entreGF = wp2;
      break;
    case 'c':
      iargIndex++;
      if (!changeStrToVal(changeStrToIdx, changeStrTo, argv[iargIndex])) {
        break;
      }
      error("Invalid arguments: rerun with --help\n");
      return USAGE_ERROR_CODE;
    case 's': {
      char cc = argv[iargIndex][2];
      iargIndex++;
      wp = argv[iargIndex];
      wp3 = 0;
      switch (cc) {
      case 0:
        wp2 = aa.sep1 = new unichar[strlen(wp) + 1];
        wp3 = 0;
        while (*wp) {
          if ((*wp < 0x20) || (*wp > 0x7e)) {
            error("Use a separator in ASCII code\r\n");
            return DEFAULT_ERROR_CODE;
          }
          switch (*wp) {
          case '\\':
            wp++;
            if (*wp == '\0') {
                // TODO(jhondoe) Put an error message here
              error("");
              return DEFAULT_ERROR_CODE;
            }
            if (*wp != '"')
              break;
          case '"':
            wp++;
            continue;
          case ',':
            wp3 = wp2;
            break;
          default:
            break;
          }
          *wp2++ = (unichar) *wp++;
        }
        *wp2 = 0;
        aa.sepL = aa.sep1;
        if (wp3) {
          *wp3++ = 0;
          aa.sepR = wp3;
        } else {
          aa.sepR = wp2;
        }

        break;
      case '0':
        wp2 = aa.saveSep = new unichar[strlen(wp) + 1];
        while (*wp) {
          if ((*wp < 0x20) || (*wp > 0x7e)) {
            error("Use a separator in ASC code\r\n");
            return DEFAULT_ERROR_CODE;
          }
          if (*wp == '\\') {
            wp++;
            if (*wp == '\0') {
                // TODO(jhondoe) Put an error message here
              error("");
              return DEFAULT_ERROR_CODE;
            }
          }
          if (*wp == '"')
            continue;
          *wp2++ = (unichar) *wp++;
        }
        *wp2 = 0;
        break;
      case 's':
        wp2 = aa.stopSignal = new unichar[strlen(wp) + 3];
        ;
        *wp2++ = (unichar) '<';
        while (*wp) {
          if (*wp == '"')
            continue;
          *wp2++ = (unichar) *wp++;
        }
        *wp2++ = (unichar) '>';
        *wp2 = 0;
        break;
      }
      break;
    }
    case 'k':
      iargIndex++;
      if (argv[iargIndex][0] == '\0') {
        error("Empty input_encoding argument\n");
        return USAGE_ERROR_CODE;
      }
      decode_reading_encoding_parameter(
          &(vec.mask_encoding_compatibility_input), argv[iargIndex]);
      break;
    case 'V': only_verify_arguments = true;
              break;
    case 'h': usage();
              return SUCCESS_RETURN_CODE;
    case 'q': {
      char* arg;
      if (argv[iargIndex][2] == '\0') {
        iargIndex++;
        arg = argv[iargIndex];
      } else {
        arg = &(argv[iargIndex][2]);
      }
      if (arg[0] == '\0') {
        error("Empty output_encoding argument\n");
        return USAGE_ERROR_CODE;
      }
      decode_writing_encoding_parameter(&(vec.encoding_output),
          &(vec.bom_output), arg);
      break;
    }
    default:
      error("Invalid arguments: rerun with --help\n");
      return USAGE_ERROR_CODE;
    }
    iargIndex++;
  }
  if (iargIndex != (argc - 1)) {
    error("Invalid arguments: rerun with --help\n");
    return USAGE_ERROR_CODE;
  }
  
  if (only_verify_arguments) {
    // freeing all allocated memory
    // TODO(jhondoe) free all allocated memory
    delete ofilename;
    return SUCCESS_RETURN_CODE;
  }
  
  aa.fileNameSet(argv[iargIndex], ofilename);
  aa.vec = vec;
  aa.getWordsFromGraph(changeStrToIdx, changeStrTo, argv[iargIndex]);
  delete ofilename;
  return SUCCESS_RETURN_CODE;
}// end main function

} // namespace unitex
