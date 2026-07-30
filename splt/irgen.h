#ifndef IRGEN_H
#define IRGEN_H

#include <stddef.h>

/* IR Symbol Kind */
typedef enum irnodekind {
   IrnodekindUnknown,
   IrnodekindRoot,
   IrnodekindAct,
   IrnodekindScene,
   IrnodekindBlock,
   IrnodekindOpcode,
   IrnodekindVar,
   IrnodekindPerson,
   IrnodekindConst,
   IrnodekindData
} irnodekind_t;

typedef struct irnode {
   enum {
      IrnodeDatkindInt,
      IrnodeDatkindStr
   } datkind;
   union {
      int n;
      struct {
         char *run;
         int len;
      } s;
   } dat;
   irnodekind_t kind;
   int lnum, lpos;
   size_t offset;
} irnode_t;

typedef enum irvar {
   IrvarDpsz,
   IrvarTeller,
   IrvarHearer,
   IrvarConst,     /* temporary variable to calculate a number */
   IrvarOperandL,  /* (1) operator left operand (2) a sole operand */
   IrvarOperandR   /* operator right operand */
} irvar_t;

typedef enum iropcode {
   IropcodeUnknown,
   IropcodeSet,     /* SET <var> <value> */
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
   IropcodeGoto,    /* GOTO <num> <string> */
   IropcodeEq,      /* EQ op_l op_r */
   IropcodeGt,      /* GT op_l op_r */
   IropcodeLt,      /* LT op_l op_r */
   IropcodeRememb,  /* REMEMB */
   IropcodeRecall,  /* RECALL */
   IropcodeJumpT,   /* JUMPTRUE  .Ln */
   IropcodeJumpF,   /* JUMPFALSE .Ln */
   IropcodeNegate   /* NEGATE */
} iropcode_t;

void irgenerate(void);

#endif
