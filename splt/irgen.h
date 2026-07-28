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

typedef enum irinst {
   IrinstUnknown,
   IrinstSet,     /* SET <var> <value> */
   IrinstEnter,   /* ENTER <character> */
   IrinstExit,
   IrinstExeunt,
   IrinstSpeak,
   IrinstPush,
   IrinstPop,
   IrinstSum,
   IrinstDiff,
   IrinstProd,
   IrinstQuot,
   IrinstRem,
   IrinstSqrt,
   IrinstSqur,
   IrinstCube,
   Irinst2x,
   IrinstFact,
   IrinstOutN,
   IrinstOutC,
   IrinstInN,
   IrinstInC,
   IrinstGoto,
   IrinstEqual,
   IrinstGt,
   IrinstLt,
   IrinstRememb,
   IrinstRecall,
   IrinstJumpT,
   IrinstJumpF,
   IrinstNegate
} irinst_t;

void irgenerate(void);

#endif
