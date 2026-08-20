#ifndef ASSEMBLER_INTERNALS_H
#define ASSEMBLER_INTERNALS_H

#include "irgen.h"
#include "global.h"
#include "tree.adt.h"
#include "wrappers.h"
#include "strutils.h"
#include "typecheck.h"

#define SPL_OPCODE_SIZ 1
#define SPL_VAR_SIZ    1
#define SPL_PERSON_SIZ SPL_INT_SIZ
#define SPL_CONST_SIZ  SPL_INT_SIZ
#define SPL_ADDR_SIZ   4

typedef struct {
   bool le, be;
   spl_uint_t offset;
   uint32_t s2p, s3p;
   tree_t *nrtv;
} asm_ctx_t;

typedef void writer_t(tree_t *t);

static tree_callback_t setoffset_route;
static tree_callback_t write_route;
static tree_callback_t write_dbginfo_route;

static writer_t write_set;
static writer_t write_enterlike;
static writer_t write_paramless_opcode;
static writer_t write_pushlike;
static writer_t write_binary_op;
static writer_t write_unary_op;
static writer_t write_goto;
static writer_t write_jumplike;

static tree_t *find_nearest_opcode(tree_t *root, size_t a, size_t s, size_t b);

static void write_debug_info(asm_ctx_t *actx);
static void write_srcfile(asm_ctx_t *actx);
static void write_header(bool debug_flag, asm_ctx_t *actx);

static FILE *fp;

#endif
