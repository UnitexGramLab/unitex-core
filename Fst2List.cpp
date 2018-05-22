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
          " -i sname : stop call of the exploration at this sub-graphe \"sname\"\r\n"
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
          " -ss \"stop\" : set \"str\" as the mark of stop exploration at \"<stop>\" \r\n"
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
  return charBuffOut;
}

enum ModeOut {
  PR_SEPARATION, PR_TOGETHER
}; // inputs separated from outputs vs. each input together with its output

enum printOutType {
  GRAPH, FULL, FST2LIST_DEBUG
};
enum initialType {
  SINGLE, MULTI
}; // 'multi' is used for graph with several starting states
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

/**
 * set one entry in dictionary of symbols for unicode characters
 * write unicode character in changeStrTo[changeStrToIdx][0]
 * write symbol in changeStrTo[changeStrToIdx][1]
 * return 0 if successful
 */
static int changeStrToVal(int &changeStrToIdx, unichar changeStrTo[][MAX_CHANGE_SYMBOL_SIZE], char *src) {
  char *wordPtr = src;

  int ptrLoc = 0;
  int i;

  i = 1;
  changeStrTo[changeStrToIdx][i++] = (unichar) '<';
  for (wordPtr = src; i < MAX_CHANGE_SYMBOL_SIZE && (*wordPtr); i++) {
    if (*wordPtr == (unichar) '=')
      break;
    changeStrTo[changeStrToIdx][i] = (unsigned short) *wordPtr++;
  }
  if (*wordPtr != (unichar) '=')
    return 1;
  if (i > (MAX_CHANGE_SYMBOL_SIZE - 2)) {
    u_printf("the name of the variable is too long %s", src);
    return 1;
  }
  changeStrTo[changeStrToIdx][i++] = (unichar) '>';
  changeStrTo[changeStrToIdx][i++] = (unichar) '\0';
  ptrLoc = i;
  if (!*wordPtr) {
    usage();
    return 1;
  }
  for (wordPtr++; i < MAX_CHANGE_SYMBOL_SIZE && (*wordPtr); i++) {
    changeStrTo[changeStrToIdx][i] = (unsigned short) *wordPtr++;
  }
  if (*wordPtr != (unichar) '\0') {
    return 1;
  }
  changeStrTo[changeStrToIdx][i++] = (unichar) '\0';
  uascToNum(&changeStrTo[changeStrToIdx][ptrLoc], &i);
  changeStrTo[changeStrToIdx][0] = (unsigned short) i;

  char charBuffOut[1024];
  u_printf("Change symbol %s --> %x\n", getUtoChar(charBuffOut,&changeStrTo[changeStrToIdx][1]),changeStrTo[changeStrToIdx][0]);

  changeStrToIdx++;
  return 0;
}
static unichar u_null_string[] = { (unichar) '\0', (unichar) '\0' };
static unichar u_epsilon_string[] = { (unichar) '<', (unichar) 'E',(unichar) '>', (unichar) '\0' };

static const char *StrMemLack = "allocation of memory for cycle data failed";

/* for stack of states, i.e. path */
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
  int listOut; // controls if output is enabled
  int verboseMode;
  int enableLoopCheck; // controls if the graph is explored to find loops
  //  int control_char; // control the output for control_chars <>
  #define  PATH_QUEUE_MAX 1024
  struct pathAndTag pathTagQ[PATH_QUEUE_MAX];
  int pathTagQidx;
  #define TAGQ_MAX 1024

  //
  // print out all paths in the .fst2
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

  // the stack of invocations of sub graphs
  struct stackAuto {
    int aId;    // automaton ID
    int next;   // next state
  } invocStack[2048];
  int invocStackIdx;  // invocation stack depth

  void printPathQ(U_FILE *f);

  int *ignoreTable;  // 1 where the automaton is ignored, else 0
  int *numOfIgnore;

  int outLineLimit;
  int numberOfOutLine;
  int count_in_line; // number of out per line

  int totalPath; // number of paths found in the automaton
  int totalLoop; // number of loops present in the automaton
  int stopPath;
  int errPath;

  presentCycleValue recursiveMode;
  printOutType display_control;
  initialType traitAuto; // single or multi initial state
  int niveau_traite_mot;
  int depthDebug;

  unichar *saveSep;     // separator between input and output of a box
  unichar *sepL,*sepR;  // parentheses for enclosing items
  unichar *sep1;        // delimiter introducing each item
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
   * builds the output file name
   * and puts it in '*des'
   */
  void buildOfileName(const char *fn, const char *ext, char *des) {
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

        invocStackIdx(0),

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

        autoStackMap(NULL), transitionListHead(NULL), transitionListTail(NULL),

        cycInfos(NULL),

        headCyc(0), cyclePathCnt(0), headCycNodes(0), cycNodeCnt(0),

        ePtrCnt(0), tPtrCnt(0), inBufferCnt(0), outBufferCnt(0),

        stopSubListIdx(0) {
    initCallIdMap();
  }
  ;
  ~CFstApp() {
    stopExploDel();
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
  void resetCounters() {
    totalPath = totalLoop = errPath = stopPath = 0;
  }

  void closeOutput() {
    if ((foutput != NULL) && (foutput != U_STDOUT)) {
      u_fclose(foutput);
    }
    foutput = NULL;
  }

  /**
   * Stack of invocations
   * Identifies each invocation with the transition
   * and a number identifying the calling automaton
   */
  struct callStackMapSt {
    Transition* tran;
    int autoId;
  }*autoStackMap;

  /**
   * list of lists of transitions
   * each list correspond to an automaton
   */
  struct transitionList {
    int cnt;
    Transition** trans;
    struct transitionList *next;
  }*transitionListHead, *transitionListTail;

  /**
   * compare *cmap and transitionListHead
   * return the automaton ID corresponding to the transitions in *cmap
   */
  int callIdentifyId(struct callStackMapSt *cmap, int count) {
    int id = 0;
    struct transitionList *callMapPtr;
    callMapPtr = transitionListHead;
    while (callMapPtr) {
      if (callMapPtr->cnt == count) {
        int i;
        // search for the automaton corresponding to the transitions in cmap
        for (i = 0; i < count; i++) {
          if (callMapPtr->trans[i] != cmap[i].tran) {
            break;
          }
        }
        // if *cmap corresponds to transitionListHead then
        // we have found the automaton ID
        if (i == count) {
          return id;
        }
      }
      id++;
      callMapPtr = callMapPtr->next;
    }
    // if the automaton is not yet in the transitionList we add it
    callMapPtr = new struct transitionList;
    callMapPtr->cnt = count;
    callMapPtr->next = 0;
    callMapPtr->trans = new Transition*[count];
    for (int i = 0; i < count; i++) {
      callMapPtr->trans[i] = cmap[i].tran;
    }

    if (transitionListTail) {
      transitionListTail->next = callMapPtr;
      transitionListTail = callMapPtr;
    } else {
      transitionListTail = transitionListHead = callMapPtr;
    }
    return id;
  }

  void initCallIdMap() {
    autoStackMap = new struct callStackMapSt[1024];
    transitionListHead = transitionListTail = 0;
  }

  void deleteCallIdMap() {
    delete[] autoStackMap;
    while (transitionListHead) {
      transitionListTail = transitionListHead;
      transitionListHead = transitionListHead->next;
      delete[] transitionListTail->trans;
      delete transitionListTail;
    }
  }

  //
  // for level
  //
  //
  //  save all cycle identify
  //
  /**
   *
   */
  struct cyclePathMark {
    int index; // number of identify
    struct pathAndTag *pathTagQueue;
    int pathCnt;
    int flag;
    struct cyclePathMark *next;
  };


  //  save all nodes which have the cycle path
  //  the path from initial to the node is used to identify the node
  //  in sub graph
  struct linkCycle {
    struct cyclePathMark *cyc;
    struct linkCycle *next;
  }*cycInfos;

  /**
   * structure to hold
   * cycle information
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
      for (int i = 0; i < pathTagQidx; i++) {
        error("%d : (%08x:%08x) : %08x\n", i, pathTagQ[i].autoNo, pathTagQ[i].stateNo, pathTagQ[i].tag);
      }

      printPathQ(U_STDERR);
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
    return (unichar *) aa;
  }

  /**
   * search for information about a cycle
   * used in case of exploration in debug mode
   */
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

  /**
   * return information about a cycle
   */
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
            return (*h);
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

    return (*h);
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

  /**
   * prints the cycle node
   */
  void prCycleNode() {
    struct cycleNodeId *cnode = headCycNodes;
    u_fprintf(foutput, "cycle Nodes %d\n", cyclePathCnt);
    while (cnode) {
      u_fprintf(foutput, "%d (%d:%d)\n", cnode->index, cnode->autoNo,
          cnode->stateNo);
      cnode = cnode->next;
    }
  }

  int wasCycleNode(int cauto, int cstate) {
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
   * return 1 if a cycle is found, else 0
   */
  int isCyclePath(int depth) {
    int scanner; 
    int curId = pathTagQ[pathTagQidx - 1].stateNo & PATHID_MASK;
    int curAutoId = pathTagQ[pathTagQidx - 1].autoNo;

    for (scanner = 0; scanner < pathTagQidx - 1; scanner++) {
      if (((pathTagQ[scanner].stateNo & PATHID_MASK) == curId) && (pathTagQ[scanner].autoNo == curAutoId)) { // find recursive path
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
        return 1;
      }
    }
    return 0;
  }

  unichar EBuff[128];
  unichar TBuff[128];
  int ePtrCnt;
  int tPtrCnt;
  unichar INPUTBUFFER[4096];  // buffer used to print the box inputs 
  unichar OUTPUTBUFFER[4096]; // buffer used to print the box outputs
  int inBufferCnt;            // buffer counter for box inputs
  int outBufferCnt;           // buffer counter for box outputs

  void resetBufferCounters() {
    ePtrCnt = tPtrCnt = inBufferCnt = outBufferCnt = 0;
  }

  /**
   * print the word
   * return 0 if we can't print it, else 1
   */
  int outOneWord(unichar *suffix) {
    int setOut;
    unichar *wordPtr;
    EBuff[ePtrCnt] = TBuff[tPtrCnt] = 0;
    if (!ePtrCnt && !tPtrCnt && !suffix) {
      return 0;
    }
    if (suffix) {
      setOut = 0;
      //printf("%d %d %d %d \n",ePtrCnt,tPtrCnt,*suffix,count_in_line);
      if (ePtrCnt || tPtrCnt || *suffix || (count_in_line == 0)) {
        setOut = 1;
        if (prMode == PR_SEPARATION) {
          wordPtr = sepL;
          while (*wordPtr) {
            INPUTBUFFER[inBufferCnt++] = *wordPtr;
            if (automateMode == TRANMODE) {
              OUTPUTBUFFER[outBufferCnt++] = *wordPtr;
            }
            wordPtr++;
          }
          for (int i = 0; i < ePtrCnt; i++) {
            INPUTBUFFER[inBufferCnt++] = EBuff[i];
	  }
          if (automateMode == TRANMODE) {
            for (int i = 0; i < tPtrCnt; i++) {
              OUTPUTBUFFER[outBufferCnt++] = TBuff[i];
            }
          }
          wordPtr = sepR;
          while (*wordPtr) {
            if (ePtrCnt) {
              INPUTBUFFER[inBufferCnt++] = *wordPtr;
            }
            if (automateMode == TRANMODE) {
              OUTPUTBUFFER[outBufferCnt++] = *wordPtr;
            }
            wordPtr++;
          }
        } else {
          wordPtr = sepL;
          while (*wordPtr) {
            INPUTBUFFER[inBufferCnt++] = *wordPtr++;
          }
          for (int i = 0; i < ePtrCnt; i++) {
            INPUTBUFFER[inBufferCnt++] = EBuff[i];
          }
          wordPtr = saveSep;
          while (*wordPtr) {
            INPUTBUFFER[inBufferCnt++] = *wordPtr++;
          }
          if (automateMode == TRANMODE) {
            for (int i = 0; i < tPtrCnt; i++) {
              INPUTBUFFER[inBufferCnt++] = TBuff[i];
            }
          }
          wordPtr = sepR;
          while (*wordPtr) {
            INPUTBUFFER[inBufferCnt++] = *wordPtr++;
          }
        }
      } // condition de out
      if ((recursiveMode == LABEL) && setOut) {
        if ((automateMode == TRANMODE) && (prMode == PR_SEPARATION)) {
          wordPtr = entreGO;
          while (*wordPtr) {
            OUTPUTBUFFER[outBufferCnt++] = *wordPtr++;
          }
          wordPtr = suffix;
          while (*wordPtr) {
            OUTPUTBUFFER[outBufferCnt++] = *wordPtr++;
          }
        } else {
          wordPtr = entreGO;
          while (*wordPtr) {
            INPUTBUFFER[inBufferCnt++] = *wordPtr++;
          }
          wordPtr = suffix;
          while (*wordPtr) {
            INPUTBUFFER[inBufferCnt++] = *wordPtr++;
          }
        }
      }
      INPUTBUFFER[inBufferCnt] = 0;
      OUTPUTBUFFER[outBufferCnt] = 0;
      u_fputs(INPUTBUFFER, foutput);
      if ((automateMode == TRANMODE) && outBufferCnt) {
        u_fprintf(foutput, "%S%S", saveSep, OUTPUTBUFFER);
      }
      if (display_control == FST2LIST_DEBUG) {
        printPathQ(foutput);
      }
      u_fprintf(foutput, "\n");
      numberOfOutLine++;
      inBufferCnt = outBufferCnt = 0;
    } else { // suffix == 0
      if (ePtrCnt || tPtrCnt) {
        if (prMode == PR_SEPARATION) {
          wordPtr = sepL;
          while (*wordPtr) {
            INPUTBUFFER[inBufferCnt++] = *wordPtr;
            if (automateMode == TRANMODE) {
              OUTPUTBUFFER[outBufferCnt++] = *wordPtr;
            }
            wordPtr++;
          }
          for (int i = 0; i < ePtrCnt; i++) {
            INPUTBUFFER[inBufferCnt++] = EBuff[i];
          }
          if (automateMode == TRANMODE) {
            for (int i = 0; i < tPtrCnt; i++) {
              OUTPUTBUFFER[outBufferCnt++] = TBuff[i];
            }
          }
          //        if(recursiveMode == LABEL){
          //          wordPtr = entreGO;while(*wordPtr)  INPUTBUFFER[inBufferCnt++] = *wordPtr++;
          //          }
          wordPtr = sepR;
          while (*wordPtr) {
            INPUTBUFFER[inBufferCnt++] = *wordPtr;
            if (automateMode == TRANMODE) {
              OUTPUTBUFFER[outBufferCnt++] = *wordPtr;
            }
            wordPtr++;
          }
        } else {
          wordPtr = sepL;
          while (*wordPtr) {
            INPUTBUFFER[inBufferCnt++] = *wordPtr++;
	  }
          for (int i = 0; i < ePtrCnt; i++) {
            INPUTBUFFER[inBufferCnt++] = EBuff[i];
	  }
          wordPtr = saveSep;
          while (*wordPtr) {
            INPUTBUFFER[inBufferCnt++] = *wordPtr++;
	  }
          if (automateMode == TRANMODE) {
            for (int i = 0; i < tPtrCnt; i++) {
              INPUTBUFFER[inBufferCnt++] = TBuff[i];
            }
          }
          if (recursiveMode == LABEL) {
            wordPtr = entreGO;
            while (*wordPtr) {
              INPUTBUFFER[inBufferCnt++] = *wordPtr++;
            }
          }
          wordPtr = sepR;
          while (*wordPtr) {
            INPUTBUFFER[inBufferCnt++] = *wordPtr++;
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
    unichar *wordPtr;
    if ((recursiveMode == LABEL) && !count_in_line) {
      wordPtr = sepL;
      while (*wordPtr)
        INPUTBUFFER[inBufferCnt++] = *wordPtr++;
      wordPtr = saveSep;
      while (*wordPtr)
        INPUTBUFFER[inBufferCnt++] = *wordPtr++;
      wordPtr = sepR;
      while (*wordPtr)
        INPUTBUFFER[inBufferCnt++] = *wordPtr++;
      wordPtr = entreGO;
      while (*wordPtr)
        INPUTBUFFER[inBufferCnt++] = *wordPtr++;
    }
    INPUTBUFFER[inBufferCnt++] = 0;
    u_fprintf(foutput, "%S\n", INPUTBUFFER);
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

  int printOutCycle() {
    struct cyclePathMark *h = headCyc;
    int i;
    unichar *wordPtr;
    //        unichar *wwordPtr;
    Fst2Tag Tag;
    while (h) {
      u_fprintf(foutput, "C%d%S", h->index, entreGO);
      ePtrCnt = tPtrCnt = 0;
      for (i = 0; i < h->pathCnt; i++) {
        //        putInt(0,h->pathTagQueue[i].path);
        Tag = a->tags[h->pathTagQueue[i].tag];
        wordPtr = (unichar *) Tag->input;

        if (u_strcmp(wordPtr, u_epsilon_string)) {
          //          wwordPtr = saveSep;while(*wwordPtr) EBuff[ePtrCnt++] = *wwordPtr++;
          while (*wordPtr) {
            EBuff[ePtrCnt++] = *wordPtr++;
          }
        }
        wordPtr = (unichar *) Tag->output;
        if ((automateMode == TRANMODE) && wordPtr && u_strcmp(wordPtr,
            u_epsilon_string)) {
          //            wwordPtr = saveSep;while(*wwordPtr) EBuff[ePtrCnt++] = *wwordPtr++;
          while (*wordPtr) {
            TBuff[tPtrCnt++] = *wordPtr++;
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

  int printOutCycleAtNode(int autoNum, int nodeNum) {
    struct cyclePathMark *h = headCyc;
    int i, st, ed;
    int tmp;
    unichar *wordPtr;
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
        wordPtr = (unichar *) Tag->input;
        if (u_strcmp(wordPtr, u_epsilon_string) && *wordPtr) {
          while (*wordPtr) {
            EBuff[ePtrCnt++] = *wordPtr++;
          }
        }
        wordPtr = (unichar *) Tag->output;
        if ((automateMode == TRANMODE) && wordPtr && u_strcmp(wordPtr,
            u_epsilon_string) && *wordPtr) {
          while (*wordPtr) {
            TBuff[tPtrCnt++] = *wordPtr++;
          }
        }
      }
      if (i == h->pathCnt) {
        if (outOneWord(u_null_string) != 0) {
          return 1;
        }
      } else
        resetBufferCounters();
      h = h->next;
    }
    return 0;
  }


  void printSubGraphCycle() {
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
  unichar *stopSubList[MAX_IGNORE_SUB_GRAPH];
  int stopSubListIdx;

  void stopExploList(char *src) {
    char *cp = src;
    unichar *wordPtr;
    if (stopSubListIdx == MAX_IGNORE_SUB_GRAPH) {
      u_printf("too many ignored sub-graph name ignore%s\n", src);
      return;
    }
    wordPtr = new unichar[strlen(src) + 1];
    stopSubList[stopSubListIdx++] = wordPtr;
    while (*cp) {
      *wordPtr++ = (unichar) (*cp++ & 0xff);
    }
    *wordPtr = (unichar) '\0';
    if (verboseMode) {
      char charBuffOut[1024];
      u_printf("IGNORE %s\n", getUtoChar(charBuffOut,
          stopSubList[stopSubListIdx - 1]));
    }
  }

  char fileLine[1024];

  void stopExploListFile(char *src) {
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

      if (stopSubListIdx == MAX_IGNORE_SUB_GRAPH) {
        u_printf("too many ignored sub-graph, name ignore %s\n", src);
        return;
      }
      stopSubList[stopSubListIdx] = new unichar[i];
      for (i = 0; EBuff[i]; i++) {
        stopSubList[stopSubListIdx][i] = EBuff[i];
      }
      stopSubList[stopSubListIdx][i] = 0;
      if (verboseMode) {
        char charBuffOut[1024];
        u_printf("IGNORE %s\n", getUtoChar(charBuffOut,stopSubList[stopSubListIdx]));
      }
      stopSubListIdx++;
    }
    u_fclose(uf);
  }

  void stopExploDel() {
    for (int i = 0; i < stopSubListIdx; i++){
      delete stopSubList[i];
    }
  }

  /**
   * prints the invocation stack,
   * the path stack and the automaton stack
   */
  void prAutoStack(int depStack) {
    int i;
    u_printf("===== invocStack\n");
    u_printf("i   aId  next_state\n");
    for (i = 0; i <= invocStackIdx; i++) {
      u_printf("%d :: %d :: %d\n", i, invocStack[i].aId,
          invocStack[i].next);
    }
    u_printf("===== stateQueue\n");
    u_printf("i autoNo  stateNo tag\n");
    for (i = 0; i < pathTagQidx; i++) {
      u_printf("%d (%d ::%d)%d\n", i, pathTagQ[i].autoNo,
          pathTagQ[i].stateNo, pathTagQ[i].tag);
    }
    u_printf("===== AutoStack\n"); 
    u_printf("i  autoId  stateNb trans_tag\n");
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

        invocStackIdx(0),

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

        autoStackMap(NULL), transitionListHead(NULL), transitionListTail(NULL),

        cycInfos(NULL),

        headCyc(0), cyclePathCnt(0), headCycNodes(0), cycNodeCnt(0),

        ePtrCnt(0), tPtrCnt(0), inBufferCnt(0), outBufferCnt(0),

        stopSubListIdx(0) {
    fatal_error("Unexpected copy constructor for CFstApp\n");
  }

  CFstApp& operator =(const CFstApp&) {
    fatal_error("Unexpected = operator for CFstApp\n");
    return *this;
  }
}; // end of fstApp


/**
 * load graph '*fname' in automaton 'a'
 * add marks to the graph
 */
void CFstApp::loadGraph(int& changeStrToIdx, unichar changeStrTo[][MAX_CHANGE_SYMBOL_SIZE], char *fname) {
  int i_1, j_1;
  Transition *transPtr;

  a = load_abstract_fst2(&vec, fname, 1, &fst2_free);
  if (a == NULL) {
    fatal_error("Cannot load graph file %s\n", fname);
  }

  // mark the automaton transitions that invoke subgraphs
  for (i_1 = 0; i_1 < a->number_of_states; i_1++) {
    transPtr = a->states[i_1]->transitions;
    if (a->states[i_1]->control & 0x80) {
      fatal_error("Not null control bit");
    }
    a->states[i_1]->control &= 0x7f; // clean to mark recursive. 0x7f=0b0111_1111
    while (transPtr) {
      if (transPtr->tag_number < 0) {   // transition invokes subgraph
        transPtr->tag_number = FILE_PATH_MARK | -transPtr->tag_number;
      }
      transPtr = transPtr->next;
    }
  }

  ignoreTable = new int[a->number_of_graphs + 1];
  numOfIgnore = new int[a->number_of_graphs + 1];
  for (i_1 = 1; i_1 <= a->number_of_graphs; i_1++) {
    ignoreTable[i_1] = 0;
    numOfIgnore[i_1] = 0;
  }

  if (stopSubListIdx) {   // set table of ignored graphs
    for (i_1 = 0; i_1 < stopSubListIdx; i_1++) {
      for (j_1 = 1; j_1 <= a->number_of_graphs; j_1++) {
        if (!u_strcmp((unichar *) a->graph_names[j_1], stopSubList[i_1])) {
          break;
        }
      }
      if (j_1 > a->number_of_graphs) {
        char charBuffOut[1024];
        u_printf("Warning : the sub graph doesn't exist %s\n", getUtoChar(charBuffOut, stopSubList[i_1]));
        continue;
      }
      char charBuffOut[1024];
      u_printf("%s %d graph ignore the exploration\n", getUtoChar(charBuffOut, a->graph_names[j_1]), j_1);

      ignoreTable[j_1] = 1;
    }
  }
  if (stopSignal) {    // mark all transitions that have stopping input
    for (i_1 = 0; i_1 < a->number_of_tags; i_1++) {
      if (u_strcmp((unichar *) a->tags[i_1]->input, stopSignal)) {
        continue;
      }
      for (j_1 = 0; j_1 < a->number_of_states; j_1++) {
        transPtr = a->states[j_1]->transitions;
        while (transPtr) {
          if (transPtr->tag_number == i_1) {
            transPtr->tag_number |= STOP_PATH_MARK;
          }
          transPtr = transPtr->next;
        }
      }
    }
  }
  if (changeStrToIdx) {   // translate symbols into unicode characters
    unichar *wordPtr;
    int i, j, k, l, m;
    unichar temp[256];
    for (i = 0; i < a->number_of_tags; i++) {
      wordPtr = (unichar *) a->tags[i]->input;
      for (j = 0; wordPtr[j]; j++) {
        temp[j] = wordPtr[j];
      }
      temp[j] = 0;
      for (j = 0; j < changeStrToIdx; j++) {
        wordPtr = changeStrTo[j] + 1;
        for (k = 0; temp[k]; k++) {
          for (l = 0; wordPtr[l]; l++) {
            if (!temp[k + l] || (temp[k + l] != wordPtr[l])) {
              break;
            }
          }
          if (wordPtr[l]) {
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
      wordPtr = (unichar *) a->tags[i]->input;
      if (u_strcmp(wordPtr, temp)) {
        char charBuffOut1[1024];
        char charBuffOut2[1024];
        u_printf("%dth index, %s==>%s\n", i, getUtoChar(charBuffOut1,wordPtr), getUtoChar(charBuffOut2, temp));
        for (j = 0; temp[j]; j++) {
          *wordPtr++ = temp[j];
        }
        *wordPtr = 0;
      }
    }
  }
}

/**
 * the main function to explore the automaton and print paths
 */
int CFstApp::getWordsFromGraph(int &changeStrToIdx, unichar changeStrTo[][MAX_CHANGE_SYMBOL_SIZE], char *fname) {
  int i;
  char *dp;
  char ofNameTmp[1024];
  char tmpchar[1024];
  char ttpchar[1024];
  // load fst2 file
  loadGraph(changeStrToIdx, changeStrTo, fname);
  resetCounters();
  ofNameTmp[0] = 0;
  switch (display_control) {
  case GRAPH: {    // explore each graph separately
    if (enableLoopCheck) {
      listOut = 0;
      niveau_traite_mot = 1;
      exploreSubAuto(1); // mark loop path start nodes
      printSubGraphCycle();
    }

    if (recursiveMode == LABEL) {
      error("warning:ignore the option -rl\r\n");
    }
    recursiveMode = SYMBOL;
    strcpy(tmpchar, ofnameOnly);
    strcat(tmpchar, "autolst");
    buildOfileName(tmpchar, ".txt",ofNameTmp);

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
      u_fprintf(foutput, "[%d th automaton %S]\n", i, a->graph_names[i]);
      //printf("[%d th automata %s]\n",i,getUtoChar(a->nom_graphe[i]));

      if (exploreSubAuto(i) != 0) {
        closeOutput();
        return 1;
      }

      if (printOutCycleAtNode(i, 0) != 0) {
        closeOutput();
        return 1;
      }
      u_fprintf(
          foutput,
          " automaton %S, %d paths, %d path stopped by cycle, %d path overflows\n",
          a->graph_names[i], totalPath, totalLoop, errPath);
      resetCounters();
    }

    if (recursiveMode == SYMBOL) { // recursiveMode can only be SYMBOL here
      if (printOutCycle() != 0) {
        closeOutput();
        return 1;
      }
    }
    closeOutput();

    break;
  }
  case FST2LIST_DEBUG:
    break;
  case FULL:
    switch (traitAuto) {
    case SINGLE: {
      // check for loops before writing the result
      if (enableLoopCheck) {
        listOut = 0;
        exploreSubAuto(1); // mark loop path start nodes
        printSubGraphCycle();
      }
      buildOfileName(0, 0, ofNameTmp);
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
        if (printOutCycle() != 0) {
          closeOutput();
          return 1;
        }
      }
      closeOutput();
    }
      break;
    case MULTI: // the first graph has only the names of the initial graphs
    {
      U_FILE* listFile;
      unichar *wordPtr;
      Transition *trans;
      strcpy(tmpchar, ofnameOnly);
      strcat(tmpchar, "lst");
      buildOfileName(tmpchar, ".txt", ofNameTmp);
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

        wordPtr = (unichar *) a->graph_names[trans->tag_number & SUB_ID_MASK];
        dp = tmpchar;

        while (*wordPtr) {
          *dp++ = (char) (*wordPtr & 0xff);
          wordPtr++;
        }
        *dp++ = '\0';
        buildOfileName(tmpchar, 0, ofNameTmp);
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
          printSubGraphCycle();
          resetCounters();
        }
        listOut = 1; // output enable
        exploreSubAuto(trans->tag_number & SUB_ID_MASK);

        if (recursiveMode == SYMBOL) {
          if (printOutCycle() != 0) {
            closeOutput();
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

        closeOutput();
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
  numberOfOutLine = 0; // reset output line counter

  autoStackMap[0].tran = &startCallTr;
  int callSubGraphId = callIdentifyId(autoStackMap, 1);
  autoStackMap[0].autoId = callSubGraphId;

  invocStackIdx = 0;
  invocStack[invocStackIdx].aId = callSubGraphId;
  invocStack[invocStackIdx].next = 0;
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

/**
 *  find cycle call
 */
int CFstApp::findCycleSubGraph(int automatonNo, int autoDepth, int stateNo, int stateDepth) {
  int skipCnt = 0;
  int tmp;
  int nextState;
  int callId;
  //prAutoStack(autoDepth);

  if (listOut && wasCycleNode(automatonNo, stateNo)) {
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
  if (isCyclePath(stateDepth)) {
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
      int i;
      for (i = invocStackIdx; i >= 0; --i) {
        if (invocStack[i].aId == -1) {
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

      if (i == 0) {
        error("unwanted state happened");
        return 1;
      }
      int tauto = invocStack[i].aId;
      nextState = invocStack[i].next;
      invocStackIdx++;
      invocStack[invocStackIdx].aId = -1;
      invocStack[invocStackIdx].next = 0;

      pathTagQ[pathTagQidx].stateNo = nextState;
      pathTagQ[pathTagQidx].tag = 0;
      pathTagQ[pathTagQidx].autoNo = tauto;
      pathTagQidx++;
      if (findCycleSubGraph(tauto, autoDepth - 1, nextState, stateDepth + 1) != 0) {
        return 1;
      }
      pathTagQidx--;
      invocStackIdx--;
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
      //    find cycle of calls
      //
      tmp = trans->tag_number & SUB_ID_MASK;

      int scanner;
      for (scanner = 0; scanner < autoDepth; scanner++) {
        if ( autoStackMap[scanner].tran->tag_number == trans->tag_number ) {
          break;
        }
      }
      autoStackMap[autoDepth].tran = trans; // add the transition to the stack
      if (scanner == autoDepth) { // didn't find a recursive call
        callId = callIdentifyId(autoStackMap, autoDepth + 1);
      } else { // found a recursive call
        pathTagQ[pathTagQidx].tag = 0;
        pathTagQ[pathTagQidx].autoNo = autoStackMap[scanner].autoId;
        pathTagQ[pathTagQidx].stateNo = a->initial_states[tmp] | LOOP_PATH_MARK;

        ++pathTagQidx;
        if (!isCyclePath(stateDepth)) { 
          // no cycle was found we identify the automaton
          // and continue the exploration
          callId = callIdentifyId(autoStackMap, autoDepth + 1);
          --pathTagQidx;
          goto noCycle;
          // previous error
          //fatal_error("failed to find the recursive call\n");
        }
        --pathTagQidx;
        continue;
      }
      noCycle:
      pathTagQ[pathTagQidx].tag = 0;
      pathTagQ[pathTagQidx].autoNo = callId;
      pathTagQ[pathTagQidx].stateNo = a->initial_states[tmp];
      ++pathTagQidx;
      autoStackMap[autoDepth].autoId = callId;

      invocStackIdx++;
      invocStack[invocStackIdx].aId = callId;
      invocStack[invocStackIdx].next = trans->state_number;
      if (findCycleSubGraph(callId, autoDepth + 1, a->initial_states[tmp], stateDepth + 1) != 0) {
        return 1;
      }
      --pathTagQidx;
      --invocStackIdx;
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
void CFstApp::printPathQ(U_FILE *f) {
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

/**
 * takes a number (decimal or hex) in char and puts its value in 'val'
 */
unichar * uascToNum(unichar *uasc, int *val) {
  unichar *wordPtr = uasc;
  int base = 10;
  int sum = 0;

  // if number is hexadecimal
  if ((*wordPtr == (unichar) '0') && ((*(wordPtr + 1) == (unichar) 'x') || (*(wordPtr + 1)
      == (unichar) 'X'))) {
    base = 16;
    wordPtr += 2;
  }
  do {
    if ((*wordPtr >= (unichar) '0') && (*wordPtr <= (unichar) '9')) {
      sum = sum * base + *wordPtr - (unichar) '0';
      wordPtr++;
    } else if ((base == 16) && (*wordPtr >= (unichar) 'a') && (*wordPtr
        <= (unichar) 'f')) {
      sum = sum * base + *wordPtr - (unichar) 'a' + 10;
      wordPtr++;
    } else if ((base == 16) && (*wordPtr >= (unichar) 'A') && (*wordPtr
        <= (unichar) 'F')) {
      sum = sum * base + *wordPtr - (unichar) 'A' + 10;
      wordPtr++;
    } else {
      break;
    }
  } while (*wordPtr);
  *val = sum;
  return wordPtr;
}

/**
 * prints the current path
 */
int CFstApp::outWordsOfGraph(int depth) {
  int s;
  Fst2Tag Tag;
  unichar *sp;
  unichar *wordPtr;
  unichar *ep;
  unichar *tp;
  unichar *chp;
  int indicateFirstUsed;
  int i;
  int markCtlChar, markPreCtlChar;
  depthDebug = pathTagQidx;
  inBufferCnt = outBufferCnt = 0;
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
    //wordPtrrintf(L"{%d,%x,%x,%s,%s}",s,pathTagQ[s].stateNo,pathTagQ[s].tag,ep,tp);
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
    //wordPtrrintf(L"\n");

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
          resetBufferCounters();
        }
        while (*sp)
          INPUTBUFFER[inBufferCnt++] = *sp++;
        wordPtr = entreGF;
        while (*wordPtr)
          INPUTBUFFER[inBufferCnt++] = *wordPtr++;
        markPreCtlChar = markCtlChar;
        continue;
      } else if (recursiveMode == SYMBOL) { // SYMBOL
        if (tPtrCnt && niveau_traite_mot) {
          if (outOneWord(0) != 0) {
            return 1;
          }
        }
        wordPtr = entreGO;
        while (*wordPtr) {
          if (automateMode == TRANMODE) {
            TBuff[tPtrCnt++] = *wordPtr;
          }
          EBuff[ePtrCnt++] = *wordPtr;
          wordPtr++;
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
        wordPtr = entreGF;
        while (*wordPtr) {
          if (automateMode == TRANMODE) {
            TBuff[tPtrCnt++] = *wordPtr;
          }
          EBuff[ePtrCnt++] = *wordPtr;
          wordPtr++;
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
          wordPtr = entreGO;
          while (*wordPtr) {
            INPUTBUFFER[inBufferCnt++] = *wordPtr;
            if ((automateMode == TRANMODE) && (prMode
                == PR_SEPARATION)) {
              OUTPUTBUFFER[outBufferCnt++] = *wordPtr;
            }
            wordPtr++;
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
          wordPtr = entreGF;
          while (*wordPtr) {
            if (automateMode == TRANMODE) {
              TBuff[tPtrCnt++] = *wordPtr;
              if (prMode == PR_SEPARATION) {
                EBuff[ePtrCnt++] = *wordPtr;
              }
            } else {
              EBuff[ePtrCnt++] = *wordPtr;
            }

            wordPtr++;
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

    // make a pair of (input, output)
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
// FIXME(jhondoe) Full of possible memory leaks: aa.saveEntre, wordPtr2...
int main_Fst2List(int argc, char* const argv[]) {
  char* ofilename = 0;
  int iargIndex = 1;

  unichar changeStrTo[16][MAX_CHANGE_SYMBOL_SIZE];
  int changeStrToIdx;

  CFstApp aa;

  changeStrToIdx = 0;

  char* wordPtr     = NULL;
  unichar* wordPtr2 = NULL;
  unichar* wordPtr3 = NULL;

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
      iargIndex++; // stop the exploration
      aa.stopExploList(argv[iargIndex]);
      break;
    case 'I':
      iargIndex++; // stop the exploration
      aa.stopExploListFile(argv[iargIndex]);
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
      wordPtr = argv[iargIndex];
      wordPtr2 = aa.saveEntre;
      wordPtr3 = 0;
      while (*wordPtr) {
        if ((*wordPtr < 0x20) || (*wordPtr > 0x7e)) {
          error("Use a separator in ASC code\r\n");
          return DEFAULT_ERROR_CODE;
        }
        if (*wordPtr == '\\')
          wordPtr++;
        else if (*wordPtr == ',')
          wordPtr3 = wordPtr2;
        if (*wordPtr != '"')
          *wordPtr2++ = (unichar) *wordPtr++;
        else
          wordPtr++;
      }
      *wordPtr2 = 0;

      aa.entreGO = aa.saveEntre;
      if (wordPtr3) {
        *wordPtr3++ = 0;
        aa.entreGF = wordPtr3;
      } else
        aa.entreGF = wordPtr2;
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
      wordPtr = argv[iargIndex];
      wordPtr3 = 0;
      switch (cc) {
      case 0:
        wordPtr2 = aa.sep1 = new unichar[strlen(wordPtr) + 1];
        wordPtr3 = 0;
        while (*wordPtr) {
          if ((*wordPtr < 0x20) || (*wordPtr > 0x7e)) {
            error("Use a separator in ASCII code\r\n");
            return DEFAULT_ERROR_CODE;
          }
          switch (*wordPtr) {
          case '\\':
            wordPtr++;
            if (*wordPtr == '\0') {
                // TODO(jhondoe) Put an error message here
              error("");
              return DEFAULT_ERROR_CODE;
            }
            if (*wordPtr != '"')
              break;
          case '"':
            wordPtr++;
            continue;
          case ',':
            wordPtr3 = wordPtr2;
            break;
          default:
            break;
          }
          *wordPtr2++ = (unichar) *wordPtr++;
        }
        *wordPtr2 = 0;
        aa.sepL = aa.sep1;
        if (wordPtr3) {
          *wordPtr3++ = 0;
          aa.sepR = wordPtr3;
        } else {
          aa.sepR = wordPtr2;
        }

        break;
      case '0':
        wordPtr2 = aa.saveSep = new unichar[strlen(wordPtr) + 1];
        while (*wordPtr) {
          if ((*wordPtr < 0x20) || (*wordPtr > 0x7e)) {
            error("Use a separator in ASC code\r\n");
            return DEFAULT_ERROR_CODE;
          }
          if (*wordPtr == '\\') {
            wordPtr++;
            if (*wordPtr == '\0') {
                // TODO(jhondoe) Put an error message here
              error("");
              return DEFAULT_ERROR_CODE;
            }
          }
          if (*wordPtr == '"')
            continue;
          *wordPtr2++ = (unichar) *wordPtr++;
        }
        *wordPtr2 = 0;
        break;
      case 's':
        wordPtr2 = aa.stopSignal = new unichar[strlen(wordPtr) + 3];
        ;
        *wordPtr2++ = (unichar) '<';
        while (*wordPtr) {
          if (*wordPtr == '"')
            continue;
          *wordPtr2++ = (unichar) *wordPtr++;
        }
        *wordPtr2++ = (unichar) '>';
        *wordPtr2 = 0;
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
