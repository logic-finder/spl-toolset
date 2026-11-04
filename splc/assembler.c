#include "assembler.h"
#include "assembler.internals.h"

extern bool assemble(compile_ctx_t *cctx) {
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
            text data...         - variable size
    */

   bool assemble_stat = false;
   bool fclosed = false;
   asm_ctx_t actx;
   char *destname;
   int nchar;

   /* Inits actx */
   actx.le = isle(), actx.be = !actx.le;
   actx.nrtv = tree_child(cctx->irt, 1);
   actx.ls = cctx->ls;

   /* Creates a destination file */
   if (!(destname = strdup(cctx->cfg->out))) {
      return false;
   }
   if (!(actx.fp = fopen(destname, "w"))) {
      goto cleanup;
   }

   /* Notifies the start of the work */
   if (cctx->cfg->vbs) {
      nchar = printf(
         ENPREFIX "generating object file " Cbyellow "\"%s\"" Creset "...",
         destname
      );
      if (nchar < 0) goto cleanup;
   }

   /* Fills opcode.offset field first */
   actx.offset = OBJFILE_HDSIZ;
   if (!tree_pre_traverse(actx.nrtv, setoffset_route, 0, &actx.offset)) {
      goto cleanup;
   }

   /* Writes the code section */
   if (!tree_pre_traverse(actx.nrtv, write_route, 0, &actx)) {
      goto cleanup;
   }

   /* Writes the debug info & source file section */
   actx.s2p = actx.offset;
   actx.s3p = 0;

   if (cctx->cfg->dbg) {
      if (!write_debug_info(&actx)) {
         goto cleanup;
      }
      actx.s3p = actx.offset;
      if (!write_srcfile(&actx)) {
         goto cleanup;
      }
   }

   /* Finally, writes the header section */
   rewind(actx.fp);

   if (!write_header(1, &actx)) {
      goto cleanup;
   }

   /* Notifies the end of the work */
   if (cctx->cfg->vbs) {
      nchar = printf(
         " " Cgreen "done!" Creset
         " (total " Cbwhite "%" SPL_UINT_FMTSPC Creset " bytes) \n",
         actx.offset
      );
      if (nchar < 0) goto cleanup;
   }

   /* Cleans up */
   if (fclose(actx.fp) == EOF) {
      fclosed = true;
      goto cleanup;
   }

   if (cctx->cfg->drn) {
      if (remove(destname)) {
         goto cleanup;
      }
   }

   /* Everything was successful */
   assemble_stat = true;

cleanup:
   if (!fclosed) (void) fclose(actx.fp);
   free(destname);

   return assemble_stat;
}

static bool setoffset_route(tree_t *t, size_t lv, void *ctx) {
   /* SIZE
         opcode  - 1 byte
         var     - 1 byte
         person  - SPL_INT_SIZ bytes
         const   - SPL_INT_SIZ bytes
    */

   irnode_t *n;
   spl_uint_t *offset;

   (void) lv;
   n = tree_dat(t);

   if (n->kind != IrnodekindOpcode && n->kind != IrnodekindEop) {
      return true;
   }

   /* Writes the offset of the current node */
   offset = ctx;
   n->offset = *offset;

   if (n->kind == IrnodekindEop) {
      return true;
   }

   /* Calculates the offset of the next node */
   *offset += SPL_OPCODE_SIZ;

   switch (n->dat.ui) {
      case IropcodeSet    : *offset += SPL_VAR_SIZ + SPL_CONST_SIZ;
                            break;

      case IropcodeAsgn   : *offset += 2 * SPL_VAR_SIZ;
                            break;

      case IropcodeEnter  : [[fallthrough]];
      case IropcodeExit   : [[fallthrough]];
      case IropcodeSpeak  : *offset += SPL_PERSON_SIZ;
                            break;

      case IropcodePush   : [[fallthrough]];
      case IropcodePop    : *offset += SPL_VAR_SIZ;
                            break;

      case IropcodeSum    : [[fallthrough]];
      case IropcodeDiff   : [[fallthrough]];
      case IropcodeProd   : [[fallthrough]];
      case IropcodeQuot   : [[fallthrough]];
      case IropcodeRem    : *offset += 3 * SPL_VAR_SIZ;
                            break;

      case IropcodeSqrt   : [[fallthrough]];
      case IropcodeSqur   : [[fallthrough]];
      case IropcodeCube   : [[fallthrough]];
      case Iropcode2x     : [[fallthrough]];
      case IropcodeFact   : [[fallthrough]];
      case IropcodeEq     : [[fallthrough]];
      case IropcodeGt     : [[fallthrough]];
      case IropcodeLt     : *offset += 2 * SPL_VAR_SIZ;
                            break;

      case IropcodeGoto   : [[fallthrough]];
      case IropcodeJumpT  : [[fallthrough]];
      case IropcodeJumpF  : *offset += SPL_ADDR_SIZ;
                            break;

      default: ;  /* opcodes with no parameter fall here */
   }

   return true;
}

static bool write_route(tree_t *t, size_t lv, void *ctx) {
   asm_ctx_t *actx;

   (void) lv;
   actx = ctx;
   actx->t = t;
   actx->n = tree_dat(actx->t);

   if (actx->n->kind != IrnodekindOpcode) {
      return true;
   }

   switch (actx->n->dat.ui) {
      case IropcodeSet    : return write_set(actx);

      case IropcodeEnter  : [[fallthrough]];
      case IropcodeExit   : [[fallthrough]];
      case IropcodeSpeak  : return write_enterlike(actx);

      case IropcodeOutN   : [[fallthrough]];
      case IropcodeOutC   : [[fallthrough]];
      case IropcodeInN    : [[fallthrough]];
      case IropcodeInC    : [[fallthrough]];
      case IropcodeRecall : [[fallthrough]];
      case IropcodeNegate : [[fallthrough]];
      case IropcodeExeunt : return write_paramless_opcode(actx);

      case IropcodeRememb : [[fallthrough]];
      case IropcodePush   : [[fallthrough]];
      case IropcodePop    : return write_pushlike(actx);

      case IropcodeSum    : [[fallthrough]];
      case IropcodeDiff   : [[fallthrough]];
      case IropcodeProd   : [[fallthrough]];
      case IropcodeQuot   : [[fallthrough]];
      case IropcodeRem    : return write_binary_op(actx);

      case IropcodeSqrt   : [[fallthrough]];
      case IropcodeSqur   : [[fallthrough]];
      case IropcodeCube   : [[fallthrough]];
      case Iropcode2x     : [[fallthrough]];
      case IropcodeFact   : [[fallthrough]];
      case IropcodeEq     : [[fallthrough]];
      case IropcodeGt     : [[fallthrough]];
      case IropcodeLt     : [[fallthrough]];
      case IropcodeAsgn   : return write_unary_op(actx);

      case IropcodeGoto   : return write_goto(actx);

      case IropcodeJumpT  : [[fallthrough]];
      case IropcodeJumpF  : return write_jumplike(actx);

      default: break;  /* control never reaches default */
   }

   /* control never reaches here */
   return true;
}

static bool write_set(asm_ctx_t *actx) {
   irnode_t *p1, *p2;

   p1 = tree_chdat(actx->t, 0);
   p2 = tree_chdat(actx->t, 1);

   if (1 > fwrite(&actx->n->dat.i, SPL_OPCODE_SIZ, 1, actx->fp)) {
      return false;
   }
   if (1 > fwrite(&p1->dat.i, SPL_VAR_SIZ, 1, actx->fp)) {
      return false;
   }
   if (1 > fwrite(&p2->dat.i, SPL_CONST_SIZ, 1, actx->fp)) {
      return false;
   }

   return true;
}

static bool write_enterlike(asm_ctx_t *actx) {
   irnode_t *p1;

   p1 = tree_chdat(actx->t, 0);

   if (1 > fwrite(&actx->n->dat.i, SPL_OPCODE_SIZ, 1, actx->fp)) {
      return false;
   }
   if (1 > fwrite(&p1->dat.i, SPL_PERSON_SIZ, 1, actx->fp)) {
      return false;
   }

   return true;
}

static bool write_paramless_opcode(asm_ctx_t *actx) {
   size_t nmemb = fwrite(&actx->n->dat.i, SPL_OPCODE_SIZ, 1, actx->fp);
   return (1 == nmemb) ? true : false;
}

static bool write_pushlike(asm_ctx_t *actx) {
   irnode_t *p1;

   p1 = tree_chdat(actx->t, 0);

   if (1 > fwrite(&actx->n->dat.i, SPL_OPCODE_SIZ, 1, actx->fp)) {
      return false;
   }
   if (1 > fwrite(&p1->dat.i, SPL_VAR_SIZ, 1, actx->fp)) {
      return false;
   }

   return true;
}

static bool write_binary_op(asm_ctx_t *actx) {
   irnode_t *p1, *p2, *p3;

   p1 = tree_chdat(actx->t, 0);
   p2 = tree_chdat(actx->t, 1);
   p3 = tree_chdat(actx->t, 2);

   if (1 > fwrite(&actx->n->dat.i, SPL_OPCODE_SIZ, 1, actx->fp)) {
      return false;
   }
   if (1 > fwrite(&p1->dat.i, SPL_VAR_SIZ, 1, actx->fp)) {
      return false;
   }
   if (1 > fwrite(&p2->dat.i, SPL_VAR_SIZ, 1, actx->fp)) {
      return false;
   }
   if (1 > fwrite(&p3->dat.i, SPL_VAR_SIZ, 1, actx->fp)) {
      return false;
   }

   return true;
}

static bool write_unary_op(asm_ctx_t *actx) {
   irnode_t *p1, *p2;

   p1 = tree_chdat(actx->t, 0);
   p2 = tree_chdat(actx->t, 1);

   if (1 < fwrite(&actx->n->dat.i, SPL_OPCODE_SIZ, 1, actx->fp)) {
      return false;
   }
   if (1 < fwrite(&p1->dat.i, SPL_VAR_SIZ, 1, actx->fp)) {
      return false;
   }
   if (1 < fwrite(&p2->dat.i, SPL_VAR_SIZ, 1, actx->fp)) {
      return false;
   }

   return true;
}

static bool write_goto(asm_ctx_t *actx) {
   tree_t *root, *act, *scene, *block, *op;
   irnode_t *p1, *p2;
   irnode_t *act_dat, *op_dat;
   size_t a, s;

   p1 = tree_chdat(actx->t, 0);
   p2 = tree_chdat(actx->t, 1);

   block = tree_parent(actx->t);
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

   if (1 < fwrite(&actx->n->dat.i, SPL_OPCODE_SIZ, 1, actx->fp)) {
      return false;
   }
   if (1 < fwrite(&op_dat->offset, SPL_ADDR_SIZ, 1, actx->fp)) {
      return false;
   }

   return true;
}

static bool write_jumplike(asm_ctx_t *actx) {
   tree_t *root, *act, *scene, *block, *op;
   irnode_t *p1;
   irnode_t *act_dat, *scene_dat, *block_dat, *op_dat;
   size_t scene_siz, a, s, b;

   p1 = tree_chdat(actx->t, 0);

   /* Approaches the current block & scene in sequence
      in which this node is */
   block = tree_parent(actx->t);
   scene = tree_parent(block);

   /* Searches the matching block in reverse order */
   scene_siz = tree_clen(scene);
   for (b = scene_siz; b > 1; b--) {
      block = tree_child(scene, b - 1);
      block_dat = tree_dat(block);
      if (block_dat->dat.i == p1->dat.i) {
         break;  /* must exist */
      }
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

   if (1 < fwrite(&actx->n->dat.i, SPL_OPCODE_SIZ, 1, actx->fp)) {
      return false;
   }
   if (1 < fwrite(&op_dat->offset, SPL_ADDR_SIZ, 1, actx->fp)) {
      return false;
   }

   return true;
}

static tree_t *find_nearest_opcode(tree_t *root, size_t a, size_t s, size_t b) {
   tree_t *act, *scene, *block, *op;
   size_t nrtv_siz, act_siz, scene_siz;

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

static bool write_debug_info(asm_ctx_t *actx) {
   fpos_t ecnt_pos, eos_pos;
   spl_uint_t before, diff;
   uint32_t ecnt;

   if (fgetpos(actx->fp, &ecnt_pos)) {
      return false;
   }

   actx->offset += OBJFILE_DI_EC;
   before = actx->offset;

   if (!tree_pre_traverse(actx->nrtv, write_dbginfo_route, 0, actx)) {
      return false;
   }
   if (fgetpos(actx->fp, &eos_pos)) {
      return false;
   }

   diff = actx->offset - before;
   ecnt = diff / OBJFILE_DI_ETSIZ;

   if (fsetpos(actx->fp, &ecnt_pos)) {
      return false;
   }
   if (actx->be) {
      ecnt = endrev32(ecnt);
   }
   if (1 > fwrite(&ecnt, OBJFILE_DI_EC, 1, actx->fp)) {
      return false;
   }
   if (fsetpos(actx->fp, &eos_pos)) {
      return false;
   }

   return true;
}

static bool write_dbginfo_route(tree_t *t, size_t lv, void *ctx) {
   irnode_t *n;
   asm_ctx_t *actx;

   (void) lv;
   actx = ctx;
   n = tree_dat(t);

   if (n->kind != IrnodekindOpcode) {
      return true;
   }

   // fixme: 프로젝트 전체적으로 lnum lpos 사이즈 spl_...로 통일
   // fixme: 프로젝트에서 size_t를 쓰는게 적절한곳엔 size_t 사용
   if (1 > fwrite(&actx->offset, OBJFILE_DI_ET_OP, 1, actx->fp)) {
      return false;
   }
   if (1 > fwrite(&n->lnum, OBJFILE_DI_ET_SL, 1, actx->fp)) {
      return false;
   }
   if (1 > fwrite(&n->lpos, OBJFILE_DI_ET_SP, 1, actx->fp)) {
      return false;
   }

   actx->offset += OBJFILE_DI_ETSIZ;

   return true;
}

static bool write_srcfile(asm_ctx_t *actx) {
   // fixme: 프로젝트 전역에서 쓰는 컨텍스트 구조체를 마련해서 거기서 값을 가져다쓰는게 나을듯
   size_t lls, cnt;
   line_t *l;

   lls = array_size(actx->ls);
   cnt = 0;

   for (size_t i = 0; i < lls; i++) {
      l = array_peek(actx->ls, i);
      if (1 > fwrite(l->run, 1, l->len, actx->fp)) {
         return false;
      }
   }

   for (size_t i = 0; i < lls; i++) {
      l = array_peek(actx->ls, i);
      cnt += l->len;
   }
   actx->offset += cnt;

   return true;
}

static bool write_header(bool debug_flag, asm_ctx_t *actx) {
   uint32_t fi, s0p, s1p;

   fi = OBJFILE_FI;
   if (actx->le) fi = endrev32(fi);
   s0p = 0;
   s1p = OBJFILE_HDSIZ;

   if (1 > fwrite(&fi, sizeof fi, 1, actx->fp)) {
      return false;
   }
   if (1 > fwrite(&debug_flag, 1, 1, actx->fp)) {
      return false;
   }
   if (1 > fwrite(&s0p, sizeof s0p, 1, actx->fp)) {
      return false;
   }
   if (1 > fwrite(&s1p, sizeof s1p, 1, actx->fp)) {
      return false;
   }
   if (1 > fwrite(&actx->s2p, sizeof actx->s2p, 1, actx->fp)) {
      return false;
   }
   if (1 > fwrite(&actx->s3p, sizeof actx->s3p, 1, actx->fp)) {
      return false;
   }

   return true;
}
