#ifndef GLOBAL_H
#define GLOBAL_H

#include "lex.h"
#include "parse.h"
#include "fatal.h"
#include "loadfile.h"
#include "colorcode.h"

/**********
 * MACROS *
 **********/
#define EXECNAME "splt"
#define ENPREFIX EXECNAME ": "
#
#define ERR(msg) fatal(ENPREFIX msg)
#define VERR(msg, ...) vfatal(ENPREFIX msg, __VA_ARGS__)
#
#define ESIZ(arr) (sizeof (arr)[0])
#define ARRLEN(arr) (sizeof (arr) / ESIZ(arr))
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

/************
 * TYPEDEFS *
 ************/
typedef void teller_t(void);
typedef void msg_template_t(
   teller_t *tell,
   const char * restrict color,
   const char * restrict msg
);

/***********************
 * FUNCTION PROTOTYPES *
 ***********************/
msg_template_t err_template;
msg_template_t warn_template;

/**********************
 * EXTERNAL VARIABLES *
 **********************/
extern const char *sfname;   // from optprocessor.c
extern arr_t *ls;            // from splt.c
extern tree_t *pt;           // from splt.c

#endif
