#include "assembler.h"
#include "assembler.internals.h"

extern void assemble(void) {
   /* OBJECT FILE STRUCTURE
         Section 0 - Header      : total 21 bytes
            format indicator     - 4 bytes; BE; contains "SPLO"
            debug flag           - 1 byte
            sect 0 pos           - 4 bytes
            sect 1 pos           - 4 bytes
            sect 2 pos           - 4 bytes
            sect 3 pos           - 4 bytes

         Section 1 - Code        : variable size
            binary data...       - variable size

         Section 2 - Debug Info  : (optional section) variable size
            entry count          - 4 bytes
            entry...             - each 8 + n bytes
               opcode pos        - n bytes; equals SPL_INT_SIZ
               src file line     - 4 bytes;
               src file pos      - 4 bytes;

         Section 3 - Source File : (optional section) variable size
            text data...         - variable size */

   assemble_ctx_t actx;
   char *destname;

   actx.le = isle(), actx.be = !actx.le;
   actx.nrtv = tree_child(irt, 1);

   destname = make_destname("hello.spl", OBJ_EXTENSION);
   fp = safe_fopen(destname, "w");

   fmtwrt(ENPREFIX "generating object file " Cbyellow "\"%s\"" Creset "...", destname);

   /* Fills opcode.offset field first */
   actx.offset = OBJFILE_HDSIZ;
   tree_pre_traverse(actx.nrtv, setoffset_route, 0, &actx.offset);

   /* Writes the code section */
   tree_pre_traverse(actx.nrtv, write_route, 0, NULL);

   /* Writes the debug info & source file section */
   actx.s2p = actx.offset;
   if (1) {
      write_debug_info(&actx);
      actx.s3p = actx.offset;
      write_srcfile(&actx);
   }

   rewind(fp);

   /* Finally, writes the header section */
   write_header(1, &actx);

   fmtwrt(
      " " Cgreen "done!" Creset
      " (total " Cbwhite "%" SPL_UINT_FMTSPC Creset " bytes) \n",
      actx.offset
   );

   /* Cleans up */
   safe_fclose(fp);
   free(destname);
}

static void setoffset_route(tree_t *t, int lv, void *ctx) {
   /* SIZE
         opcode  - 1 byte
         var     - 1 byte
         person  - SPL_INT_SIZ bytes
         const   - SPL_INT_SIZ bytes */

   irnode_t *n;
   spl_uint_t *offset;

   (void) lv;
   n = tree_dat(t);

   if (n->kind != IrnodekindOpcode && n->kind != IrnodekindEop)
      return;

   /* Writes the offset of the current node */
   offset = ctx;
   n->offset = *offset;

   if (n->kind == IrnodekindEop)
      return;

   /* Calculates the offset of the next node */
   *offset += SPL_OPCODE_SIZ;
   switch (n->dat.ui) {
      case IropcodeSet    : *offset += SPL_VAR_SIZ + SPL_CONST_SIZ; break;
      case IropcodeAsgn   : *offset += 2 * SPL_VAR_SIZ; break;
      case IropcodeEnter  : /* fall-through */
      case IropcodeExit   : /* fall-through */
      case IropcodeSpeak  : *offset += SPL_PERSON_SIZ; break;
      case IropcodePush   : /* fall-through */
      case IropcodePop    : *offset += SPL_VAR_SIZ; break;
      case IropcodeSum    : /* fall-through */
      case IropcodeDiff   : /* fall-through */
      case IropcodeProd   : /* fall-through */
      case IropcodeQuot   : /* fall-through */
      case IropcodeRem    : *offset += 3 * SPL_VAR_SIZ; break;
      case IropcodeSqrt   : /* fall-through */
      case IropcodeSqur   : /* fall-through */
      case IropcodeCube   : /* fall-through */
      case Iropcode2x     : /* fall-through */
      case IropcodeFact   : /* fall-through */
      case IropcodeEq     : /* fall-through */
      case IropcodeGt     : /* fall-through */
      case IropcodeLt     : *offset += 2 * SPL_VAR_SIZ; break;
      case IropcodeGoto   : /* fall-through */
      case IropcodeJumpT  : /* fall-through */
      case IropcodeJumpF  : *offset += SPL_ADDR_SIZ; break;
      default: ;  /* opcodes with no parameter fall here */
   }
}

static void write_route(tree_t *t, int lv, void *ctx) {
   irnode_t *n;

   (void) lv, (void) ctx;
   n = tree_dat(t);

   if (n->kind != IrnodekindOpcode)
      return;

   switch (n->dat.ui) {
      case IropcodeSet    : write_set(t); break;
      case IropcodeEnter  : /* fall-through */
      case IropcodeExit   : /* fall-through */
      case IropcodeSpeak  : write_enterlike(t); break;
      case IropcodeOutN   : /* fall-through */
      case IropcodeOutC   : /* fall-through */
      case IropcodeInN    : /* fall-through */
      case IropcodeInC    : /* fall-through */
      case IropcodeRememb : /* fall-through */
      case IropcodeRecall : /* fall-through */
      case IropcodeNegate : /* fall-through */
      case IropcodeExeunt : write_paramless_opcode(t); break;
      case IropcodePush   : /* fall-through */
      case IropcodePop    : write_pushlike(t); break;
      case IropcodeSum    : /* fall-through */
      case IropcodeDiff   : /* fall-through */
      case IropcodeProd   : /* fall-through */
      case IropcodeQuot   : /* fall-through */
      case IropcodeRem    : write_binary_op(t); break;
      case IropcodeSqrt   : /* fall-through */
      case IropcodeSqur   : /* fall-through */
      case IropcodeCube   : /* fall-through */
      case Iropcode2x     : /* fall-through */
      case IropcodeFact   : /* fall-through */
      case IropcodeEq     : /* fall-through */
      case IropcodeGt     : /* fall-through */
      case IropcodeLt     : /* fall-through */
      case IropcodeAsgn   : write_unary_op(t); break;
      case IropcodeGoto   : write_goto(t); break;
      case IropcodeJumpT  : /* fall-through */
      case IropcodeJumpF  : write_jumplike(t); break;
      default: ;
   }
}

static void write_set(tree_t *t) {
   irnode_t *n, *p1, *p2;

   n = tree_dat(t);
   p1 = tree_chdat(t, 0);
   p2 = tree_chdat(t, 1);

   sfwrite(&n->dat.i, SPL_OPCODE_SIZ, 1, fp);
   sfwrite(&p1->dat.i, SPL_VAR_SIZ, 1, fp);
   sfwrite(&p2->dat.i, SPL_CONST_SIZ, 1, fp);
}

static void write_enterlike(tree_t *t) {
   irnode_t *n, *p1;

   n = tree_dat(t);
   p1 = tree_chdat(t, 0);

   sfwrite(&n->dat.i, SPL_OPCODE_SIZ, 1, fp);
   sfwrite(&p1->dat.i, SPL_PERSON_SIZ, 1, fp);
}

static void write_paramless_opcode(tree_t *t) {
   irnode_t *n = tree_dat(t);
   sfwrite(&n->dat.i, SPL_OPCODE_SIZ, 1, fp);
}

static void write_pushlike(tree_t *t) {
   irnode_t *n, *p1;

   n = tree_dat(t);
   p1 = tree_chdat(t, 0);

   sfwrite(&n->dat.i, SPL_OPCODE_SIZ, 1, fp);
   sfwrite(&p1->dat.i, SPL_VAR_SIZ, 1, fp);
}

static void write_binary_op(tree_t *t) {
   irnode_t *n, *p1, *p2, *p3;

   n = tree_dat(t);
   p1 = tree_chdat(t, 0);
   p2 = tree_chdat(t, 1);
   p3 = tree_chdat(t, 2);

   sfwrite(&n->dat.i, SPL_OPCODE_SIZ, 1, fp);
   sfwrite(&p1->dat.i, SPL_VAR_SIZ, 1, fp);
   sfwrite(&p2->dat.i, SPL_VAR_SIZ, 1, fp);
   sfwrite(&p3->dat.i, SPL_VAR_SIZ, 1, fp);
}

static void write_unary_op(tree_t *t) {
   irnode_t *n, *p1, *p2;

   n = tree_dat(t);
   p1 = tree_chdat(t, 0);
   p2 = tree_chdat(t, 1);

   sfwrite(&n->dat.i, SPL_OPCODE_SIZ, 1, fp);
   sfwrite(&p1->dat.i, SPL_VAR_SIZ, 1, fp);
   sfwrite(&p2->dat.i, SPL_VAR_SIZ, 1, fp);
}

static void write_goto(tree_t *t) {
   tree_t *root, *act, *scene, *block, *op;
   irnode_t *n, *p1, *p2;
   irnode_t *act_dat, *op_dat;
   int a, s;

   n = tree_dat(t);
   p1 = tree_chdat(t, 0);
   p2 = tree_chdat(t, 1);

   block = tree_parent(t);
   scene = tree_parent(block);
   act = tree_parent(scene);
   root = tree_parent(act);

   if (p1->dat.i == NODEKIND_ACT) {
      a = interpret_romnum(p2->dat.s.run) - 1;
      op = find_nearest_opcode(root, a, 0, 0);
   }
   else {  /* NODEKIND_SCENE */
      act_dat = tree_dat(act);
      a = interpret_romnum(act_dat->dat.s.run) - 1;
      s = interpret_romnum(p2->dat.s.run) - 1;
      op = find_nearest_opcode(root, a, s, 0);
   }
   op_dat = tree_dat(op);

   sfwrite(&n->dat.i, SPL_OPCODE_SIZ, 1, fp);
   sfwrite(&op_dat->offset, SPL_ADDR_SIZ, 1, fp);
}

static void write_jumplike(tree_t *t) {
   tree_t *root, *act, *scene, *block, *op;
   irnode_t *n, *p1;
   irnode_t *act_dat, *scene_dat, *block_dat, *op_dat;
   int scene_siz, a, s, b;

   n = tree_dat(t);
   p1 = tree_chdat(t, 0);

   /* Approaches the current block & scene in sequence
      in which this node is */
   block = tree_parent(t);
   scene = tree_parent(block);

   /* Searches the matching block in reverse order */
   scene_siz = tree_clen(scene);
   for (b = scene_siz - 1; b >= 0; b--) {
      block = tree_child(scene, b);
      block_dat = tree_dat(block);
      if (block_dat->dat.i == p1->dat.i)
         break;  /* must exist */
   }

   act = tree_parent(scene);
   root = tree_parent(act);

   act_dat = tree_dat(act);
   scene_dat = tree_dat(scene);

   a = interpret_romnum(act_dat->dat.s.run) - 1;
   s = interpret_romnum(scene_dat->dat.s.run) - 1;

   /* Acquires the first opcode of the requested
      block in the scene */
   op = find_nearest_opcode(root, a, s, b);
   op_dat = tree_dat(op);

   sfwrite(&n->dat.i, SPL_OPCODE_SIZ, 1, fp);
   sfwrite(&op_dat->offset, SPL_ADDR_SIZ, 1, fp);
}

static tree_t *find_nearest_opcode(tree_t *root, int a, int s, int b) {
   tree_t *act, *scene, *block, *op;
   int nrtv_siz, act_siz, scene_siz;

   nrtv_siz = tree_clen(root);
   for ( ; a < nrtv_siz; a++) {
      act = tree_child(root, a);
      act_siz = tree_clen(act);
      for ( ; s < act_siz; s++) {
         scene = tree_child(act, s);
         scene_siz = tree_clen(scene);
         for ( ; b < scene_siz; b++) {
            block = tree_child(scene, b);
            if (!tree_clen(block))
               continue;
            op = tree_child(block, 0);
            return op;
         }
         b = 0;
      }
      s = 0;
   }

   /* control never reaches here */
   return NULL;
}

static void write_debug_info(assemble_ctx_t *actx) {
   fpos_t ecnt_pos, eos_pos;
   spl_uint_t before, diff;
   uint32_t ecnt;

   sfgetpos(fp, &ecnt_pos);
   actx->offset += OBJFILE_DI_EC;
   before = actx->offset;
   tree_pre_traverse(actx->nrtv, write_dbginfo_route, 0, actx);
   sfgetpos(fp, &eos_pos);
   diff = actx->offset - before;
   ecnt = diff / OBJFILE_DI_ETSIZ;
   sfsetpos(fp, &ecnt_pos);
   if (actx->be) ecnt = endrev32(ecnt);
   sfwrite(&ecnt, OBJFILE_DI_EC, 1, fp);
   sfsetpos(fp, &eos_pos);
}

static void write_dbginfo_route(tree_t *t, int lv, void *ctx) {
   irnode_t *n;
   assemble_ctx_t *actx;

   (void) lv;
   actx = ctx;
   n = tree_dat(t);

   if (n->kind != IrnodekindOpcode)
      return;

   // fixme: 프로젝트 전체적으로 lnum lpos 사이즈 spl_...로 통일
   // fixme: 프로젝트에서 size_t를 쓰는게 적절한곳엔 size_t 사용
   sfwrite(&actx->offset, OBJFILE_DI_ET_OP, 1, fp);
   sfwrite(&n->lnum, OBJFILE_DI_ET_SL, 1, fp);
   sfwrite(&n->lpos, OBJFILE_DI_ET_SP, 1, fp);

   actx->offset += OBJFILE_DI_ETSIZ;
}

static void write_srcfile(assemble_ctx_t *actx) {
   // fixme: 프로젝트 전역에서 쓰는 컨텍스트 구조체를 마련해서 거기서 값을 가져다쓰는게 나을듯
   size_t lls, cnt;
   line_t *l;

   lls = arr_size(ls);
   cnt = 0;
   for (size_t i = 0; i < lls; i++) {
      l = arr_peek(ls, i);
      cnt += l->len;  // fixme: 현재 len은 \0을 포함한 길이이므로 제외해야 할듯
      sfputs(fp, l->run);
   }

   actx->offset += cnt;
}

static void write_header(bool debug_flag, assemble_ctx_t *actx) {
   uint32_t fi, s0p, s1p;

   fi = OBJFILE_FI;
   if (actx->le) fi = endrev32(fi);
   s0p = 0;
   s1p = OBJFILE_HDSIZ;

   sfwrite(&fi, sizeof fi, 1, fp);
   sfwrite(&debug_flag, 1, 1, fp);
   sfwrite(&s0p, sizeof s0p, 1, fp);
   sfwrite(&s1p, sizeof s1p, 1, fp);
   sfwrite(&actx->s2p, sizeof actx->s2p, 1, fp);
   sfwrite(&actx->s3p, sizeof actx->s3p, 1, fp);
}
