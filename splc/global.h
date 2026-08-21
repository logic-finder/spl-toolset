#ifndef GLOBAL_H
#define GLOBAL_H

#include "lex.h"
#include "msg.h"
#include "parse.h"
#include "loadfile.h"
#include "colorcode.h"

/**********
 * MACROS *
 **********/
#define EXECNAME "splc"
#define ENPREFIX EXECNAME ": "

#define LONGJMP_TEMPLET(e,v) longjmp((e),(v))

#define IR_EXTENSION  ".splasm"
#define OBJ_EXTENSION ".splo"
#define C_EXTENSION   ".c"

/*==========*
 | KEYWORDS |
 *==========*/
#define KEYWRD_ACT     "Act"     // fixme: use lowercase!!
#define KEYWRD_SCENE   "Scene"   // fixme: use lowercase!!
#define KEYWRD_ACT_L   "act"
#define KEYWRD_SCENE_L "scene"

#define KEYWRD_ENTER  "Enter"    // fixme: use lowercase!!
#define KEYWRD_EXIT   "Exit"     // fixme: use lowercase!!
#define KEYWRD_EXEUNT "Exeunt"   // fixme: use lowercase!!

#define KEYWRD_OPEN   "Open"     // fixme: use lowercase!!
#define KEYWRD_SPEAK  "Speak"    // fixme: use lowercase!!
#define KEYWRD_LISTEN "Listen"   // fixme: use lowercase!!
#define KEYWRD_HEART  "heart"
#define KEYWRD_MIND   "mind"

#define KEYWRD_LET    "Let"      // fixme: use lowercase!!
#define KEYWRD_WE     "We"       // fixme: use lowercase!!
#define KEYWRD_US     "us"
#define KEYWRD_SHALL  "shall"
#define KEYWRD_MUST   "must"
#define KEYWRD_RETURN "return"
#define KEYWRD_PROCED "proceed"

#define KEYWRD_IF     "If"          // fixme: use lowercase!!

#define KEYWRD_REMEMB "Remember"    // fixme: use lowercase!!
#define KEYWRD_RECALL "Recall"      // fixme: use lowercase!!

#define KEYWRD_TO     "to"
#define KEYWRD_AND    "and"
#define KEYWRD_SO     "so"
#define KEYWRD_NOT    "not"
#define KEYWRD_THAN   "than"
#define KEYWRD_AS     "as"

/* Operators */
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

/* Articles */
#define KEYWRD_A      "a"
#define KEYWRD_AN     "an"
#define KEYWRD_THE    "the"

/* Be Verb Conjugations */
#define KEYWRD_AM     "Am"    // fixme: use lowercase!!
#define KEYWRD_ARE    "are"
#define KEYWRD_ART    "art"
#define KEYWRD_ARE_C  "Are"   // fixme: use lowercase!!
#define KEYWRD_ART_C  "Art"   // fixme: use lowercase!!
#define KEYWRD_IS     "Is"    // fixme: use lowercase!!
#define KEYWRD_BE     "be"

/* Nil */
#define KEYWRD_NOTHING "nothing"
#define KEYWRD_ZERO    "zero"

/* Pronouns */
/* 1st person */
#define KEYWRD_I      "I"     // fixme: use lowercase!!
#define KEYWRD_ME     "me"
/* 2nd person */
#define KEYWRD_THEE   "thee"
#define KEYWRD_THOU_L "thou"
#define KEYWRD_YOU_L  "you"
#define KEYWRD_YOU    "You"   // fixme: use lowercase!!
#define KEYWRD_THOU   "Thou"  // fixme: use lowercase!!

/* Possessives */
/* 1st person */
#define KEYWRD_MINE   "mine"  /* archaic usage: mine eyes! */
#define KEYWRD_MY     "my"
/* 2nd person */
#define KEYWRD_THINE  "thine"
#define KEYWRD_THY    "thy"
#define KEYWRD_YOUR   "your"
#define KEYWRD_YOUR_U "YOUR"  // fixme: use lowercase!!
/* 3rd person */
#define KEYWRD_HIS    "his"
#define KEYWRD_HER    "her"
#define KEYWRD_ITS    "its"
#define KEYWRD_THEIR  "their"

/* Reflexives */
/* 1st person */
#define KEYWRD_MYSELF "myself"
/* 2nd person */
#define KEYWRD_TYSELF "thyself"
#define KEYWRD_YRSELF "yourself"

#endif
