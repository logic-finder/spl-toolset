#ifndef IRGEN_H
#define IRGEN_H

#include <stddef.h>

#include "splc.h"
#include "common.h"
#include "global.h"

/* IR Symbol Kind */
typedef enum irnodekind {
   Irnodekind_Unknown,
   IrnodekindEop,
   IrnodekindRoot,
   IrnodekindDp,
   IrnodekindNrtv,
   IrnodekindAct,
   IrnodekindScene,
   IrnodekindBlock,
   IrnodekindOpcode,
   IrnodekindVar,
   IrnodekindPerson,
   IrnodekindConst,
   IrnodekindData
} irnodekind_t;

typedef enum {
   IrnodeDatkindInt,
   IrnodeDatkindUint,
   IrnodeDatkindStr
} irnode_datkind_t;

typedef struct irnode {
   irnode_datkind_t datkind;
   union {
      int i;
      unsigned int ui;
      struct {
         char *run;
         int len;
      } s;
   } dat;
   irnodekind_t kind;
   size_t lnum, lpos;
   spl_uint_t offset;
} irnode_t;

typedef enum irvar {
   IrvarDpsz,
   IrvarTeller,
   IrvarHearer,
   IrvarConst,     /* temporary variable to calculate a number */
   IrvarOperandL,  /* (1) operator left operand (2) a sole operand */
   IrvarOperandR,  /* operator right operand */
   Irvar_Dp_Begin  /* dp[0] = this value */
} irvar_t;

typedef enum iropcode {
   Iropcode_Unknown,
   IropcodeSet,     /* SET <var> <value> */
   IropcodeAsgn,    /* ASGN <var> <var> */
   IropcodeEnter,   /* ENTER <char> */
   IropcodeExit,    /* EXIT  <char> */
   IropcodeExeunt,  /* EXEUNT */
   IropcodeSpeak,   /* SPEAK <char> */
   IropcodePush,    /* PUSH <var> */
   IropcodePop,     /* POP  <var> */
   IropcodeSum,     /* SUM  dest op_l op_r */
   IropcodeDiff,    /* DIFF dest op_l op_r */
   IropcodeProd,    /* PROD dest op_l op_r */
   IropcodeQuot,    /* QUOT dest op_l op_r */
   IropcodeRem,     /* REM  dest op_l op_r */
   IropcodeSqrt,    /* SQRT dest op */
   IropcodeSqur,    /* SQUR dest op */
   IropcodeCube,    /* CUBE dest op */
   Iropcode2x,      /* 2x   dest op */
   IropcodeFact,    /* FACT dest op */
   IropcodeOutN,    /* OUTN */
   IropcodeOutC,    /* OUTC */
   IropcodeInN,     /* INN */
   IropcodeInC,     /* INC */
   IropcodeGoto,    /* GOTO <addr> */
   IropcodeEq,      /* EQ op_l op_r */
   IropcodeGt,      /* GT op_l op_r */
   IropcodeLt,      /* LT op_l op_r */
   IropcodeRememb,  /* REMEMB const */
   IropcodeRecall,  /* RECALL */
   IropcodeJumpT,   /* JUMPTRUE  <addr> */
   IropcodeJumpF,   /* JUMPFALSE <addr> */
   IropcodeNegate   /* NEGATE */
} iropcode_t;

void irgenerate(compile_ctx_t *cctx);
void destroy_irt(compile_ctx_t *cctx);
size_t count_opcodes(tree_t *irt);

#endif
