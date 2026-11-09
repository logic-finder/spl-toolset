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

   bool fclosed = false;
   asm_ctx_t *actx = NULL;

   // Init actx
   if (!(actx = malloc(sizeof *actx))) {
      cctx->perr = "failed to create actx";
      PTRACE(cctx);
      return false;
   }

   actx->destname = cctx->cfg->out;
   actx->cfg = cctx->cfg;
   actx->le = isle();
   actx->be = !actx->le;
   actx->nrtv = tree_child(cctx->irt, 1);
   actx->ls = cctx->ls;
   actx->lc = cctx->lc;
   actx->ptrace = cctx->ptrace;
   actx->perr = NULL;
   actx->phdlr = NULL;
   actx->pnctx = NULL;

   // Create a destination file
   if (!(actx->fp = fopen(actx->destname, "w"))) {
      actx->perr = "failed to fopen dest";
      goto err;
   }

   // Notify the start of the work
   if (cctx->cfg->vbs) {
      (void) printf(
         ENPREFIX "generating object file " Cbyellow "\"%s\"" Creset "...",
         actx->destname
      );
   }

   // Write .splo into the destination
   if (!write2dest(actx)) {
      goto err;
   }

   // Notify the end of the work
   if (cctx->cfg->vbs) {
      (void) printf(
         " " Cgreen "done!" Creset
         " (total " Cbwhite "%" SPL_UINT_FMTSPC Creset " bytes) \n",
         actx->offset
      );
   }

   // Clean up
   if (fclose(actx->fp) == EOF) {
      fclosed = true;
      actx->perr = "failed to fclose";
      goto err;
   }

   if (cctx->cfg->drn) {
      if (remove(actx->destname)) {
         actx->perr = "failed to remove dest";
         goto err;
      }
   }

   // Normal execution path
   return true;

err:
   PTRACE(actx);
   PH_COPY(cctx, actx);
   FCLOSE_ERR_PATH(fclosed, actx->fp);
   return false;
}

static bool write2dest(asm_ctx_t *actx) {
   // This string is used in all panic situations
   // which occurred while writing to dest unless
   // overwritten to dedicated error message
   actx->perr = "error occurred while writing to dest";

   // Fill opcode.offset
   actx->offset = OBJFILE_HDSIZ;

   if (!tree_pre_traverse(actx->nrtv, setoffset_route, 0, &actx->offset)) {
      goto err;
   }

   // Write the code section
   if (!tree_pre_traverse(actx->nrtv, write_route, 0, actx)) {
      goto err;
   }

   // Write the debug info & source file section
   actx->s2p = actx->offset;
   actx->s3p = 0;

   if (actx->cfg->dbg) {
      if (!write_debug_info(actx)) {
         goto err;
      }
      actx->s3p = actx->offset;
      if (!write_srcfile(actx)) {
         goto err;
      }
   }

   // Write the header section
   rewind(actx->fp);

   if (!write_header(actx)) {
      goto err;
   }

   return true;

err:
   PTRACE(actx);
   return false;
}

static bool setoffset_route(tree_t *t, size_t lv, void *ctx) {
   /* SIZE
         opcode  - 1 byte
         var     - 1 byte
         person  - SPL_INT_SIZ bytes
         const   - SPL_INT_SIZ bytes
    */

   irnode_t *n = tree_dat(t);
   spl_uint_t *offset = ctx;

   (void) lv;

   // Early return if this node is neither opcode nor end-of-program
   if ((n->kind != IrnodekindOpcode) && (n->kind != IrnodekindEop)) {
      return true;
   }

   // Write the offset of the current node
   n->offset = *offset;

   // End-of-program?
   if (n->kind == IrnodekindEop) {
      return true;
   }

   // Calculate the offset of the next node
   *offset += SPL_OPCODE_SIZ;

   switch (n->dat.ui) {
      case IropcodeSet: {
         *offset += SPL_VAR_SIZ + SPL_CONST_SIZ;
         break;
      }

      case IropcodeAsgn: {
         *offset += 2 * SPL_VAR_SIZ;
         break;
      }

      case IropcodeEnter : [[fallthrough]];
      case IropcodeExit  : [[fallthrough]];
      case IropcodeSpeak : {
         *offset += SPL_PERSON_SIZ;
         break;
      }

      case IropcodePush : [[fallthrough]];
      case IropcodePop  : {
         *offset += SPL_VAR_SIZ;
         break;
      }

      case IropcodeSum  : [[fallthrough]];
      case IropcodeDiff : [[fallthrough]];
      case IropcodeProd : [[fallthrough]];
      case IropcodeQuot : [[fallthrough]];
      case IropcodeRem  : {
         *offset += 3 * SPL_VAR_SIZ;
         break;
      }

      case IropcodeSqrt : [[fallthrough]];
      case IropcodeSqur : [[fallthrough]];
      case IropcodeCube : [[fallthrough]];
      case Iropcode2x   : [[fallthrough]];
      case IropcodeFact : [[fallthrough]];
      case IropcodeEq   : [[fallthrough]];
      case IropcodeGt   : [[fallthrough]];
      case IropcodeLt   : {
         *offset += 2 * SPL_VAR_SIZ;
         break;
      }

      case IropcodeGoto  : [[fallthrough]];
      case IropcodeJumpT : [[fallthrough]];
      case IropcodeJumpF : {
         *offset += SPL_ADDR_SIZ;
         break;
      }

      default: {
         break;  // Opcodes with no parameter fall here
      }
   }

   return true;
}

static bool write_route(tree_t *t, size_t lv, void *ctx) {
   asm_ctx_t *actx = ctx;

   bool hdlr_ret;

   (void) lv;
   actx->t = t;
   actx->n = tree_dat(actx->t);

   // Early return if this node is not an opcode
   if (actx->n->kind != IrnodekindOpcode) {
      return true;
   }

   switch (actx->n->dat.ui) {
      case IropcodeSet: {
         hdlr_ret = write_set(actx);
         break;
      }

      case IropcodeEnter : [[fallthrough]];
      case IropcodeExit  : [[fallthrough]];
      case IropcodeSpeak : {
         hdlr_ret = write_enterlike(actx);
         break;
      }

      case IropcodeOutN   : [[fallthrough]];
      case IropcodeOutC   : [[fallthrough]];
      case IropcodeInN    : [[fallthrough]];
      case IropcodeInC    : [[fallthrough]];
      case IropcodeRecall : [[fallthrough]];
      case IropcodeNegate : [[fallthrough]];
      case IropcodeExeunt : {
         hdlr_ret = write_paramless_opcode(actx);
         break;
      }

      case IropcodeRememb : [[fallthrough]];
      case IropcodePush   : [[fallthrough]];
      case IropcodePop    : {
         hdlr_ret = write_pushlike(actx);
         break;
      }

      case IropcodeSum  : [[fallthrough]];
      case IropcodeDiff : [[fallthrough]];
      case IropcodeProd : [[fallthrough]];
      case IropcodeQuot : [[fallthrough]];
      case IropcodeRem  : {
         hdlr_ret = write_binary_op(actx);
         break;
      }

      case IropcodeSqrt : [[fallthrough]];
      case IropcodeSqur : [[fallthrough]];
      case IropcodeCube : [[fallthrough]];
      case Iropcode2x   : [[fallthrough]];
      case IropcodeFact : [[fallthrough]];
      case IropcodeEq   : [[fallthrough]];
      case IropcodeGt   : [[fallthrough]];
      case IropcodeLt   : [[fallthrough]];
      case IropcodeAsgn : {
         hdlr_ret = write_unary_op(actx);
         break;
      }

      case IropcodeGoto : {
         hdlr_ret = write_goto(actx);
         break;
      }

      case IropcodeJumpT : [[fallthrough]];
      case IropcodeJumpF : {
         hdlr_ret = write_jumplike(actx);
         break;
      }

      default: {
         actx->perr = "unknown opcode";
         hdlr_ret = false;
         break;
      }
   }
   if (!hdlr_ret) {
      goto err;
   }

   return true;

err:
   PTRACE(actx);
   return false;
}

static bool write_set(asm_ctx_t *actx) {
   irnode_t *p1 = tree_chdat(actx->t, 0);
   irnode_t *p2 = tree_chdat(actx->t, 1);

   if (1 > fwrite(&actx->n->dat.i, SPL_OPCODE_SIZ, 1, actx->fp)) {
      goto err;
   }
   if (1 > fwrite(&p1->dat.i, SPL_VAR_SIZ, 1, actx->fp)) {
      goto err;
   }
   if (1 > fwrite(&p2->dat.i, SPL_CONST_SIZ, 1, actx->fp)) {
      goto err;
   }

   return true;

err:
   PTRACE(actx);
   return false;
}

static bool write_enterlike(asm_ctx_t *actx) {
   irnode_t *p1 = tree_chdat(actx->t, 0);

   if (1 > fwrite(&actx->n->dat.i, SPL_OPCODE_SIZ, 1, actx->fp)) {
      goto err;
   }
   if (1 > fwrite(&p1->dat.i, SPL_PERSON_SIZ, 1, actx->fp)) {
      goto err;
   }

   return true;

err:
   PTRACE(actx);
   return false;
}

static bool write_paramless_opcode(asm_ctx_t *actx) {
   size_t nmemb = fwrite(&actx->n->dat.i, SPL_OPCODE_SIZ, 1, actx->fp);

   if (nmemb != 1) {
      goto err;
   }

   return true;

err:
   PTRACE(actx);
   return false;
}

static bool write_pushlike(asm_ctx_t *actx) {
   irnode_t *p1 = tree_chdat(actx->t, 0);

   if (1 > fwrite(&actx->n->dat.i, SPL_OPCODE_SIZ, 1, actx->fp)) {
      goto err;
   }
   if (1 > fwrite(&p1->dat.i, SPL_VAR_SIZ, 1, actx->fp)) {
      goto err;
   }

   return true;

err:
   PTRACE(actx);
   return false;
}

static bool write_binary_op(asm_ctx_t *actx) {
   irnode_t *p1 = tree_chdat(actx->t, 0);
   irnode_t *p2 = tree_chdat(actx->t, 1);
   irnode_t *p3 = tree_chdat(actx->t, 2);

   if (1 > fwrite(&actx->n->dat.i, SPL_OPCODE_SIZ, 1, actx->fp)) {
      goto err;
   }
   if (1 > fwrite(&p1->dat.i, SPL_VAR_SIZ, 1, actx->fp)) {
      goto err;
   }
   if (1 > fwrite(&p2->dat.i, SPL_VAR_SIZ, 1, actx->fp)) {
      goto err;
   }
   if (1 > fwrite(&p3->dat.i, SPL_VAR_SIZ, 1, actx->fp)) {
      goto err;
   }

   return true;

err:
   PTRACE(actx);
   return false;
}

static bool write_unary_op(asm_ctx_t *actx) {
   irnode_t *p1 = tree_chdat(actx->t, 0);
   irnode_t *p2 = tree_chdat(actx->t, 1);

   if (1 < fwrite(&actx->n->dat.i, SPL_OPCODE_SIZ, 1, actx->fp)) {
      goto err;
   }
   if (1 < fwrite(&p1->dat.i, SPL_VAR_SIZ, 1, actx->fp)) {
      goto err;
   }
   if (1 < fwrite(&p2->dat.i, SPL_VAR_SIZ, 1, actx->fp)) {
      goto err;
   }

   return true;

err:
   PTRACE(actx);
   return false;
}

static bool write_goto(asm_ctx_t *actx) {
   irnode_t *p1 = tree_chdat(actx->t, 0);
   irnode_t *p2 = tree_chdat(actx->t, 1);

   tree_t *block = tree_parent(actx->t);
   tree_t *scene = tree_parent(block);
   tree_t *act = tree_parent(scene);
   tree_t *root = tree_parent(act);

   size_t a, s;
   tree_t *op;
   irnode_t *act_dat, *op_dat;

   switch (p1->dat.i) {
      case NODEKIND_ACT: {
         a = interpret_romnum(p2->dat.s.run) - 1;
         if (!(op = find_nearest_opcode(actx, root, a, 0, 0))) {
            goto err;
         }
         break;
      }

      case NODEKIND_SCENE: {
         act_dat = tree_dat(act);
         a = interpret_romnum(act_dat->dat.s.run) - 1;
         s = interpret_romnum(p2->dat.s.run) - 1;
         if (!(op = find_nearest_opcode(actx, root, a, s, 0))) {
            goto err;
         }
         break;
      }

      default: {
         actx->perr = "corrupted p1->dat.i";
         goto err;
      }
   }

   op_dat = tree_dat(op);

   if (1 < fwrite(&actx->n->dat.i, SPL_OPCODE_SIZ, 1, actx->fp)) {
      goto err;
   }
   if (1 < fwrite(&op_dat->offset, SPL_ADDR_SIZ, 1, actx->fp)) {
      goto err;
   }

   return true;

err:
   PTRACE(actx);
   return false;
}

static bool write_jumplike(asm_ctx_t *actx) {
   tree_t *root, *act, *scene, *block, *op;
   irnode_t *p1;
   irnode_t *act_dat, *scene_dat, *block_dat, *op_dat;
   size_t scene_siz, a, s, b;

   p1 = tree_chdat(actx->t, 0);

   // Search the matching block in reverse order
   block = tree_parent(actx->t);
   scene = tree_parent(block);

   scene_siz = tree_clen(scene);

   for (b = scene_siz; b > 1; b--) {
      block = tree_child(scene, b - 1);
      block_dat = tree_dat(block);
      if (block_dat->dat.i == p1->dat.i) {
         break;  // must exist
      }
   }

   // Get the first opcode of the requested block in the scene
   act = tree_parent(scene);
   root = tree_parent(act);

   act_dat = tree_dat(act);
   scene_dat = tree_dat(scene);

   a = interpret_romnum(act_dat->dat.s.run) - 1;
   s = interpret_romnum(scene_dat->dat.s.run) - 1;

   if (!(op = find_nearest_opcode(actx, root, a, s, b))) {
      goto err;
   }
   op_dat = tree_dat(op);

   if (1 < fwrite(&actx->n->dat.i, SPL_OPCODE_SIZ, 1, actx->fp)) {
      goto err;
   }
   if (1 < fwrite(&op_dat->offset, SPL_ADDR_SIZ, 1, actx->fp)) {
      goto err;
   }

   return true;

err:
   PTRACE(actx);
   return false;
}

static tree_t *find_nearest_opcode(
   asm_ctx_t *actx,
   tree_t *root,
   size_t a,
   size_t s,
   size_t b
) {
   tree_t *act, *scene, *block, *op;
   size_t nrtv_siz, act_siz, scene_siz;

   nrtv_siz = tree_clen(root);

   for (/* empty */; a < nrtv_siz; a++) {
      act = tree_child(root, a);
      act_siz = tree_clen(act);

      for (/* empty */; s < act_siz; s++) {
         scene = tree_child(act, s);
         scene_siz = tree_clen(scene);

         for (/* empty */; b < scene_siz; b++) {
            block = tree_child(scene, b);
            if (tree_clen(block) == 0) {
               continue;
            }
            op = tree_child(block, 0);
            return op;
         }
         b = 0;
      }
      s = 0;
   }

   // Control never reaches here
   actx->perr = "deviated control";
   PTRACE(actx);
   return NULL;
}

static bool write_debug_info(asm_ctx_t *actx) {
   fpos_t ecnt_pos, eos_pos;
   spl_uint_t before, diff;
   uint32_t ecnt;

   if (fgetpos(actx->fp, &ecnt_pos)) {
      goto err;
   }

   actx->offset += OBJFILE_DI_EC;
   before = actx->offset;

   if (!tree_pre_traverse(actx->nrtv, write_dbginfo_route, 0, actx)) {
      goto err;
   }
   if (fgetpos(actx->fp, &eos_pos)) {
      goto err;
   }

   diff = actx->offset - before;
   ecnt = diff / OBJFILE_DI_ETSIZ;

   if (fsetpos(actx->fp, &ecnt_pos)) {
      goto err;
   }
   if (actx->be) {
      ecnt = endrev32(ecnt);
   }
   if (1 > fwrite(&ecnt, OBJFILE_DI_EC, 1, actx->fp)) {
      goto err;
   }
   if (fsetpos(actx->fp, &eos_pos)) {
      goto err;
   }

   return true;

err:
   PTRACE(actx);
   return false;
}

static bool write_dbginfo_route(tree_t *t, size_t lv, void *ctx) {
   asm_ctx_t *actx = ctx;
   irnode_t *n = tree_dat(t);

   (void) lv;

   if (n->kind != IrnodekindOpcode) {
      return true;
   }

   // fixme: 프로젝트 전체적으로 lnum lpos 사이즈 spl_...로 통일
   // fixme: 프로젝트에서 size_t를 쓰는게 적절한곳엔 size_t 사용
   if (1 > fwrite(&actx->offset, OBJFILE_DI_ET_OP, 1, actx->fp)) {
      goto err;
   }
   if (1 > fwrite(&n->lnum, OBJFILE_DI_ET_SL, 1, actx->fp)) {
      goto err;
   }
   if (1 > fwrite(&n->lpos, OBJFILE_DI_ET_SP, 1, actx->fp)) {
      goto err;
   }

   actx->offset += OBJFILE_DI_ETSIZ;

   return true;

err:
   PTRACE(actx);
   return false;
}

static bool write_srcfile(asm_ctx_t *actx) {
   size_t lls = actx->lc;
   size_t cnt = 0;

   line_t *l;

   for (size_t i = 0; i < lls; i++) {
      l = array_peek(actx->ls, i);
      if (1 > fwrite(l->run, 1, l->len, actx->fp)) {
         goto err;
      }
   }

   for (size_t i = 0; i < lls; i++) {
      l = array_peek(actx->ls, i);
      cnt += l->len;
   }
   actx->offset += cnt;

   return true;

err:
   PTRACE(actx);
   return false;
}

static bool write_header(asm_ctx_t *actx) {
   uint32_t fi, s0p, s1p;

   fi = OBJFILE_FI;
   if (actx->le) fi = endrev32(fi);
   s0p = 0;
   s1p = OBJFILE_HDSIZ;

   // Write in accordance with .splo file structure

   // format indicator
   if (1 > fwrite(&fi, sizeof fi, 1, actx->fp)) {
      goto err;
   }

   // debug flag
   if (1 > fwrite(&actx->cfg->dbg, sizeof (char), 1, actx->fp)) {
      goto err;
   }

   // section 0 position
   if (1 > fwrite(&s0p, sizeof s0p, 1, actx->fp)) {
      goto err;
   }

   // section 1 position
   if (1 > fwrite(&s1p, sizeof s1p, 1, actx->fp)) {
      goto err;
   }

   // section 2 position
   if (1 > fwrite(&actx->s2p, sizeof actx->s2p, 1, actx->fp)) {
      goto err;
   }

   // section 3 position
   if (1 > fwrite(&actx->s3p, sizeof actx->s3p, 1, actx->fp)) {
      goto err;
   }

   return true;

err:
   PTRACE(actx);
   return false;
}
