#ifndef GLOBAL_H
#define GLOBAL_H

#include "lex.h"
#include "msg.h"
#include "parse.h"
#include "loadfile.h"
#include "colorcodes.h"

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
#define KEYWRD_ACT    "Act"
#define KEYWRD_SCENE  "Scene"

#define KEYWRD_ENTER  "Enter"
#define KEYWRD_EXIT   "Exit"
#define KEYWRD_EXEUNT "Exeunt"

#define KEYWRD_OPEN   "Open"
#define KEYWRD_SPEAK  "Speak"
#define KEYWRD_LISTEN "Listen"
#define KEYWRD_HEART  "heart"
#define KEYWRD_MIND   "mind"

#define KEYWRD_LET    "Let"
#define KEYWRD_WE     "We"
#define KEYWRD_US     "us"
#define KEYWRD_SHALL  "shall"
#define KEYWRD_MUST   "must"
#define KEYWRD_RETURN "return"
#define KEYWRD_PROCED "proceed"

#define KEYWRD_IF     "If"

#define KEYWRD_REMEMB "Remember"
#define KEYWRD_RECALL "Recall"

#define KEYWRD_TO     "to"
#define KEYWRD_AND    "and"
#define KEYWRD_SO     "so"
#define KEYWRD_NOT    "not"
#define KEYWRD_THAN   "than"
#define KEYWRD_AS     "as"

#define KEYWRD_MORE   "more"
#define KEYWRD_LESS   "less"

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
#define KEYWRD_AM     "Am"
#define KEYWRD_ARE    "Are"
#define KEYWRD_ART    "Art"
#define KEYWRD_IS     "Is"
#define KEYWRD_BE     "Be"

/* Nil */
#define KEYWRD_NOTHING "nothing"
#define KEYWRD_ZERO    "zero"

/* Pronouns */
/* 1st person */
#define KEYWRD_I      "I"
#define KEYWRD_ME     "me"
/* 2nd person */
#define KEYWRD_THEE   "Thee"
#define KEYWRD_YOU    "You"
#define KEYWRD_THOU   "Thou"

/* Possessives */
/* 1st person */
#define KEYWRD_MINE   "mine"  /* archaic usage: mine eyes! */
#define KEYWRD_MY     "my"
/* 2nd person */
#define KEYWRD_THINE  "thine"
#define KEYWRD_THY    "thy"
#define KEYWRD_YOUR   "your"
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
