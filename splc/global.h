#ifndef GLOBAL_H
#define GLOBAL_H

#include <inttypes.h>

#include "lex.h"
#include "msg.h"
#include "parse.h"
#include "common.h"
#include "loadfile.h"
#include "colorcode.h"

/**********
 * MACROS *
 **********/
#define EXECNAME "splc"
#define ENPREFIX EXECNAME ": "
#
#define LONGJMP_TEMPLET(e,v) longjmp((e),(v))
#
#define KEYWRD_ACT    "Act"
#define KEYWRD_SCENE  "Scene"
#define KEYWRD_ENTER  "Enter"
#define KEYWRD_EXIT   "Exit"
#define KEYWRD_EXEUNT "Exeunt"
#define KEYWRD_OPEN   "Open"
#define KEYWRD_SPEAK  "Speak"
#define KEYWRD_LISTEN "Listen"
#define KEYWRD_LET    "Let"
#define KEYWRD_WE     "We"
#define KEYWRD_IF     "If"
#define KEYWRD_REMEMB "Remember"
#define KEYWRD_RECALL "Recall"
#define KEYWRD_HEART  "heart"
#define KEYWRD_TO     "to"
#define KEYWRD_MIND   "mind"
#define KEYWRD_AND    "and"
#define KEYWRD_SO     "so"
#define KEYWRD_NOT    "not"
#define KEYWRD_THAN   "than"
#define KEYWRD_AS     "as"
#define KEYWRD_SUM    "sum"
#define KEYWRD_DIFF   "difference"
#define KEYWRD_PROD   "product"
#define KEYWRD_QUOT   "quotient"
#define KEYWRD_REM    "remainder"
#define KEYWRD_SQUR   "square"
#define KEYWRD_ROOT   "root"
#define KEYWRD_CUBE   "cube"
#define KEYWRD_2X     "twice"
#define KEYWRD_FACT   "factorial"
#define KEYWRD_OF     "of"
#define KEYWRD_BTW    "between"
#define KEYWRD_I      "I"
#define KEYWRD_MY     "my"
#define KEYWRD_ME     "me"
#define KEYWRD_MYSELF "myself"
#define KEYWRD_YOU    "You"
#define KEYWRD_THOU   "Thou"
#define KEYWRD_YOU_L  "you"
#define KEYWRD_THOU_L "thou"
#define KEYWRD_YOUR   "your"
#define KEYWRD_YOUR_U "YOUR"
#define KEYWRD_YRSELF "yourself"
#define KEYWRD_THY    "thy"
#define KEYWRD_THINE  "thine"
#define KEYWRD_THEE   "thee"
#define KEYWRD_TYSELF "thyself"
#define KEYWRD_HIS    "his"
#define KEYWRD_HER    "her"
#define KEYWRD_ITS    "its"
#define KEYWRD_A      "a"
#define KEYWRD_AN     "an"
#define KEYWRD_THE    "the"
#define KEYWRD_AM     "Am"
#define KEYWRD_ARE    "are"
#define KEYWRD_ART    "art"
#define KEYWRD_ARE_C  "Are"
#define KEYWRD_ART_C  "Art"
#define KEYWRD_IS     "Is"
#define KEYWRD_US     "us"
#define KEYWRD_SHALL  "shall"
#define KEYWRD_MUST   "must"
#define KEYWRD_RETURN "return"
#define KEYWRD_PROCED "proceed"

#define IR_EXTENSION  ".splasm"
#define OBJ_EXTENSION ".splo"

#define  SPL_INT_SIZ 4
#if   SPL_INT_SIZ == 4
#define SPL_INT_MAX INT32_MAX
#define SPL_INT_MIN INT32_MIN
#define SPL_UINT_FMTSPC     PRIu32
#define SPL_UINT_FMTSPC_HEX PRIX32
typedef int32_t  spl_int_t;
typedef uint32_t spl_uint_t;
#elif SPL_INT_SIZ == 8
#define SPL_INT int64_t
#define SPL_INT_MAX INT64_MAX
#define SPL_INT_MIN INT64_MIN
#define SPL_UINT_FMTSPC     PRIu64
#define SPL_UINT_FMTSPC_HEX PRIX64
typedef int64_t  spl_int_t;
typedef uint64_t spl_uint_t;
#else
#error undefined int size
#endif

/* fixme: 여기 말고, 각자의 h파일로 옮길것 */
/**********************
 * EXTERNAL VARIABLES *
 **********************/
extern const char *sfname;   // from optprocessor.c
extern msg_t msgs;           // from msg.c
extern arr_t *ls;            // from splc.c
extern tree_t *pt;           // from splc.c
extern tree_t *irt;          // from irgen.c

#endif
