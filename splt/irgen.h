#ifndef IRGEN_H
#define IRGEN_H

/* IR Symbol Kind */
typedef enum irnodekind {
   IrnodekindUnknown,
   IrnodekindRoot,
   IrnodekindAct,
   IrnodekindScene,
   IrnodekindBlock,
   IrnodekindInst,
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
   IropcodeEnter,   /* ENTER <character> */
   IropcodeExit,
   IropcodeExeunt,
   IropcodeSpeak,
   IropcodePush,
   IropcodePop,
   IropcodeSum,
   IropcodeDiff,
   IropcodeProd,
   IropcodeQuot,
   IropcodeRem,
   IropcodeSqrt,
   IropcodeSqur,
   IropcodeCube,
   Iropcode2x,
   IropcodeFact,
   IropcodeOutN,
   IropcodeOutC,
   IropcodeInN,
   IropcodeInC,
   IropcodeGoto,
   IropcodeEqual,
   IropcodeGt,
   IropcodeLt,
   IropcodeRememb,
   IropcodeRecall,
   IropcodeJumpT,
   IropcodeJumpF,
   IropcodeNegate
} iropcode_t;

void irgenerate(void);

#endif
