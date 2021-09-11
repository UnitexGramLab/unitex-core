/**
 * Unitex
 *
 * Copyright (C) 2001-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include <stdio.h>

#include <locale.h>
#include "Unicode.h"
#include "Fst2.h"
#include "AbstractFst2Load.h"
#include "Alphabet.h"
#include "Copyright.h"
#include "File.h"
#include "Error.h"
#include "Transitions.h"
#include "UnitexGetOpt.h"

#include "TransductionStack.h"
#include "LocateTfst_lib.h"
#include "LocatePattern.h"
#include "MorphologicalLocate.h"
#include "Korean.h"
#include "Dico.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char* usage_Fst2List =
    "Usage:\n"
        "Fst2List [-o <file>][-p (s|f|d)][-(a|t) (s|m)] [-m] [-d] [-K] [-E <path>] [-f (s|a)] [-g <str>] [-Q <str>][-s <str>] [-r (s|l|x) <str>] [-l <line#>] [-i <subgraphname>]... [-c SS=<0xXXXX>]... [-D <file>]... <fname>\r\n"

        "<fname>: input file name with extension \".fst2\"\r\n"
        "-S, --print: display result on standard output. Exclusive with -o\r\n"
        "-o <file>, --output <file>: if this option and -S are not used, save paths in \"<file>lst.txt\"\r\n"
        "-(a|t) (s|m), --(ignore_outputs|allow_outputs) (s|m)\r\n"
        "    a: ignore grammars outputs (default)\r\n"
        "    t: take into accout the grammars outputs\r\n"
        "    s: the grammar has only one initial state (default)\r\n"
        "    m: the grammar has several initial states. This mode is useful in Korean\r\n"
        "-l <line>, --limit <line>: maximum number of lines to be printed in the output file\r\n"
        "-i <subgraphname>..., --stop_subgraph <subgraphname>... \r\n"
        "    indicates that the recursive exploration must end when the program enters in graph <subgraphname>. \r\n"
        "    This parameter can be used several times in order to specify several stop graphs\r\n"
        "-p (s|f|d), --paths_print_mode (s|f|d)\r\n"
        "    s: displays paths graph by graph \r\n"
        "    f: (default) displays global paths;\r\n"
        "    d: displays global paths with information on nested graph calls\r\n"
        "-c <SS>=<0xXXXX>...: replaces symbol <SS> when it appears between angle brackets \r\n"
        "    by the Unicode character whose hexadecimal number is <0xXXXX>\r\n"
        "-s <L[,R]>: specifies the left (L) and right (R) delimiters that will enclose items. By default, no delimiters are specified\r\n"
        "-g <str>: if the program must take outputs into account (-t), this parameter specifies\r\n"
        "    the sequence <str> that will be inserted between input and output. By default, there is no separator.\r\n"
        "-f (a|s): if the program must take outputs into account (-t), this parameter specifies the format\r\n"
        "    of the lines that will be generated: in0 in1 out0 out1 (s) or in0 out0 in1 out1 (a). The default value is s\r\n"
        "    default value is 's'\r\n"
        "-Q <stop>: set <stop> as the mark of stop exploration at \"<stop>\". The default value is null.\r\n"
        "-m, --word_mode: mode special for description with alphabet\r\n"
        "-d, --disable_loop_check: faster execution at the cost of information about loops\r\n"
        "-v, --verbose: prints information during the process\r\n"
        "-r (s|l|x) <L[,R]>: enclose loops in L and R strings as in (c0|...|cn) by Lc0|..|cnR : default null\r\n"
        "-P <file>, generate dictionary file\r\n"
        "-K, indicates that the <fname> argument is in Korean\r\n"
        "-D <file>, morphological dictionary file to load, <file> must have the extension \".bin\"\r\n"
        "-E <path>/--elg_extensions_path=<path>: uses ELGs extensions directory X instead of App/elg\n"
        "-V, --only_verify_arguments: only verify arguments syntax and exit\r\n"
        "-h, --help: display this help and exit";

static void usage() {
  display_copyright_notice();
  u_printf(usage_Fst2List);
}

static char *getUtoChar(char charBuffOut[], unichar *s) {
  int i;
  for (i = 0; (i < 1024) && s[i]; i++){
    charBuffOut[i] = (char) s[i];
  }
  charBuffOut[i] = 0;
  return charBuffOut;
}

enum ModeOut {
  PR_SEPARATION, PR_TOGETHER
}; // inputs separated from outputs vs. each input together with its output

enum GrammarMode {
  NONE, MERGE, REPLACE
}; // no grammar mode vs outputs left inserted vs inputs replaced by outputs

enum printOutType {
  GRAPH, FULL, FST2LIST_DEBUG
}; // we either print each subgraph independently or whole automaton recursively
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

/**
 * The marks are applied directly to states and transitions on the loaded automaton
 */
// mark states and transitions to indicate call to a subgraph
#define SUBGRAPH_PATH_MARK  0x10000000 // equals 268435456
// mark states to indicate a loop
#define LOOP_PATH_MARK  0x20000000 // equals 536870912
// mark states and transitions to indicate end of a path
#define STOP_PATH_MARK  0x40000000 // equals 1073741824
#define LOOP_NODE_MARK  0x80      // equals 128

// mask STOP_PATH_MARK and LOOP_PATH_MARK
#define PATHID_MASK    0x1FFFFFFF // equals 536870911
// mask STOP_PATH_MARK, LOOP_PATH_MARK and SUBGRAPH_PATH_MARK
#define SUB_ID_MASK    0x0FFFFFFF // equals 268435455
// unused mask
//#define CTL_MASK       0xE0000000    // equals 3758096384

#define DIS_LINE_LIMIT_MAX  4096
#define MAX_CHANGE_SYMBOL_SIZE 32
#define MAGIC_OUT_STDOUT "<WRITE_U_STDOUT>"

#define NB_LITTERAL_MASKS 14
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
    if (*wordPtr == (unichar) '='){
      break;
    }
    changeStrTo[changeStrToIdx][i] = (unsigned short) *wordPtr++;
  }
  if (*wordPtr != (unichar) '='){
    return 1;
  }
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
  u_printf("Change symbol %s --> %x\n", getUtoChar(charBuffOut,
        &changeStrTo[changeStrToIdx][1]),changeStrTo[changeStrToIdx][0]);

  changeStrToIdx++;
  return 0;
}
static unichar u_null_string[] = { (unichar) '\0', (unichar) '\0' };
static unichar u_epsilon_string[] = { (unichar) '<', (unichar) 'E',(unichar) '>', (unichar) '\0' };

static const char *StrMemLack = "allocation of memory for cycle data failed";

/* stack of states, i.e. path */
struct pathStack_t {
  // identifies a snapshot of the call stack (i.e. autoCallStack) at this given point
  int stackStateID; 
  int stateNo;      // state's number in fst2 file
  int tag;          // tag's number in fst2 file
};



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
  #define  PATH_STACK_MAX 1024
  // history of states explored in the current path
  struct pathStack_t pathStack[PATH_STACK_MAX];
  int pathIdx;
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
  int exploreSubgraphRecursively(int stackStateID, int autodep, int testState, int depthState);
  int outWordsOfGraph(int depth);
  int checkAutoCallStack(); // unused function

  // Stack of invocations of subgraphs
  // and of exits from subgraphs
  // History of invocations and exits for the current path
  // Items are popped only when backtracking to explore the next path
  struct {
    int stackStateID;    // if invocation, identifies a snapshot of autoCallStack
                         // if exit, equals -1
    int targetState; 
  } invocStack[2048];
  int invocStackIdx;  // invocation stack depth

  void printPathNames(U_FILE *f);
  void setGrammarMode(char* fst2_filename);

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
  // either explore each subgraph independently or all the automaton recursively
  printOutType display_control; 
  initialType traitAuto; // single or multi initial state
  GrammarMode grammarMode;
  int wordMode;
  int depthDebug;

  unichar *saveSep;     // separator between input and output of a box
  unichar *sepL,*sepR;  // parentheses for enclosing items
  unichar *sep1;        // delimiter introducing each item
  unichar *stopSignal;
  unichar *saveEntre, *openingQuote, *closingQuote;
  VersatileEncodingConfig vec;
  char ofdirName[1024];
  char ofExt[16];
  char ofnameOnly[512];        // output file name
  char defaultIgnoreName[512]; // input file name
  bool inMorphoMode;  // true if the current state is in morphological mode
  bool isKorean;  // true if the graph is in korean
  struct locate_parameters* p;
  int morphDicCnt;  // number of dic to explore when a lexical mask is encountered
  bool isMdg;  // true if the graph is a morphological dictionary-graph
  struct hash_table* path_to_stop; /* a hash table to know all the Fst2Tag whose path exploration must be interrupted */
  struct hash_table* dela_entries; /* a hash table to get the dela_entries of created boxes when lexical masks are processed */

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
    if (fn){
      strcat(des, fn);
    }
    else{
      strcat(des, ofnameOnly);
    }
    if (ext){
      strcat(des, ext);
    }
    else{
      strcat(des, ofExt);
    }
  }

  CFstApp() :
    a(0), fst2_free(FST2_free_info_init), foutput(0),
        prMode(PR_SEPARATION), automateMode(AUTOMODE), listOut(0),
        verboseMode(0), enableLoopCheck(1), pathIdx(0),

        invocStackIdx(0),

        ignoreTable(0), numOfIgnore(0),

        outLineLimit(0x10000000), numberOfOutLine(0), count_in_line(0),

        totalPath(0), totalLoop(0), stopPath(0), errPath(0),

        recursiveMode(STOP), display_control(FULL),
        traitAuto(SINGLE),
        wordMode(1), // unit of box is word
        depthDebug(0),

        saveSep(u_null_string), sepL(u_null_string),
        sepR(u_null_string), sep1(u_null_string), stopSignal(
            u_null_string), saveEntre(u_null_string), openingQuote(
            u_null_string), closingQuote(u_null_string),

        inMorphoMode(false), isKorean(false), p(NULL), morphDicCnt(0),
        
        isMdg(false), path_to_stop(NULL), dela_entries(NULL),
        
        autoCallStack(NULL), transitionListHead(NULL), transitionListTail(NULL),

        cycInfos(NULL),
        headCyc(0), cyclePathCnt(0), headCycNodes(0), cycNodeCnt(0),
        
        inputPtrCnt(0), outputPtrCnt(0),
        inBufferCnt(0), outBufferCnt(0),
        alphabet(NULL), korean(NULL), input_variables(NULL), processedLexicalMasks(NULL),
        lexicalMaskCnt(0), maxLexicalMaskCnt(8),
        
        stopSubListIdx(0) {
          initCallIdMap();
        }
  ;
  ~CFstApp() {
    stopExploDel();
    cleanCyclePath();
    free_abstract_Fst2(a, &fst2_free);
    if (saveSep != u_null_string){
      free(saveSep);
    }
    if (sep1 != u_null_string){
      free(sep1);
    }
    if (stopSignal != u_null_string){
      free(stopSignal);
    }
    if (saveEntre != u_null_string){
      free(saveEntre);
    }
    if (ignoreTable){
      free(ignoreTable);
    }
    if (numOfIgnore){
      free(numOfIgnore);
    }
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
    delete korean;
    free_Variables(p->input_variables);
    free_OutputVariables(p->output_variables);
    free_OutputVariables(input_variables);
    free_alphabet(alphabet);
    for (int i = 0; i < lexicalMaskCnt; i++){
      free(processedLexicalMasks[i].input);
      free(processedLexicalMasks[i].output);
    }
    free(processedLexicalMasks);
  }

  /**
   * Call stack for the currently opened automata
   * Transitions can be out of date after backtracking
   * in exploreSubgraphRecursively
   * The depth of autoCallStack is managed by autoDepth
   * in exploreSubgraphRecursively
   */
  struct autoCallStack_t {
    // transition that invoked the current subgraph
    Transition* tran;
    // identifies a snapshot of autoCallStack
    int stackStateID; 
  }*autoCallStack;

  /**
   * Linked list of lists of transitions
   * each transitionList identifies a snapshot of autoCallStack
   * they're all pairwise distinct
   */
  struct transitionList {
    int cnt;    // list size
    Transition** trans; // list of transitions identifying a stackStateID
    struct transitionList *next;
  }*transitionListHead, *transitionListTail;

  /**
   * Searches transitionList for an occurrence of autoCallStack current state
   * Returns its ID if found, or a new ID if it didn't exist
   */
  int identifyStackState(int depth) {
    int id = 0;
    struct transitionList *callMapPtr;
    callMapPtr = transitionListHead;
    // search for list of transitions at the given depth
    while (callMapPtr) {
      if (callMapPtr->cnt == depth) {
        int i;
        // search for the subgraph call corresponding to the transitions in autoCallStack
        for (i = 0; i < depth; i++) {
          if (callMapPtr->trans[i] != autoCallStack[i].tran) {
            break;
          }
        }
        // if autoCallStack corresponds to transitionList then
        // we have found the stack state ID
        if (i == depth) {
          return id;
        }
      }
      id++;
      callMapPtr = callMapPtr->next;
    }
    // if the stack state is not yet in the transitionList we add it
    callMapPtr = new struct transitionList;
    callMapPtr->cnt = depth;
    callMapPtr->next = 0;
    callMapPtr->trans = new Transition*[depth];
    for (int i = 0; i < depth; i++) {
      callMapPtr->trans[i] = autoCallStack[i].tran;
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
    autoCallStack = new struct autoCallStack_t[1024];
    transitionListHead = transitionListTail = 0;
  }

  void deleteCallIdMap() {
    delete[] autoCallStack;
    while (transitionListHead) {
      transitionListTail = transitionListHead;
      transitionListHead = transitionListHead->next;
      delete[] transitionListTail->trans;
      delete transitionListTail;
    }
  }

  struct cyclePathMark {
    int index; // number of identify
    struct pathStack_t *pathTagCopy; // copy of cycle
    int pathCnt; // counter in pathTagCopy
    int flag;
    struct cyclePathMark *next;
  };


  //  save all nodes which have the cycle path
  //  the path from initial to the node is used to identify the node
  //  in subgraph
  struct linkCycle {
    struct cyclePathMark *cyc;
    struct linkCycle *next;
  }*cycInfos;


 /**
  * structure to represent all the processed lexical masks
  */
  struct ProcessedLexicalMask {
    unichar* input;         // input extracted from dic
    unichar* output;        // output extracted from inf file
    int maxEntriesCnt;      // max number of element in entries array, useful to reallocation
    int entriesCnt;         // number of entries in entries field
  };

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

  /**
   * Search in headCycNodes for an index corresponding to
   * stateNo, stackStateID and tag from pathStack element at 'curidx'
   */
  unichar *getLabelNumber(unichar*aa, int numOfPath, int &flag, int curidx, int setflag) {
    struct cycleNodeId *cnode = headCycNodes;
    int searchState = pathStack[curidx].stateNo & PATHID_MASK;
    int searchStateAuto = pathStack[curidx].stackStateID;
    int searchTag = pathStack[curidx].tag;
    while (cnode) {
      if ((searchStateAuto == cnode->autoNo) 
          && (searchState == cnode->stateNo) && (searchTag == cnode->tag)) {
        break;
      }
      cnode = cnode->next;
    }
    if (!cnode) {
      error("%d/%d stack\n", numOfPath, curidx);
      for (int i = 0; i < pathIdx; i++) {
        error("%d : (%08x:%08x) : %08x\n", i, pathStack[i].stackStateID, pathStack[i].stateNo, pathStack[i].tag);
      }

      printPathNames(U_STDERR);
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
   * used in case of exploration when loop check is enabled
   */
  void setIdentifyValue(int offset, int cntNode) {
    struct cycleNodeId **cnode = &headCycNodes;

    int cycStateNo = pathStack[cntNode - 1].stateNo & PATHID_MASK;
    int cycStateAutoNo = pathStack[cntNode - 1].stackStateID;
    int cycStateTag = pathStack[cntNode - 1].tag;
    while (*cnode) {
      if (((*cnode)->autoNo == cycStateAutoNo) 
          && ((*cnode)->stateNo == cycStateNo) 
          && ((*cnode)->tag == cycStateTag)) {
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
    numOfPath = pathIdx - offset;
    while (*h) {
      if ((*h)->pathCnt == numOfPath) {
        for (i = 0; i < numOfPath; i++) {
          if ((pathStack[offset].stackStateID
              == (*h)->pathTagCopy[i].stackStateID)
              && ((pathStack[offset].stateNo & PATHID_MASK)
                  == (*h)->pathTagCopy[i].stateNo)
              && (pathStack[offset].tag
                  == (*h)->pathTagCopy[i].tag)) {
            break;
          }
        }
        if (i != numOfPath) { // find first the position in cycle ring
          for (j = 0; j < numOfPath; j++) {
            if (((pathStack[offset + j].stateNo & PATHID_MASK)
                != (*h)->pathTagCopy[i].stateNo)
                || (pathStack[offset + j].tag
                    != (*h)->pathTagCopy[i].tag)
                || (pathStack[offset + j].stackStateID
                    != (*h)->pathTagCopy[i].stackStateID)) {
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
    (*h)->pathTagCopy = new struct pathStack_t[numOfPath];
    if (!((*h)->pathTagCopy)) {
      fatal_error(StrMemLack);
    }
    for (i = 0; i < numOfPath; i++) {
      (*h)->pathTagCopy[i].stateNo = pathStack[i + offset].stateNo
          & PATHID_MASK;
      (*h)->pathTagCopy[i].tag = pathStack[i + offset].tag;
      (*h)->pathTagCopy[i].stackStateID = pathStack[i + offset].stackStateID;
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
      delete cp->pathTagCopy;
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
   * Detect a recursive call
   * Increment 'totalLoop' if a loop is found
   * Check if there is a recursion using pathStack
   * as a history of exploration
   * Return 1 if a cycle is found, else 0
   */
  int isCyclePath(int depth) {
    int scanner; 
    int curId = pathStack[pathIdx - 1].stateNo & PATHID_MASK;
    int curAutoId = pathStack[pathIdx - 1].stackStateID;

    for (scanner = 0; scanner < pathIdx - 1; scanner++) {
      // find recursive path
      if (((pathStack[scanner].stateNo & PATHID_MASK) == curId) 
                 && (pathStack[scanner].stackStateID == curAutoId)) {
        switch (recursiveMode) {
        case LABEL:
          if (listOut) {
            pathStack[pathIdx - 1].stateNo |= STOP_PATH_MARK;
            outWordsOfGraph(scanner);
          } else {
            //          saveNodeIndex(scanner);
            setIdentifyValue(scanner, pathIdx);
          }
          break;
        case SYMBOL:
          if (!listOut) {
            setIdentifyValue(scanner, pathIdx);
          }
          break;
        case STOP:
          if (listOut) {
            pathStack[scanner].stateNo |= LOOP_PATH_MARK;
            pathStack[pathIdx - 1].stateNo |= LOOP_PATH_MARK
                | STOP_PATH_MARK;
            outWordsOfGraph(depth);
            pathStack[scanner].stateNo &= ~LOOP_PATH_MARK;
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

  unichar inputBuffer[128];
  unichar outputBuffer[128];
  int inputPtrCnt;            // input pointer counter
  int outputPtrCnt;           // output pointer counter
  unichar INPUTBUFFER[4096];  // buffer used to print the box inputs 
  unichar OUTPUTBUFFER[4096]; // buffer used to print the box outputs
  unichar jamos[4096 * 3];    // buffer used for korean
  int inBufferCnt;            // buffer counter for box inputs
  int outBufferCnt;           // buffer counter for box outputs
  Alphabet *alphabet;
  Korean *korean;

  OutputVariables *input_variables;

  ProcessedLexicalMask *processedLexicalMasks;
  int lexicalMaskCnt;
  int maxLexicalMaskCnt;
  struct dela_entry** entries;
  
  void resetBufferCounters() {
    inputPtrCnt = outputPtrCnt = inBufferCnt = outBufferCnt = 0;
  }

  /**
    * append a single space to the buffers when the morphological mode is off
  **/
  void appendSingleSpace() {
    unichar *wordPtr;
    wordPtr = sepR;
    if(!inMorphoMode) {
      while (*wordPtr) {
        INPUTBUFFER[inBufferCnt++] = *wordPtr++;
      }
    }
  }

  /**
   * fill INPUTBUFFER and OUTPUTBUFFER and 
   * print their content in 'fouput'
   * return 0 when output limit has been reached, else 1
   */
  int outOneWord(unichar *suffix) {
    int setOut;
    unichar *wordPtr;
    inputBuffer[inputPtrCnt] = outputBuffer[outputPtrCnt] = 0;
    if (!inputPtrCnt && !outputPtrCnt && !suffix) {
      return 0;
    }
    if (suffix) {
      setOut = 0;
      if (inputPtrCnt || outputPtrCnt || *suffix || (count_in_line == 0)) {
        setOut = 1;
        if (prMode == PR_SEPARATION) {
          if(inputPtrCnt) {
            wordPtr = sepL;
            while (*wordPtr) {
              INPUTBUFFER[inBufferCnt++] = *wordPtr;
              if (automateMode == TRANMODE) {
                OUTPUTBUFFER[outBufferCnt++] = *wordPtr;
              }
              wordPtr++;
            }
            for (int i = 0; i < inputPtrCnt; i++) {
              INPUTBUFFER[inBufferCnt++] = inputBuffer[i];
            }
            appendSingleSpace();
          }
          if (automateMode == TRANMODE && !(grammarMode == REPLACE && inputPtrCnt && !isMdg)) {
            for (int i = 0; i < outputPtrCnt; i++) {
              OUTPUTBUFFER[outBufferCnt++] = outputBuffer[i];
            }
            if(grammarMode == MERGE) {
              for (int i = 0; i < inputPtrCnt; i++) {
                OUTPUTBUFFER[outBufferCnt++] = inputBuffer[i];
              }
            }
          }
        } else {
          wordPtr = sepL;
          while (*wordPtr) {
            INPUTBUFFER[inBufferCnt++] = *wordPtr++;
          }
          if(grammarMode == MERGE) {
            for (int i = 0; i < outputPtrCnt; i++) {
              INPUTBUFFER[inBufferCnt++] = outputBuffer[i];
            }
          }
          else {
            for (int i = 0; i < inputPtrCnt; i++) {
              INPUTBUFFER[inBufferCnt++] = inputBuffer[i];
            }
          }
          wordPtr = saveSep;
          while (*wordPtr) {
            INPUTBUFFER[inBufferCnt++] = *wordPtr++;
          }
          if (automateMode == TRANMODE) {
            if(grammarMode == MERGE) {
              for (int i = 0; i < outputPtrCnt; i++) {
                INPUTBUFFER[inBufferCnt++] = outputBuffer[i];
              }
            }
            else {
              for (int i = 0; i < inputPtrCnt; i++) {
                INPUTBUFFER[inBufferCnt++] = inputBuffer[i];
              }
            }
          }
          appendSingleSpace();
        }
      } // condition de out
      if ((recursiveMode == LABEL) && setOut) {
        if ((automateMode == TRANMODE) && (prMode == PR_SEPARATION)) {
          wordPtr = openingQuote;
          while (*wordPtr) {
            OUTPUTBUFFER[outBufferCnt++] = *wordPtr++;
          }
          wordPtr = suffix;
          while (*wordPtr) {
            OUTPUTBUFFER[outBufferCnt++] = *wordPtr++;
          }
        } else {
          wordPtr = openingQuote;
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
      if(isKorean) {
        Hanguls_to_Jamos(INPUTBUFFER, jamos, korean, 1);
        convert_jamo_to_hangul(jamos, INPUTBUFFER, korean);
      }
      u_fputs(INPUTBUFFER, foutput);
      if ((automateMode == TRANMODE) && outBufferCnt) {
        OUTPUTBUFFER[outBufferCnt] = 0;
        u_fprintf(foutput, "%S%S", saveSep, OUTPUTBUFFER);
      }
      if (display_control == FST2LIST_DEBUG) {
        printPathNames(foutput);
      }
      u_fprintf(foutput, "\n");
      numberOfOutLine++;
      inBufferCnt = outBufferCnt = 0;
      empty_non_pending_variables(input_variables);
      empty_non_pending_variables(p->output_variables);
    } else { // suffix == 0
      if (inputPtrCnt || outputPtrCnt) {
        if (prMode == PR_SEPARATION) {
          if(inputPtrCnt) {
            wordPtr = sepL;
            while (*wordPtr) {
              INPUTBUFFER[inBufferCnt++] = *wordPtr;
              if (automateMode == TRANMODE) {
                OUTPUTBUFFER[outBufferCnt++] = *wordPtr;
              }
              wordPtr++;
            }
            for (int i = 0; i < inputPtrCnt; i++) {
              INPUTBUFFER[inBufferCnt++] = inputBuffer[i];
            }
            appendSingleSpace();
          }
          if (automateMode == TRANMODE && !(grammarMode == REPLACE && inputPtrCnt && !isMdg)) {
            for (int i = 0; i < outputPtrCnt; i++) {
              OUTPUTBUFFER[outBufferCnt++] = outputBuffer[i];
            }
            if(grammarMode == MERGE) {
              for (int i = 0; i < inputPtrCnt; i++) {
                OUTPUTBUFFER[outBufferCnt++] = inputBuffer[i];
              }
            }
          }
          //        if(recursiveMode == LABEL){
          //          wordPtr = openingQuote;while(*wordPtr)  INPUTBUFFER[inBufferCnt++] = *wordPtr++;
          //          }
        } else {
          wordPtr = sepL;
          while (*wordPtr) {
            INPUTBUFFER[inBufferCnt++] = *wordPtr++;
          }
          if(grammarMode == MERGE) {
            for (int i = 0; i < outputPtrCnt; i++) {
              INPUTBUFFER[inBufferCnt++] = outputBuffer[i];
            }
          }
          else {
            for (int i = 0; i < inputPtrCnt; i++) {
              INPUTBUFFER[inBufferCnt++] = inputBuffer[i];
            }
          }
          wordPtr = saveSep;
          while (*wordPtr) {
            INPUTBUFFER[inBufferCnt++] = *wordPtr++;
          }
          if (automateMode == TRANMODE) {
            if(grammarMode == MERGE) {
              for (int i = 0; i < inputPtrCnt; i++) {
                INPUTBUFFER[inBufferCnt++] = inputBuffer[i];
              }
            }
            else {
              for (int i = 0; i < outputPtrCnt; i++) {
                INPUTBUFFER[inBufferCnt++] = outputBuffer[i];
              }
            }
          }
          if (recursiveMode == LABEL) {
            wordPtr = openingQuote;
            while (*wordPtr) {
              INPUTBUFFER[inBufferCnt++] = *wordPtr++;
            }
          }
          appendSingleSpace();
        }
        count_in_line++;
      }
    }

    inputPtrCnt = outputPtrCnt = 0;

    if (outLineLimit <= numberOfOutLine) {
      return 1;
    }
    return 0;
  }

  void dummyWordOut() {
    unichar *wordPtr;
    if ((recursiveMode == LABEL) && !count_in_line) {
      wordPtr = sepL;
      while (*wordPtr){
        INPUTBUFFER[inBufferCnt++] = *wordPtr++;
      }
      wordPtr = saveSep;
      while (*wordPtr){
        INPUTBUFFER[inBufferCnt++] = *wordPtr++;
      }
      wordPtr = sepR;
      while (*wordPtr){
        INPUTBUFFER[inBufferCnt++] = *wordPtr++;
      }
      wordPtr = openingQuote;
      while (*wordPtr){
        INPUTBUFFER[inBufferCnt++] = *wordPtr++;
      }
    }
    INPUTBUFFER[inBufferCnt++] = 0;
    u_fprintf(foutput, "%S\n", INPUTBUFFER);
    numberOfOutLine++;
  }

  void putInt(int flag, int v) {
    if (v < 10) {
      inputBuffer[inputPtrCnt] = v % 10 + (unichar) '0';
      if (flag) {
        outputBuffer[outputPtrCnt++] = inputBuffer[inputPtrCnt];
      }
      inputPtrCnt++;
      return;
    }
    putInt(flag, v / 10);
    inputBuffer[inputPtrCnt] = v % 10 + (unichar) '0';
    if (flag) {
      outputBuffer[outputPtrCnt++] = inputBuffer[inputPtrCnt];
    }
    inputPtrCnt++;
  }

  int printOutCycle() {
    struct cyclePathMark *h = headCyc;
    int i;
    unichar *wordPtr;
    //        unichar *wwordPtr;
    Fst2Tag Tag;
    while (h) {
      u_fprintf(foutput, "C%d%S", h->index, openingQuote);
      inputPtrCnt = outputPtrCnt = 0;
      for (i = 0; i < h->pathCnt; i++) {
        //        putInt(0,h->pathTagCopy[i].path);
        Tag = a->tags[h->pathTagCopy[i].tag];
        wordPtr = (unichar *) Tag->input;
        if (u_strcmp(wordPtr, u_epsilon_string)) {
          //          wwordPtr = saveSep;while(*wwordPtr) inputBuffer[inputPtrCnt++] = *wwordPtr++;
          while (*wordPtr) {
            inputBuffer[inputPtrCnt++] = *wordPtr++;
          }
        }
        wordPtr = (unichar *) Tag->output;
        if ((automateMode == TRANMODE) && wordPtr && u_strcmp(wordPtr,
            u_epsilon_string)) {
          //            wwordPtr = saveSep;while(*wwordPtr) inputBuffer[inputPtrCnt++] = *wwordPtr++;
          while (*wordPtr) {
            outputBuffer[outputPtrCnt++] = *wordPtr++;
          }
        }
      }
      if (outOneWord(closingQuote) != 0) {
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
      tmp = h->pathTagCopy[h->pathCnt - 1].stateNo;
      if (tmp & SUBGRAPH_PATH_MARK) {
        tmp = a->initial_states[tmp & SUB_ID_MASK];
      }
      if ((tmp < st) || (tmp >= ed)) {
        h = h->next;
        continue;
      }

      inputPtrCnt = outputPtrCnt = 0;
      inputBuffer[inputPtrCnt++] = (unichar) 'C';
      putInt(0, h->index);
      inputBuffer[inputPtrCnt++] = (unichar) ':';

      for (i = 0; i < h->pathCnt; i++) {
        tmp = h->pathTagCopy[i].stateNo;
        if (tmp & SUBGRAPH_PATH_MARK) {
          tmp = a->initial_states[tmp & SUB_ID_MASK];
        }
        if ((tmp < st) || (tmp >= ed)) {
          break;
        }
        Tag = a->tags[h->pathTagCopy[i].tag];
        wordPtr = (unichar *) Tag->input;
        if (u_strcmp(wordPtr, u_epsilon_string) && *wordPtr) {
          while (*wordPtr) {
            inputBuffer[inputPtrCnt++] = *wordPtr++;
          }
        }
        wordPtr = (unichar *) Tag->output;
        if ((automateMode == TRANMODE) && wordPtr && u_strcmp(wordPtr,
            u_epsilon_string) && *wordPtr) {
          while (*wordPtr) {
            outputBuffer[outputPtrCnt++] = *wordPtr++;
          }
        }
      }
      if (i == h->pathCnt) {
        if (outOneWord(u_null_string) != 0) {
          return 1;
        }
      }
      else{
        resetBufferCounters();
      }
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
        inputBuffer[i] = (unichar) (fileLine[i] & 0xff);
      }
      inputBuffer[i++] = 0;

      if (stopSubListIdx == MAX_IGNORE_SUB_GRAPH) {
        u_printf("too many ignored sub-graph, name ignore %s\n", src);
        return;
      }
      stopSubList[stopSubListIdx] = new unichar[i];
      for (i = 0; inputBuffer[i]; i++) {
        stopSubList[stopSubListIdx][i] = inputBuffer[i];
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
   * Prints the invocation stacks,
   * the path stack and the automaton stack
   * hints :
   * stackStateID is -1 means the automaton reached final state
   * `trans_tag` are `tag_number` which is the ID of 
   * the targeted automaton with marks describing the transition
   */
  void printStacks(int autoCallStackDepth) {
    int i;
    u_printf("===== invocStack\n");
    u_printf("i stackStateID next_state\n");
    for (i = 0; i <= invocStackIdx; i++) {
      u_printf("%d :: %d :: %d\n", i, invocStack[i].stackStateID,
          invocStack[i].targetState);
    }
    u_printf("===== pathStack\n");
    u_printf("i stackStateID stateNo tag\n");
    for (i = 0; i < pathIdx; i++) {
      u_printf("%d (%d ::%d)%d\n", i, pathStack[i].stackStateID,
          pathStack[i].stateNo, pathStack[i].tag);
    }
    u_printf("===== autoCallStack\n"); 
    u_printf("i stackStateID stateNb trans_tag\n");
    Transition *k;
    for (i = 0; i < autoCallStackDepth; i++) {
      k = autoCallStack[i].tran;
      u_printf("%d %d(%d ::%d)\n", i, autoCallStack[i].stackStateID,
          k->state_number, k->tag_number);
    }
    u_printf("\n");
  }

  /**
   * Print all the list present in `transitionList`
   */
  void printTransitionList () {
    struct transitionList *callMapPtr;
    callMapPtr = transitionListHead;
    int cnt = 0;
    while (callMapPtr) {
      u_printf("list %d\n",cnt);
      for (int i = 0; i < callMapPtr->cnt; i++) {
        u_printf("depth %d trans_tag %d state %d\n",i,callMapPtr->trans[i]->tag_number,callMapPtr->trans[i]->state_number);
      }
      cnt++;
      callMapPtr = callMapPtr->next;
    }
  }

  void printAutoCallStack(int autoCallStackDepth) {
    u_printf("===== autoCallStack\n"); 
    u_printf("i  stackStateID  stateNb trans_tag\n");
    Transition *k;
    for (int i = 0; i < autoCallStackDepth; i++) {
      k = autoCallStack[i].tran;
      u_printf("%d %d(%d ::%d)\n", i, autoCallStack[i].stackStateID,k->state_number
          , k->tag_number);
    }
  }

  void printPathStack() {
    for (int i = 0; i < pathIdx; i++)
      u_fprintf(foutput, "%d (%d ::%d)%d\n", i, pathStack[i].stackStateID,
          pathStack[i].stateNo, pathStack[i].tag);
  }

  static void update_last_position(struct locate_parameters* p, int pos) {
    if (pos > p->last_tested_position) {
      p->last_tested_position = pos;
    }
  }

  /**
    * check if the given lexical mask has already been processed
    * output parameter is output of box with lexical mask; may be NULL
    * if this lexical mask has already been processed, this function returns the corresponding index in processedLexicalMasks
    * returns -1 in the other case
  **/
  int isProcessedLexicalMask(unichar* lexical_mask, unichar* output) {
    for(int i = 0; i < lexicalMaskCnt; i++) {
      if(!u_strcmp(lexical_mask, processedLexicalMasks[i].input) && !u_strcmp(output, processedLexicalMasks[i].output)) {
        return i;
      }
    }
    return -1;
  }

  /**
    * explore all the possible paths in the given dictionary
    * extract the entries matching the lexical mask
    * the entries are put in processedLexicalMask at the index corresponding to the lexical mask
  **/
  void extractEntriesFromDic(struct locate_parameters* param, Dictionary* d, int offset, unichar inflected[], int pos_in_inflected,
                        int pos_offset, Ustring *line_buffer, Ustring* ustr, struct pattern* pattern, int index, bool getAllDicEntries) {
    int final_state, n_transitions, inf_number;
    int z = save_output(ustr);
    offset = read_dictionary_state(d, offset, &final_state, &n_transitions, &inf_number);
    if (final_state) {  // if the current state is final, uncompress the entry to obtain the gramatical label
      inflected[pos_in_inflected] = '\0';

      if(isKorean) {
        convert_jamo_to_hangul(inflected, jamos, korean);
      }

      struct list_ustring* tmp = d->inf->codes[inf_number];
      uncompress_entry(inflected, tmp->string, line_buffer);
      struct dela_entry* entry = tokenize_DELAF_line_opt(line_buffer->str, NULL);
      if(getAllDicEntries || is_entry_compatible_with_pattern(entry, pattern)) {  // the pattern matches the gramatical label
        if(processedLexicalMasks[index].entriesCnt >= processedLexicalMasks[index].maxEntriesCnt) {
          entries = (struct dela_entry**)realloc(entries, (sizeof(struct dela_entry*) * processedLexicalMasks[lexicalMaskCnt].maxEntriesCnt * 2));
          if(entries == NULL) {
            fatal_error("realloc error for entries in extractEntriesFromDic");
          }
          processedLexicalMasks[index].maxEntriesCnt *= 2;
        }
        entries[processedLexicalMasks[lexicalMaskCnt].entriesCnt++] = clone_dela_entry(entry, NULL);  // add the dela entry into the dela entry array
      }
      free_dela_entry(entry, NULL);
    }
    unichar c;
    int adr;
    for (int i = 0; i < n_transitions; i++) {  // if the current state is not final, explores all the outgoing transitions
      update_last_position(param, pos_offset);
      offset = read_dictionary_transition(d,offset,&c,&adr,ustr);
      inflected[pos_in_inflected] = c;
      extractEntriesFromDic(param, d, adr, inflected, pos_in_inflected + 1, pos_offset, line_buffer, ustr, pattern, index, getAllDicEntries);
      restore_output(z,ustr);
    }
  }

  /**
    * create a subgraph when a new lexical mask is found
    * this subgraph contains two states : the initial state
    * and the state with all the entries found in processedLexicalMask's index corresponding to this lexical mask (the last index i.e lexicalMaskCnt)
  **/
  void createLexicalMaskSubgraph() {
    a->number_of_graphs += 1;
    if(processedLexicalMasks[lexicalMaskCnt].output != NULL) {
      a->graph_names[a->number_of_graphs] = (unichar*)malloc(sizeof(unichar) *
                                            ((int)u_strlen(processedLexicalMasks[lexicalMaskCnt].input) +
                                            (int)u_strlen(processedLexicalMasks[lexicalMaskCnt].output)) + 2);
      if(a->graph_names[a->number_of_graphs] == NULL) {
        fatal_error("malloc error for internal-use graph in createLexicalMaskSubgraph");
      }
      u_sprintf(a->graph_names[a->number_of_graphs],"%S%S", processedLexicalMasks[lexicalMaskCnt].input,
                processedLexicalMasks[lexicalMaskCnt].output);
    }
    else {
      a->graph_names[a->number_of_graphs] = (unichar*)malloc(sizeof(unichar) * (int)u_strlen(processedLexicalMasks[lexicalMaskCnt].input) + 2);
      if(a->graph_names[a->number_of_graphs] == NULL) {
        fatal_error("malloc error for internal-use graph in createLexicalMaskSubgraph");
      }
      u_sprintf(a->graph_names[a->number_of_graphs],"%S", processedLexicalMasks[lexicalMaskCnt].input);
    }
    a->initial_states[a->number_of_graphs] = a->number_of_states;
    a->number_of_states_per_graphs[a->number_of_graphs] = 2;
    a->number_of_states += 2;
    a->states[a->number_of_states - 2] = new_Fst2State(NULL);
    set_initial_state(a->states[a->number_of_states - 2], 1);
    a->states[a->number_of_states - 1] = new_Fst2State(NULL);
    set_final_state(a->states[a->number_of_states -1], 1);
    int last_number_of_tags = a->number_of_tags;
    a->number_of_tags += processedLexicalMasks[lexicalMaskCnt].entriesCnt;
    a->tags = (Fst2Tag*)realloc(a->tags, a->number_of_tags * sizeof(Fst2Tag));
    if(a->tags == NULL) {
      fatal_error("realloc error for tags in createLexicalMaskSubgraph");
    }
    // create a new tag for each entry found in processedLexicalMask at lexicalMaskCnt index
    int k = a->number_of_tags - 1;
    for(int i = last_number_of_tags; i < a->number_of_tags; i++) {
      a->tags[i] = new_Fst2Tag(NULL);
      a->tags[i]->input = u_strdup(entries[lexicalMaskCnt + k - last_number_of_tags]->inflected);
      if(processedLexicalMasks[lexicalMaskCnt].output != NULL) {
        a->tags[i]->output = u_strdup(processedLexicalMasks[lexicalMaskCnt].output);
        if (u_starts_with(processedLexicalMasks[lexicalMaskCnt].output, "$")){ // var dic
          struct any* value = get_value(dela_entries, a->tags[i], HT_INSERT_IF_NEEDED);
          value->_ptr = entries[lexicalMaskCnt + k - last_number_of_tags];
          entries[lexicalMaskCnt + k - last_number_of_tags] = NULL;
        }
      }
      free_dela_entry(entries[lexicalMaskCnt + k - last_number_of_tags]);
      add_transition_to_state(a->states[a->number_of_states - 2], i, a->number_of_states - 1, NULL);
      k--;
    }
  }

  /**
  * count the number of lexical mask in the current automaton
  */
  int countLexicalMasks() {
    int count = 0;
    for(int j = 0; j < a->number_of_states; j++) {
      Transition *t = a->states[j]->transitions;
      while(t != NULL) {
        if(!(t->tag_number & SUBGRAPH_PATH_MARK) && (a->tags[t->tag_number]->input[0] == '<' && a->tags[t->tag_number]->input[u_strlen(a->tags[t->tag_number]->input) - 1] == '>')
          && u_strcmp(a->tags[t->tag_number]->input, "<E>")) {
          count++;
        }
        t = t->next;
      }
    }
    return count;
  }
  
  /**
   * lexical masks which should be read literally and not trigger extraction of entries from dictionaries
   */
  bool maskMustBeReadLiterally(unichar *lexical_mask){
    char special_mask[NB_LITTERAL_MASKS][16] = {"<NB>", "<PRE>", "<^>", "<MAJ>", "<PNC>", "<MOT>", "<TDIC>",
                                                "<MIN>", "<WORD>", "<FIRST>", "<LOWER>", "<UPPER>", "<TOKEN>", "<LETTER>"}; // 4.3 manual
    if (lexical_mask == NULL || u_starts_with(lexical_mask, "<!")){ // negative lexicals masks are processed literally too
      return true;
    }
    for (int i = 0; i < NB_LITTERAL_MASKS; i++){
      if (!u_strcmp(lexical_mask, special_mask[i])){
        return true;
      }
    }
    return false;
  }
  
  /**
   * tells if the lexical masks is a DIC mask
   * DIC masks require the extraction of all entries from binary dictionaries
   * This is a functional difference with LocatePattern
   * LocatePattern matches "SDIC" only with simple entries and "CDIC" only with multiword entries
   */
  bool isDicMask(unichar *lexical_mask){
     char dic_mask[3][16] = {"<DIC>", "<SDIC>", "<CDIC>"}; // 4.3 manual
     if (lexical_mask == NULL){
       return false;
     }
     for (int i = 0; i < 3; i++){
       if (!u_strcmp(lexical_mask, dic_mask[i])){
         return true;
       }
     }
     return false;
  }
  
  /**
   * This function returns a hash code for a Fst2Tag.
   *
  */
  static unsigned int Fst2Tag_hash(const void *ptr){
    uint64_t pointer_int = (uint64_t)ptr;
    uint64_t hash = (pointer_int >> 48) ^ (pointer_int >> 32) ^ (pointer_int >> 16) ^ pointer_int;
    return (unsigned int)hash;
  }
  
 /**
   * This function tests the memory equality of two Fst2Tag.
   *
  */
  static int Fst2Tag_equal(Fst2Tag a, Fst2Tag b){
    return (a == b);
  }
  
 /**
   * This function frees a Fst2Tag.
   *
  */
  static void Fst2Tag_free(Fst2Tag ptr){
    free_Fst2Tag(ptr,STANDARD_ALLOCATOR);
  }
  
  /**
   * This function frees a dela entry.
   *
  */
  static void Dela_entry_free(void *entry){
    free_dela_entry((struct dela_entry *)entry,STANDARD_ALLOCATOR);
  }
  
  /**
  * reallocate each pointer whose size depends on the number of graphs
  */
  void reallocFst2(int count) {
    int total_number_of_graph = a->number_of_graphs + count + 1;
    a->graph_names = (unichar**)realloc(a->graph_names, sizeof(unichar*) * total_number_of_graph);
    if(a->graph_names == NULL) {
      fatal_error("realloc error for graph_names in reallocFst2");
    }
    a->initial_states = (int*)realloc(a->initial_states, sizeof(int) * total_number_of_graph);
    if(a->initial_states == NULL) {
      fatal_error("realloc error for initial_states in reallocFst2");
    }
    a->number_of_states_per_graphs = (int*)realloc(a->number_of_states_per_graphs, sizeof(int) * total_number_of_graph);
    if(a->number_of_states_per_graphs == NULL) {
      fatal_error("realloc error for number_of_states_per_graph in reallocFst2");
    }
    a->states = (Fst2State*)realloc(a->states, (a->number_of_states + (count * 2)) * sizeof(Fst2State));
    if(a->states == NULL) {
      fatal_error("realloc error for states in reallocFst2");
    }
    ignoreTable = (int*)realloc(ignoreTable, sizeof(int) * total_number_of_graph);
    numOfIgnore = (int*)realloc(numOfIgnore, sizeof(int) * total_number_of_graph);
    for(int i = a->number_of_graphs; i < total_number_of_graph; i++) {
      ignoreTable[i] = 0;
      numOfIgnore[i] = 0;
    }
  }

  /**
    * check the automaton's tags to find lexical masks
    * for each lexical mask, explore the morphological dictionnaries
    * and create a subgraph with all the entries that match the lexical mask
  **/
  void check_lexical_masks() {
    unichar inflected[1024];
    struct pattern* pattern;
    int n_states = a->number_of_states;
    int count = countLexicalMasks();
    bool getAllDicEntries = false;
    inMorphoMode = false;
    if(count == 0) {  // no lexical mask in this automaton
      return;
    }
    reallocFst2(count);
    for(int j = 0; j < n_states; j++) {
      Transition *t = a->states[j]->transitions;
      while(t != NULL) {
        if(!(t->tag_number & SUBGRAPH_PATH_MARK)) {  // check if the input is sub-graph-call (negative tag)
          if(a->tags[t->tag_number]->type == BEGIN_MORPHO_TAG){
            inMorphoMode = true;
          }
          else if(a->tags[t->tag_number]->type == END_MORPHO_TAG){
            inMorphoMode = false;
          }
          // check if the input tag is a lexical mask
          if(a->tags[t->tag_number]->input[0] == '<' && a->tags[t->tag_number]->input[u_strlen(a->tags[t->tag_number]->input) - 1] == '>'
             && u_strcmp(a->tags[t->tag_number]->input, "<E>") && inMorphoMode) {
             if (maskMustBeReadLiterally(a->tags[t->tag_number]->input)){
                u_fprintf(U_STDERR, "Warning: %S will be processed literally\n", a->tags[t->tag_number]->input);
                t = t->next;
                continue;
             }
             else if(isDicMask(a->tags[t->tag_number]->input)) {  // Dic case, all the entries from the morphological dictionaries will be extracted
               getAllDicEntries = true;
             }
             unichar *lexical_mask = (unichar*)malloc(sizeof(unichar) * 64);
             if (lexical_mask == NULL){
               fatal_alloc_error("check_lexical_masks");
             }
             u_strcpy(lexical_mask, a->tags[t->tag_number]->input);
             lexical_mask[u_strlen(lexical_mask) -1] = '\0';
             lexical_mask++;
             int index = isProcessedLexicalMask(lexical_mask, a->tags[t->tag_number]->output);
             if(index >= 0) {  // the current lexical mask has already been processed
               if(processedLexicalMasks[index].entriesCnt == 0) {  // this lexical mask doesn't match any entry in morphological dic
                 get_value(path_to_stop, a->tags[t->tag_number], HT_INSERT_IF_NEEDED);
               }
               else {  // the current lexical mask has already been processed
                 t->tag_number = SUBGRAPH_PATH_MARK | (a->number_of_graphs - (lexicalMaskCnt - index) + 1);  // the transition references the corresponding subgraph
               }
             }
             else {  // this lexical mask isn't yet processed
               if(lexicalMaskCnt >= maxLexicalMaskCnt) {
                 processedLexicalMasks = (ProcessedLexicalMask*)realloc(processedLexicalMasks, sizeof(ProcessedLexicalMask) * maxLexicalMaskCnt * 2);
                 if(processedLexicalMasks == NULL){
                   fatal_error("realloc error for processedLexicalMasks in check_lexical_masks");
                 }
                 maxLexicalMaskCnt *= 2;
               }
               processedLexicalMasks[lexicalMaskCnt].input = u_strdup(lexical_mask);
               processedLexicalMasks[lexicalMaskCnt].maxEntriesCnt = 64;
               processedLexicalMasks[lexicalMaskCnt].entriesCnt = 0;
               if(a->tags[t->tag_number]->output != NULL) {
                 processedLexicalMasks[lexicalMaskCnt].output = u_strdup(a->tags[t->tag_number]->output);
               }
               else {
                 processedLexicalMasks[lexicalMaskCnt].output = NULL;
               }
               entries = (struct dela_entry**)malloc(sizeof(struct dela_entry*) * processedLexicalMasks[lexicalMaskCnt].maxEntriesCnt);
               if (entries == NULL){
                 fatal_error("malloc error for entries in check_lexical_masks");
               }
               pattern = build_pattern(lexical_mask, NULL, 0, NULL);
               for(int i = 0; i < morphDicCnt; i++) {
                 Ustring* ustr = new_Ustring();
                 Ustring* line_buffer = new_Ustring();
                 // extract all the entries matching the lexical_mask
                 extractEntriesFromDic(p, p->morpho_dic[i], p->morpho_dic[i]->initial_state_offset, inflected, 0, 0,
                                       line_buffer, ustr, pattern, lexicalMaskCnt, getAllDicEntries);
                 free_Ustring(ustr);
                 free_Ustring(line_buffer);
               }
               free_pattern(pattern, NULL);
               if(processedLexicalMasks[lexicalMaskCnt].entriesCnt > 0) {
                 createLexicalMaskSubgraph();
                 // modify the tran between the current state (lexical_mask)and the last state
                 t->tag_number = SUBGRAPH_PATH_MARK | a->number_of_graphs;
               }
               else {
                 get_value(path_to_stop, a->tags[t->tag_number], HT_INSERT_IF_NEEDED);
               }
               free(entries);
               lexicalMaskCnt++;
             }
             lexical_mask--;
             free(lexical_mask);
          }
        }
        t = t->next;
      }
    }
  }

private:
  /* prevent GCC warning */

  CFstApp(const CFstApp&) :
    a(0), fst2_free(FST2_free_info_init), foutput(0),
        prMode(PR_SEPARATION), automateMode(AUTOMODE), listOut(0),
        verboseMode(0), pathIdx(0),

        invocStackIdx(0),

        ignoreTable(0), numOfIgnore(0),

        outLineLimit(0x10000000), numberOfOutLine(0), count_in_line(0),

        totalPath(0), totalLoop(0), stopPath(0), errPath(0),

        recursiveMode(STOP), display_control(FULL),
        traitAuto(SINGLE),
        wordMode(1), // unit of box is word
        depthDebug(0),

        saveSep(u_null_string), sepL(u_null_string),
        sepR(u_null_string), sep1(u_null_string), stopSignal(
            u_null_string), saveEntre(u_null_string), openingQuote(
            u_null_string), closingQuote(u_null_string),

        autoCallStack(NULL), transitionListHead(NULL), transitionListTail(NULL),

        cycInfos(NULL),

        headCyc(0), cyclePathCnt(0), headCycNodes(0), cycNodeCnt(0),

        inputPtrCnt(0), outputPtrCnt(0), inBufferCnt(0), outBufferCnt(0),

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
  Fst2 *original;
  original = load_abstract_fst2(&vec, fname, 1, &fst2_free);
  if (original == NULL) {
    fatal_error("Cannot load graph file %s\n", fname);
  }
  a = new_Fst2_clone(original, STANDARD_ALLOCATOR);
  if (a == NULL) {
    fatal_error("Cannot clone original Fst2\n");
  }
  free_abstract_Fst2(original, &fst2_free);
  // mark the automaton transitions that invoke subgraphs
  for (i_1 = 0; i_1 < a->number_of_states; i_1++) {
    transPtr = a->states[i_1]->transitions;
    if (a->states[i_1]->control & 0x80) {
      fatal_error("Not null control bit");
    }
    a->states[i_1]->control &= 0x7f; // clean to mark recursive. 0x7f=0b0111_1111
    while (transPtr) {
      if (transPtr->tag_number < 0) {   // transition invokes subgraph
        transPtr->tag_number = SUBGRAPH_PATH_MARK | -transPtr->tag_number;
      }
      transPtr = transPtr->next;
    }
  }
  ignoreTable = (int*)malloc(sizeof(int) * (a->number_of_graphs + 1));
  numOfIgnore = (int*)malloc(sizeof(int) * (a->number_of_graphs + 1));
  if (ignoreTable == NULL || numOfIgnore == NULL){
    fatal_alloc_error("loadGraph");
  }
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
  alphabet = new_alphabet(1);
  korean = new Korean(alphabet);
  ofNameTmp[0] = 0;
  processedLexicalMasks = (ProcessedLexicalMask*)malloc(sizeof(ProcessedLexicalMask) * maxLexicalMaskCnt);
  if(processedLexicalMasks == NULL) {
    fatal_error("Malloc error for processedLexicalMasks in getWordsFromGraph");
  }

  input_variables = new_OutputVariables(a->input_variables, 0, NULL);
  p->output_variables = new_OutputVariables(a->output_variables, 0, NULL);
  p->input_variables = new_Variables(NULL, 0);
  p->dic_variables = NULL;
  p->variable_error_policy = EXIT_ON_VARIABLE_ERRORS;
  path_to_stop = new_hash_table(maxLexicalMaskCnt, (HASH_FUNCTION)Fst2Tag_hash, (EQUAL_FUNCTION)Fst2Tag_equal, (FREE_FUNCTION)Fst2Tag_free, NULL, NULL);
  dela_entries = new_hash_table((HASH_FUNCTION)Fst2Tag_hash, (EQUAL_FUNCTION)Fst2Tag_equal, (FREE_FUNCTION)Fst2Tag_free, Dela_entry_free, NULL);
  //Checks the automaton's tags to find lexical masks
  check_lexical_masks();
  for(i = 0; i < morphDicCnt; i++) {
    free_Dictionary(p->morpho_dic[i], NULL);
  }
  free(p->morpho_dic);
  switch (display_control) {
  case GRAPH: {    // explore each graph separately
    if (enableLoopCheck) {
      listOut = 0;
      wordMode = 1;
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
        if (!(trans->tag_number & SUBGRAPH_PATH_MARK)) {
          continue;
        }
        ignoreTable[trans->tag_number & SUB_ID_MASK] = 1;
        i++;
      }
      u_fprintf(listFile, " %d\n", i);
      for (trans = a->states[0]->transitions; trans != 0; trans = trans->next) {
        if (!(trans->tag_number & SUBGRAPH_PATH_MARK)) {
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
          u_printf(" automaton %s: %d paths, %d path stopped by cycle, %d path overflows \n"
              ,getUtoChar(charBuffOut, a->graph_names[trans->tag_number & SUB_ID_MASK]), totalPath, totalLoop, errPath);

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
  startCallTr.tag_number = startAutoNo | SUBGRAPH_PATH_MARK;
  startCallTr.state_number = 0;
  numberOfOutLine = 0; // reset output line counter

  // start the exploration on a 'fake' transition to the first automaton
  autoCallStack[0].tran = &startCallTr;
  int callSubGraphId = identifyStackState(1);
  autoCallStack[0].stackStateID = callSubGraphId;

  invocStackIdx = 0;
  invocStack[invocStackIdx].stackStateID = callSubGraphId;
  invocStack[invocStackIdx].targetState = 0;
  pathIdx = 0;
  pathStack[pathIdx].stackStateID = callSubGraphId;
  pathStack[pathIdx].stateNo = a->initial_states[startAutoNo];
  pathStack[pathIdx].tag = 0;
  pathIdx++;
  if (exploreSubgraphRecursively(callSubGraphId, 1, a->initial_states[startAutoNo], 0) != 0) {
    return 1;
  }
  pathIdx--;
  if (pathIdx) {
    fatal_error("error in program");
  }
  return 0;
}

/**
 * Explores graph and subgraph recursively
 * Doesn't enter in loop of calls
 * Begins from current state, includes
 * backtracking among future states but not 
 * backtracking to past states
 * When backtracking, autoCallStack doesn't push on
 * the stack the call of the automaton it returns to (items may have changed)
 * autoDepth is the depth of autoCallStack i.e. the depth of automaton calls
 */
int CFstApp::exploreSubgraphRecursively(int stackStateID, int autoDepth, int stateNo, int stateDepth) {
  int skipCnt = 0;
  int tmp;
  int nextState;
  int callId;

  if (listOut && wasCycleNode(stackStateID, stateNo)) {
    pathStack[pathIdx - 1].stateNo |= LOOP_PATH_MARK;
  }
  if (pathIdx >= PATH_STACK_MAX) {
    pathIdx = PATH_STACK_MAX - 1;
    if (listOut) {
      errPath++;
      pathStack[pathIdx].stateNo = STOP_PATH_MARK;
      pathStack[pathIdx].tag = 0;
      pathIdx++;
      if (outWordsOfGraph(pathIdx) != 0) {
        return 1;
      }
      pathIdx--;
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
        if (invocStack[i].stackStateID == -1) {
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
      int tauto = invocStack[i].stackStateID;
      nextState = invocStack[i].targetState;
      invocStackIdx++;
      invocStack[invocStackIdx].stackStateID = -1;
      invocStack[invocStackIdx].targetState = 0;

      pathStack[pathIdx].stateNo = nextState;
      pathStack[pathIdx].tag = 0;
      pathStack[pathIdx].stackStateID = tauto;
      pathIdx++;
      if (exploreSubgraphRecursively(tauto, autoDepth - 1, nextState, stateDepth + 1) != 0) {
        return 1;
      }
      pathIdx--;
      invocStackIdx--;
    } else { // stop condition
      if (listOut) {
        totalPath++;
        pathStack[pathIdx].stateNo = STOP_PATH_MARK;
        pathStack[pathIdx].tag = 0;
        pathStack[pathIdx].stackStateID = stackStateID;
        pathIdx++;
        if (outWordsOfGraph(pathIdx) != 0) {
          return 1;
        }
        pathIdx--;
      }
    }
  } // end if terminal node
  for (Transition *trans = a->states[stateNo]->transitions; trans != 0; trans = trans->next) {
    if (trans->tag_number & STOP_PATH_MARK) {
      if (listOut) {
        totalPath++;
        pathStack[pathIdx].stackStateID = stackStateID;
        pathStack[pathIdx].stateNo = STOP_PATH_MARK;
        pathStack[pathIdx].tag = trans->tag_number & ~STOP_PATH_MARK;
        pathIdx++;
        if (outWordsOfGraph(pathIdx) != 0) {
          return 1;
        }
        pathIdx--;
      }
      continue;
    }
    if (display_control == GRAPH) {
      if (listOut) {
        pathStack[pathIdx].stackStateID = stackStateID;
        pathStack[pathIdx].stateNo = trans->state_number;
        pathStack[pathIdx].tag = trans->tag_number;
        pathIdx++;
        if (exploreSubgraphRecursively(stackStateID, autoDepth, trans->state_number, stateDepth + 1) != 0) {
          return 1;
        }
        pathIdx--;
      }
      continue;
    }
    if (trans->tag_number & SUBGRAPH_PATH_MARK) { // handling sub graph call
      if (ignoreTable[trans->tag_number & SUB_ID_MASK]) {
        // find stop condition path
        if (listOut) {
          totalPath++;
          stopPath++;
          numOfIgnore[trans->tag_number & SUB_ID_MASK]++;
          pathStack[pathIdx].stackStateID = stackStateID;
          pathStack[pathIdx].tag = trans->tag_number;
          pathStack[pathIdx].stateNo = STOP_PATH_MARK;
          pathIdx++;
          if (outWordsOfGraph(pathIdx) != 0) {
            return 1;
          }
          pathIdx--;
        }
        continue;
      }
      //
      //    find cycle of calls
      //
      tmp = trans->tag_number & SUB_ID_MASK;

      // Scanning autoCallStack isn't necessary to find 
      // recursions as `isCycle()` is called for each
      // instance of exploreSubgraphRecursively
      
      /*int scanner;
      for (scanner = 0; scanner < autoDepth; scanner++) {
        if ( autoCallStack[scanner].tran->tag_number == trans->tag_number ) {
          break;
        }
      }*/
      
      
      
      autoCallStack[autoDepth].tran = trans; // add the transition to the stack
      //if (scanner == autoDepth) { // didn't find a recursive call
        callId = identifyStackState(autoDepth + 1);
      /*} else { // found an item in autoCallStack with same transitions as `trans`
        pathStack[pathIdx].tag = 0;
        pathStack[pathIdx].stackStateID = autoCallStack[autoDepth].stackStateID;
        pathStack[pathIdx].stateNo = a->initial_states[tmp] | LOOP_PATH_MARK;

        ++pathIdx;
        // check for a recursion that might not have been detected
        // in autoCallStack due to backtracking
        if (!isCyclePath(stateDepth)) { 
          // no cycle was found: we identify call stack
          // and continue the exploration
          callId = identifyStackState(autoDepth + 1);
          --pathIdx;
        } else {
          --pathIdx;
          continue; // skip current transition
        }
      }*/
      pathStack[pathIdx].tag = 0;
      pathStack[pathIdx].stackStateID = callId;
      pathStack[pathIdx].stateNo = a->initial_states[tmp];
      ++pathIdx;
      autoCallStack[autoDepth].stackStateID = callId;

      invocStackIdx++;
      invocStack[invocStackIdx].stackStateID = callId;
      invocStack[invocStackIdx].targetState = trans->state_number;
      if (exploreSubgraphRecursively(callId, autoDepth + 1, a->initial_states[tmp], stateDepth + 1) != 0) {
        return 1;
      }
      --pathIdx;
      --invocStackIdx;
      continue;
    }
    pathStack[pathIdx].stateNo = trans->state_number;
    pathStack[pathIdx].tag = trans->tag_number;
    pathStack[pathIdx].stackStateID = stackStateID;
    ++pathIdx;
    if (exploreSubgraphRecursively(stackStateID, autoDepth, trans->state_number, stateDepth + 1) != 0) {
      return 1;
    }
    pathIdx--;
  } // end for each transition
  return 0;
}

/**
 * Check if state of autoCallStack is consistent
 * with last call in invocStack
 * Used in exploreSubgraphRecursively to fix
 * previous call stack being erased
 * before backtracking
 */
int CFstApp::checkAutoCallStack() {
  int stackStateID = invocStack[invocStackIdx].stackStateID;
  struct transitionList *transitionListPtr;
  transitionListPtr = transitionListHead;
  int count = 0;
  while (transitionListPtr) { // search for the transitionList
    if (count == stackStateID) { // when we found it
      int i;
      for (i = 0; i < transitionListPtr->cnt; i++) {
        if( autoCallStack[i].tran != transitionListPtr->trans[i] ) {
          // correct autoCallStack if it's different
          autoCallStack[i].tran = transitionListPtr->trans[i];
        }
      }
      autoCallStack[i-1].stackStateID = stackStateID;
    }
    count++;
    transitionListPtr = transitionListPtr->next;
  }
  return 0;
}

//
//  for debugging, display all stack
//
void CFstApp::printPathNames(U_FILE *f) {
  int pidx = -1;
  int i;
  u_fprintf(f, "#");
  for (i = 0; i < pathIdx; i++) {
    if (pathStack[i].stackStateID != pidx) { // skip the same value
      pidx = pathStack[i].stackStateID;
      u_fprintf(f, "%S>", a->graph_names[pidx]);
    }
  }
}

void CFstApp::setGrammarMode(char* fst2_filename) {
  char* fst2_filename_cpy = (char*)malloc(sizeof(char) * (strlen(fst2_filename) + 1));
  if(fst2_filename_cpy == NULL){
    fatal_alloc_error("setGrammarMode");
  }
  remove_extension(fst2_filename, fst2_filename_cpy);
  OutputPolicy outputPolicy = MERGE_OUTPUTS;
  int export_in_morpho_dic;
  MatchPolicy matchPolicy;
  int l = (int)strlen(fst2_filename_cpy) - 1;
  analyse_fst2_graph_options(fst2_filename_cpy, l, &outputPolicy, &export_in_morpho_dic, &matchPolicy);
  if(outputPolicy == MERGE_OUTPUTS) {
    grammarMode = MERGE;
    prMode = PR_SEPARATION;
  }
  else if(outputPolicy == REPLACE_OUTPUTS) {
    grammarMode = REPLACE;
    prMode = PR_SEPARATION;
  }
  
  free(fst2_filename_cpy);
}

/**
 * takes a number (decimal or hex) in char and puts its value in 'val'
 */
unichar * uascToNum(unichar *uasc, int *val) {
  unichar *wordPtr = uasc;
  int base = 10;
  int sum = 0;

  // if number is hexadecimal
  if ((*wordPtr == (unichar) '0') && ((*(wordPtr + 1) == (unichar) 'x') 
        || (*(wordPtr + 1) == (unichar) 'X'))) {
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
 *
 * Go through pathStack to construct the words
 * return 1 when output limit has been reached, else 0
 */
int CFstApp::outWordsOfGraph(int depth) {
  int s;
  Fst2Tag Tag;
  unichar *suffixPtr = NULL;
  unichar *wordPtr = NULL;
  unichar *inputBufferPtr = NULL;  // buffer for box inputs
  unichar *outputBufferPtr = NULL; // buffer for box outputs
  unichar *chp = NULL;
  int indicateFirstUsed;
  int i;
  int return_value;  // return value when we try to access a value in the hash table
  int res;
  int markCtlChar, markPreCtlChar;
  depthDebug = pathIdx;
  inBufferCnt = outBufferCnt = 0;
  inputPtrCnt = outputPtrCnt = 0;
  unichar aaBuffer_for_getLabelNumber[64];
  unichar *var_dic_name = NULL;
  struct any *value; // the value retrieved from the hash table
  bool isWord = false;  // false if the tag content is not a word (like $< or $>)
  //  fini = (tagQ[tagQidx - 1] & (SUBGRAPH_PATH_MARK | LOOP_PATH_MARK)) ?
  //    tagQ[tagQidx -1 ]:0;
  //
  //  elimine the value signified repete
  //
  markCtlChar = markPreCtlChar = 0;
  indicateFirstUsed = 0;
  count_in_line = 0;
  //printPathStack();

  for (s = 0; s < pathIdx; s++) {
    res = -1;
    return_value = -1;
    value = NULL;
    inputBuffer[inputPtrCnt] = outputBuffer[outputPtrCnt] = 0;
    if (!pathStack[s].tag) {
      inputBufferPtr = outputBufferPtr = u_null_string;
    }
    else if (pathStack[s].tag & SUBGRAPH_PATH_MARK) {
      inputBufferPtr = (display_control == GRAPH) ?
                       (unichar *) a->graph_names[pathStack[s].tag & SUB_ID_MASK] : u_null_string;
      outputBufferPtr = u_null_string;
    }
    else {
      Tag = a->tags[pathStack[s].tag & SUB_ID_MASK];
      get_value(path_to_stop, Tag, HT_DONT_INSERT, &return_value);
      if(return_value == HT_KEY_ALREADY_THERE) {
        break;
      }
      isWord = false;
      switch (Tag->type) { // check if the current node is a morphological begin or end, and update the boolean to begin/stop the morphological mode
        case BEGIN_MORPHO_TAG :
          inMorphoMode = true;
          break;
        case END_MORPHO_TAG :
          inMorphoMode = false;
          appendSingleSpace(); // insert one space between the last word of the morphological mode and the next word
          continue;
        case BEGIN_OUTPUT_VAR_TAG :
          set_output_variable_pending(p->output_variables,Tag->variable);
          break;
        case END_OUTPUT_VAR_TAG :
          unset_output_variable_pending(p->output_variables,Tag->variable);
          break;
        case BEGIN_VAR_TAG :
          set_output_variable_pending(input_variables,Tag->variable);
          break;
        case END_VAR_TAG :
          unset_output_variable_pending(input_variables,Tag->variable);
          break;
        case UNDEFINED_TAG:
          isWord = true;
          if(p->output_variables->pending != NULL) {
            res = add_raw_string_to_output_variables(p->output_variables, Tag->output);
          }
          if(input_variables->pending != NULL) {
            res = add_raw_string_to_output_variables(input_variables, Tag->input);
          }
          break;
        default :
          break;
      }

      // ignore the tag if his input is not a word (like morphological end and begin tags)
      if(isWord) {
        inputBufferPtr = (u_strcmp(Tag->input, u_epsilon_string)) ?
                         Tag->input : u_null_string;
        if (Tag->output != NULL) {
          outputBufferPtr = (u_strcmp(Tag->output, u_epsilon_string)) ? Tag->output : u_null_string;
          if(!u_strcmp(Tag->output, "/")) {  // if the output is '/', it's a MDG, this output is not put in the outputfile
            isMdg = true;
            outputBufferPtr = u_null_string;
          }
          else if(res > 0) {
            outputBufferPtr = u_null_string;
          }
          else{
            value = get_value(dela_entries, Tag, HT_DONT_INSERT, &return_value);
            if(return_value == HT_KEY_ALREADY_THERE && Tag->output[0] == (unichar)'$'
            && Tag->output[u_strlen(Tag->output) - 1] == (unichar)'$')
            {  // if the tag contains a dela_entry and if the output contains a variable name, put that dela_entry in the dic_var
              var_dic_name = u_strdup(Tag->output);
              var_dic_name[u_strlen(Tag->output) - 1] = '\0';
              var_dic_name++;
              struct dela_entry *entry = (dela_entry *)value->_ptr;
              set_dic_variable(var_dic_name, entry, &(p->dic_variables), 1);
              var_dic_name--;
              free(var_dic_name);
              if(grammarMode == NONE){
                outputBufferPtr = u_null_string;
              }
              else{
                inputBufferPtr = entry->inflected;
                outputBufferPtr = u_null_string;
              }
            }
            else {  //In the other, check if the output is a input/output variable call in the tag's output
              // to render literal outputs
              extended_output_render r;

              // process the output
              if(!process_extended_output(Tag->output, p, 0, &r, input_variables)) {
                break;  // process_output may returns 0 in the case of unsatisfied equations
              }

              // there is no more chars to add to the output template,
              // hence we put a mark to indicate the end of the string
              if (!is_empty(r.stack_template)) {
                push(r.stack_template, '\0');
              }

              // prepare the output template to be rendered
              r.prepare();

              // copy the passed output into the literal output stack
              int captured_chars = 0;
              append_literal_output(r.render(0), p, &captured_chars);

              if(p->literal_output == NULL){
                outputBufferPtr = u_null_string;
              }
              else{
                outputBufferPtr = (u_strcmp(p->literal_output->buffer, u_epsilon_string)) ?
                  p->literal_output->buffer : u_null_string;
                p->literal_output->buffer[p->literal_output->top+1]='\0';
                empty(p->literal_output);
              }
            }
          }
        }
        else {
          outputBufferPtr = u_null_string;
        }
      }
    }
    markCtlChar = 0;
    // mark control character
    if (!(pathStack[s].stateNo & STOP_PATH_MARK) && !wordMode && (*inputBufferPtr == '<')) {
      chp = inputBufferPtr + 1;
      while (*chp) {
        chp++;
      }
      --chp;
      if (*chp == (unichar) '>') {
        markCtlChar = 1;
        //               if(inputPtrCnt || outputPtrCnt) outOneWord(0);
        //               else if(control_char) outOneWord(0);
        //                   control_char = 1;
        //             while(*inputBufferPtr) inputBuffer[inputPtrCnt++] = *inputBufferPtr++;
        //           while(*outputBufferPtr) outputBuffer[outputPtrCnt++] = *outputBufferPtr++;
        //           continue;
      }
    }

    if (pathStack[s].stateNo & LOOP_PATH_MARK) {
      if (recursiveMode == LABEL) {
        if (*inputBufferPtr || *outputBufferPtr) { // current
          if (outputPtrCnt) {
            if (outOneWord(0) != 0) {
              return 1;
            }
          }
          if (markPreCtlChar && *inputBufferPtr) {
            if (outOneWord(0) != 0) {
              return 1;
            }
          }
          while (*inputBufferPtr){
            inputBuffer[inputPtrCnt++] = *inputBufferPtr++;
          }
          if (automateMode == TRANMODE) {
            while (*outputBufferPtr) {
              outputBuffer[outputPtrCnt++] = *outputBufferPtr++;
            }
          }
          if (wordMode) {
            if (inputPtrCnt || outputPtrCnt) {
              if (outOneWord(0) != 0) {
                return 1;
              }
            }
          }
          //                    else {
          //                  if(control_char) outOneWord(0);
          //                }

        }
        if (pathStack[s].stateNo & STOP_PATH_MARK) {
          suffixPtr = getLabelNumber(aaBuffer_for_getLabelNumber, depth,indicateFirstUsed, s, 0);
          if (outOneWord(suffixPtr) != 0) {
            return 1;
          }
          break;
        }
        suffixPtr = getLabelNumber(aaBuffer_for_getLabelNumber, s,indicateFirstUsed, s, 1);
        if (!indicateFirstUsed) { // first print out
          if (outOneWord(suffixPtr) != 0) {
            return 1;
          }
        } else {
          resetBufferCounters();
        }
        while (*suffixPtr){
          INPUTBUFFER[inBufferCnt++] = *suffixPtr++;
        }
        wordPtr = closingQuote;
        while (*wordPtr) {
          INPUTBUFFER[inBufferCnt++] = *wordPtr++;
        }
        markPreCtlChar = markCtlChar;
        continue;
      } else if (recursiveMode == SYMBOL) { // SYMBOL
        if (outputPtrCnt && wordMode) {
          if (outOneWord(0) != 0) {
            return 1;
          }
        }
        wordPtr = openingQuote;
        while (*wordPtr) {
          if (automateMode == TRANMODE) {
            outputBuffer[outputPtrCnt++] = *wordPtr;
          }
          inputBuffer[inputPtrCnt++] = *wordPtr;
          wordPtr++;
        }
        while (*inputBufferPtr){
          inputBuffer[inputPtrCnt++] = *inputBufferPtr++;
        }
        inputBuffer[inputPtrCnt++] = (unichar) '|';
        if (automateMode == TRANMODE) {
          while (*outputBufferPtr){
            outputBuffer[outputPtrCnt++] = *outputBufferPtr++;
          }
          outputBuffer[outputPtrCnt++] = (unichar) '|';
        }
        struct cyclePathMark *h = headCyc;
        int findId = pathStack[s].stateNo & PATHID_MASK;
        while (h) {
          for (i = 0; i < h->pathCnt; i++) {
            if (h->pathTagCopy[i].stateNo == findId) {
              break;
            }
          }
          if (i != h->pathCnt) {
            if (automateMode == TRANMODE) {
              inputBuffer[inputPtrCnt++] = (unichar) 'C';
              outputBuffer[outputPtrCnt++] = (unichar) 'C';
              putInt(1, h->index);
              inputBuffer[inputPtrCnt++] = (unichar) '|';
              outputBuffer[outputPtrCnt++] = (unichar) '|';
            } else {
              inputBuffer[inputPtrCnt++] = (unichar) 'C';
              putInt(0, h->index);
              inputBuffer[inputPtrCnt++] = (unichar) '|';
            }
          }
          h = h->next;
        }
        if (automateMode == TRANMODE) {
          --outputPtrCnt;
        }
        --inputPtrCnt;
        wordPtr = closingQuote;
        while (*wordPtr) {
          if (automateMode == TRANMODE) {
            outputBuffer[outputPtrCnt++] = *wordPtr;
          }
          inputBuffer[inputPtrCnt++] = *wordPtr;
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
        if (!(pathStack[s].stateNo & STOP_PATH_MARK)) {
          // mark the stop
          wordPtr = openingQuote;
          while (*wordPtr) {
            INPUTBUFFER[inBufferCnt++] = *wordPtr;
            if ((automateMode == TRANMODE) && (prMode
                == PR_SEPARATION)) {
              OUTPUTBUFFER[outBufferCnt++] = *wordPtr;
            }
            wordPtr++;
          }
        }
        while (*inputBufferPtr) {
          inputBuffer[inputPtrCnt++] = *inputBufferPtr++;
        }
        if (automateMode == TRANMODE) {
          while (*outputBufferPtr) {
            outputBuffer[outputPtrCnt++] = *outputBufferPtr++;
          }
        }
        if (pathStack[s].stateNo & STOP_PATH_MARK) {
          wordPtr = closingQuote;
          while (*wordPtr) {
            if (automateMode == TRANMODE) {
              outputBuffer[outputPtrCnt++] = *wordPtr;
              if (prMode == PR_SEPARATION) {
                inputBuffer[inputPtrCnt++] = *wordPtr;
              }
            } else {
              inputBuffer[inputPtrCnt++] = *wordPtr;
            }

            wordPtr++;
          }
          if (outOneWord(u_null_string) != 0) {
            return 1;
          }
        } else {
          if (wordMode) {
            if (inputPtrCnt || outputPtrCnt) {
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
    } // end if LOOP_PATH_MARK

    if (pathStack[s].stateNo & STOP_PATH_MARK) {
      if (markPreCtlChar && markCtlChar) {
        if (outOneWord(0) != 0) {
          return 1;
        }
      }
      if ((automateMode == TRANMODE) && *outputBufferPtr) { // current
        if (outputPtrCnt) {
          if (outOneWord(0) != 0) {
            return 1;
          }
        }
        while (*outputBufferPtr) {
          outputBuffer[outputPtrCnt++] = *outputBufferPtr++;
        }
      }
      if (pathStack[s].tag & SUBGRAPH_PATH_MARK) {
        if (outOneWord((unichar *) a->graph_names[pathStack[s].tag
            & SUB_ID_MASK]) != 0) {
          return 1;
        }
      } else {
        if (outOneWord(u_null_string) != 0) {
          return 1;
        }
      }
      break;
    } // end if STOP_PATH_MARK

    if (pathStack[s].tag & SUBGRAPH_PATH_MARK) {
      if (outputPtrCnt || (markPreCtlChar && *inputBufferPtr)) {
        if (outOneWord(0) != 0) {
          return 1;
        }
      }
      switch (display_control) {
      case GRAPH:
        inputBuffer[inputPtrCnt++] = (unichar) '{';
        while (*inputBufferPtr){
          inputBuffer[inputPtrCnt++] = *inputBufferPtr++;
        }
        inputBuffer[inputPtrCnt++] = (unichar) '}';
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
    } // end if SUBGRAPH_PATH_MARK
    // make a pair of (input, output)
    if ((*inputBufferPtr == 0) && (*outputBufferPtr == 0)) {
      continue;
    }

    if (outputPtrCnt || (markPreCtlChar && *inputBufferPtr)) {
      if (outOneWord(0) != 0) {
        return 1;
      }
    }
    while (*inputBufferPtr){
      inputBuffer[inputPtrCnt++] = *inputBufferPtr++;
    }
    if (automateMode == TRANMODE) {
      while (*outputBufferPtr) {
        outputBuffer[outputPtrCnt++] = *outputBufferPtr++;
      }
    }
    if (wordMode) {
      if (inputPtrCnt || outputPtrCnt) {
        if (outOneWord(0) != 0) {
          return 1;
        }
      }
    }
    //         else {
    //      if(control_char) outOneWord(0);
    //    }
    markPreCtlChar = markCtlChar;
  } // end for 's = 0; s < pathIdx; s++'
  return 0;
}

//
//
//

const char* optstring_Fst2List=":o:Sp:a:t:l:i:mdf:vVKPhs:q:r:c:g:D:Q:E:";
const struct option_TS lopts_Fst2List[]= {
  {"output",required_argument_TS,NULL,'o'},
  {"ignore_outputs",required_argument_TS,NULL,'a'},
  {"allow_outputs",required_argument_TS,NULL,'t'},
  {"limit",required_argument_TS,NULL,'l'},
  {"stop_subgraph",required_argument_TS,NULL,'i'},
  {"paths_print_mode",required_argument_TS,NULL,'p'},
  {"word_mode",no_argument_TS,NULL,'m'},
  {"disable_loop_check",no_argument_TS,NULL,'d'},
  {"output_format",required_argument_TS,NULL,'f'},
  {"lr_delimiters",required_argument_TS,NULL,'s'},
  {"verbose",no_argument_TS,NULL,'v'},
  {"print",no_argument_TS,NULL,'S'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"cycles_format",required_argument_TS,NULL,'r'},
  {"unicode",required_argument_TS,NULL,'c'},
  {"io_separator",required_argument_TS,NULL,'g'},
  {"stop_mark",required_argument_TS,NULL,'Q'},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"korean",no_argument_TS,NULL,'K'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {"make_dictionary",no_argument_TS,NULL,'P'},
  {"help",no_argument_TS,NULL,'h'},
  {"binary dics",required_argument_TS,NULL,'D'},
  {"elg_extensions_path",required_argument_TS,NULL,'E'},
  {NULL,no_argument_TS,NULL,0}
};

int main_Fst2List(int argc, char* const argv[]) {
  char* ofilename = NULL;
  char morpho_dic[1025] = "";

  unichar changeStrTo[16][MAX_CHANGE_SYMBOL_SIZE];
  int changeStrToIdx;

  CFstApp aa;

  changeStrToIdx = 0;

  char* wordPtr     = NULL;
  unichar* wordPtr2 = NULL;
  unichar* wordPtr3 = NULL;

  char fst2_filename[FILENAME_MAX];
  fst2_filename[0] = '\0';
  int val,index=-1;
  bool only_verify_arguments = false;
  UnitexGetOpt options;
  VersatileEncodingConfig vec = VEC_DEFAULT;
  bool makeDic = false;

  char elg_extensions_path[FILENAME_MAX]="";

  while (EOF!=(val=options.parse_long(argc,argv,optstring_Fst2List,lopts_Fst2List,&index))) {
    switch(val) {
    case 'f': 
      if (options.vars()->optarg[0] == '\0') {
        error("You must specify a mode : s/a\nSeparation or Together\n");
        return USAGE_ERROR_CODE;
      }
      switch(options.vars()->optarg[0]) {
        case 's': aa.prMode = PR_SEPARATION; break;
        case 'a': aa.prMode = PR_TOGETHER; break;
        default:
          error("Invalid output format (-f), valid values are \"a\" or \"s\": rerun with --help\n");
          return USAGE_ERROR_CODE;
      }
      break;
    case 'd':
      aa.enableLoopCheck = false;
      break;
    case 'D':
      // load morphological dictionaries
      if (options.vars()->optarg[0]!='\0') {
        if (strcmp(morpho_dic, "") == 0) {
          strcpy(morpho_dic, options.vars()->optarg);
        }
        else {
          strcat(morpho_dic,";");
          strcat(morpho_dic,options.vars()->optarg);
        }
        aa.morphDicCnt++;
      }
      break;
   case 'E':
      if (options.vars()->optarg[0]=='\0') {
        error("You must specify a non empty ELGs path\n");
        return USAGE_ERROR_CODE;
      }
      strcpy(elg_extensions_path,options.vars()->optarg);
      break;
    case 'P':
      makeDic = true;
      break;
    case 'S':
      ofilename = (char *)malloc((strlen(MAGIC_OUT_STDOUT) + 1) * sizeof(char));
      if(ofilename == NULL){
         fatal_alloc_error("main_Fst2List");
      }
      strcpy(ofilename, MAGIC_OUT_STDOUT);
      break;
    case 'o': // set a name for the output file
      ofilename = (char *)malloc((strlen((char*)&options.vars()->optarg[0]) + 1) * sizeof(char));
      if(ofilename == NULL){
        fatal_alloc_error("main_Fst2List");
      }
      strcpy(ofilename, (char*) &options.vars()->optarg[0]);
      break;
    case 'l':
      aa.outLineLimit = atoi((char*)&options.vars()->optarg[0]);
      break;
    case 'i':
      aa.stopExploList((char*)&options.vars()->optarg[0]);
      break;
    case 'K':
      aa.isKorean = true;
      break;
    case 'I':
      aa.stopExploListFile((char*)&options.vars()->optarg[0]);
      break;
    case 'p':
      switch (options.vars()->optarg[0]) {
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
      // FALLTHROUGH INTENDED
    case 't':
      aa.automateMode = (val == 't') ? TRANMODE : AUTOMODE;
      aa.grammarMode = NONE;
      switch (options.vars()->optarg[0]) {
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
      aa.wordMode = 0;
      break;
    case 'r':
      switch (options.vars()->optarg[0]) { 
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
      // we consume more than 1 argument which is not expected by getopt
      // we need to manually increment optind
      options.vars()->optind++;
      // parse the "L[,R]" string
      aa.saveEntre = (unichar *)malloc((strlen(&options.vars()->optarg[1]) + 1) * sizeof(unichar));
      if(aa.saveEntre == NULL){
        fatal_alloc_error("main_Fst2List");
      }
      wordPtr = (char*) &options.vars()->optarg[2];
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

      aa.openingQuote = aa.saveEntre;
      if (wordPtr3) {
        *wordPtr3++ = 0;
        aa.closingQuote = wordPtr3;
      } else aa.closingQuote = wordPtr2;
      break;  // end case 'r'
    case 'c':
      if (!changeStrToVal(changeStrToIdx, changeStrTo, (char*) &options.vars()->optarg[0])) {
        break;
      }
      error("Invalid arguments: rerun with --help\n");
      return USAGE_ERROR_CODE;
    case 'g': // option '--io_separator'
      io_separator:
      if(val=='g') { // check the deprecated option '-s0' wasn't used
        wordPtr = (char*) &options.vars()->optarg[0];
      }
      wordPtr3 = 0;
      wordPtr2 = aa.saveSep = (unichar *)malloc((strlen(wordPtr) + 1) * sizeof(unichar));
      if(aa.saveSep == NULL){
        fatal_alloc_error("main_Fst2List");  
      }
      while (*wordPtr) {
        if ((*wordPtr < 0x20) || (*wordPtr > 0x7e)) {
          error("Use a separator in ASC code\r\n");
          return DEFAULT_ERROR_CODE;
        }
        if (*wordPtr == '\\') {
          wordPtr++;
          if (*wordPtr == '\0') {
            error("You must specify a separator\n");
            return DEFAULT_ERROR_CODE;
          }
        }
        if (*wordPtr == '"')
          continue;
        *wordPtr2++ = (unichar) *wordPtr++;
      }
      *wordPtr2 = 0;
      break;

    case 'Q':  // option '--stop_mark'
      stop_mark:
      wordPtr = (char*) &options.vars()->optarg[1];
      wordPtr3 = 0;
      wordPtr2 = aa.stopSignal = (unichar *)malloc((strlen(wordPtr) + 3) * sizeof(unichar));
      if(aa.stopSignal == NULL){
        fatal_alloc_error("main_Fst2List");
      }
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
    case 's':
    {
      // supports the deprecated options '-ss' and '-s0'
      switch (options.vars()->optarg[0]) { 
      case '0':
        u_printf("Warning: '-s0' is deprecated, use '--io_separator' instead\n");
        // manually increment optind to consume more args than expected by getopt
        options.vars()->optind++;
        wordPtr = argv[options.vars()->optind-1];
        // goto the correct switch case to avoid code duplication
        goto io_separator;
      case 's':
        u_printf("Warning: '-ss' is deprecated, use '--stop-mark' instead\n");
        // manually increment optind to consume more args than expected by getopt
        options.vars()->optind++;
        // goto the correct switch case to avoid code duplication
        goto stop_mark;
      } // end switch deprecated option

      wordPtr = (char*) &options.vars()->optarg[1] - 1;
      wordPtr3 = 0;
      wordPtr2 = aa.sep1 = (unichar *)malloc((strlen(wordPtr) + 1) * sizeof(unichar));
      if(aa.sep1 == NULL){
       fatal_alloc_error("main_Fst2List");
      }
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
            error("You must specify a separator");
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
    } // end case 's'
    case 'k':
      if (options.vars()->optarg[0] == '\0') {
        error("Empty input_encoding argument\n");
        return USAGE_ERROR_CODE;
      }
      decode_reading_encoding_parameter(&(vec.mask_encoding_compatibility_input), &options.vars()->optarg[0]);
      break;
    case 'V': only_verify_arguments = true; break;
    case 'h': usage(); return SUCCESS_RETURN_CODE;
    case 'q': {
      char* arg = NULL;
      if (options.vars()->optarg[0] == '\0') {
        error("couldn't get the argument for option 'q'\n");
        return USAGE_ERROR_CODE;
      } else {
        arg = (char*)&options.vars()->optarg[0];
      }
      decode_writing_encoding_parameter(&(vec.encoding_output),
          &(vec.bom_output), arg);
      break;
    }
    case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                          error("Missing argument for option --%s\n",lopts_Fst2List[index].name);
              return USAGE_ERROR_CODE;
    case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt) :
                          error("Invalid option --%s\n",options.vars()->optarg);
              return USAGE_ERROR_CODE;
    } // end switch
    index=-1;
  } // end while

  // test each option has at most one argument
  if (options.vars()->optind != argc-1) {
    error("Invalid arguments: rerun with --help\n");
    return USAGE_ERROR_CODE;
  }

  if (only_verify_arguments) {
    free(ofilename);
    return SUCCESS_RETURN_CODE;
  }

  strcpy(fst2_filename,argv[options.vars()->optind]);
  aa.fileNameSet(argv[options.vars()->optind], ofilename);
  aa.vec = vec;

  // --------------------------------------------------------------------------
  // TODO() refactor this code in a single function together with Locate.cpp:610
  // --------------------------------------------------------------------------
  // if the path of the ELGs extensions is not given,
  // a default path is calculated
  if (elg_extensions_path[0] == '\0') {
    // the current executable path
    char exec_path[FILENAME_MAX] = "";
    // App
    get_exec_path(exec_path);
    // App/
    add_path_separator(exec_path);
    // App/elg
    strcat(exec_path, ELG_FUNCTION_DEFAULT_SCRIPT_DIR_NAME);
    // Copy back
    strcpy(elg_extensions_path, exec_path);
  }
  // --------------------------------------------------------------------------

  // --------------------------------------------------------------------------
  // TODO() refactor this code in a single function together with LocatePattern.cpp:267
  // check if the ELGs path exists and is a directory
  if(!is_directory(elg_extensions_path)) {
    error("ELG error: %s directory doesn't exist\n", elg_extensions_path);
    return 0;
  }

  // get the real scripts path
  char real_elg_extensions_path[FILENAME_MAX]="";
  get_real_path(elg_extensions_path, real_elg_extensions_path);

  // Make sure that the ELGs path always ends with a path separator
  add_path_separator(real_elg_extensions_path);

  // Check if the ELG init function exists
  char script_init_name[FILENAME_MAX]   = { };
  char script_init_file[FILENAME_MAX]   = { };

  // script name = extension_name.upp
  strcat(script_init_name, ELG_FUNCTION_DEFAULT_SCRIPT_INIT_NAME);
  strcat(script_init_name, ELG_FUNCTION_DEFAULT_EXTENSION);

  // script_file = /default/path/extension_name.upp
  strcat(script_init_file, real_elg_extensions_path);
  strcat(script_init_file, script_init_name);

  // throw an error if the init script do not exist
  if (!is_regular_file(script_init_file)) {
    error("ELG error: %s doesn't exist. Please create at least an empty file\n", script_init_file);
    return USAGE_ERROR_CODE;
  }
  // --------------------------------------------------------------------------

  aa.p = new_locate_parameters(real_elg_extensions_path);
  (*aa.p->literal_output->buffer) = '\0';
  load_morphological_dictionaries(&aa.vec, morpho_dic, aa.p);
  if(makeDic) {
    aa.setGrammarMode(fst2_filename);
  }
  aa.getWordsFromGraph(changeStrToIdx, changeStrTo, fst2_filename);

  free(ofilename);

  free_stack_unichar(aa.p->literal_output);
  free_stack_unichar(aa.p->stack_elg);
  free(aa.p->morpho_dic_bin_free);
  free(aa.p->morpho_dic_inf_free);
  clear_dic_variable_list(&aa.p->dic_variables);
  aa.p->dic_variables = NULL;
  free_locate_parameters(aa.p);

  aa.path_to_stop->free_key = NULL;
  aa.dela_entries->free_key = NULL;
  free_hash_table(aa.path_to_stop);
  free_hash_table(aa.dela_entries);

  return SUCCESS_RETURN_CODE;
}

} // namespace unitex
