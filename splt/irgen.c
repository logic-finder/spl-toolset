#include "irgen.h"
#include "irgen.type.h"

extern const char *sfname;  // see global.h

static int actcnt, scenecnt;
static bool p2_mentioned;
static int teller;
static arr_t *irtoks;

extern void generate_ir(optflg_t *of, optval_t *ov) {
   tree_t *nartiv;

   nartiv = tree_child(pt, 2);
   actcnt = scenecnt = 0;
   p2_mentioned = false;
   irtoks = arr_create();

   gen_dpsz();
   tree_pre_traverse(nartiv, generate, 0);
}

static void write_irfile(void) {
   static FILE *fp;
   char *basename, *buf;

   basename = extfnm(sfname, false);
   buf = smalloc(strlen(basename) + strlen(IR_EXTENSION) + 1);
   strcpy(buf, basename);
   strcat(buf, IR_EXTENSION);
   fp = sfopen(buf, "w");

   // TODO: iterate irtoks and write contents

   sfclose(fp);
   free(basename);
   free(buf);
}

static void mark_inst(ir_inst_t instkind, int lnum, int lpos) {
   irtok_inst_t inst;

   inst.kind = Irtokkind_inst;
   inst.val = instkind;
   inst.lnum = lnum;
   inst.lpos = lpos;
   arr_append(irtoks, &inst, sizeof inst);
}

static void mark_param(irtokkind_t kind, int val) {
   irtok_param_t param;

   param.kind = kind;
   param.val = val;
   arr_append(irtoks, &param, sizeof param);
}

static void gen_dpsz(void) {
   tree_t *dp;
   node_t *n;
   int dpsz;

   dp = tree_child(pt, 1);
   dpsz = tree_clen(dp);

   mark_inst(Ir_inst_set, n->lnum, n->lpos);
   mark_param(Irtokkind_var, Ir_var_dpsz);
   mark_param(Irtokkind_const, dpsz);
}

static void generate(tree_t *t, int _) {
   node_t *n;

   (void) _;
   n = tree_dat(t);

   switch (n->kind) {
      case NODEKIND_ACT   : gen_act  (t); break;
      case NODEKIND_SCENE : gen_scene(t); break;
   }
}

static void gen_act(tree_t *t) {
   node_t *n;

   n = tree_dat(t);
   mark_inst(Ir_inst_act, n->lnum, n->lpos);
   mark_param(Irtokkind_const, actcnt++);
}

static void gen_scene(tree_t *t) {
   node_t *n;

   n = tree_dat(t);
   mark_inst(Ir_inst_scene, n->lnum, n->lpos);
   mark_param(Irtokkind_const, scenecnt++);
}

static void gen_enter(tree_t *t) {
   node_t *n;
   int clen;

   clen = tree_clen(t);
   for (int i = 0; i < clen; i++) {
      n = tree_chdat(t, i);
      mark_inst(Ir_inst_enter, n->lnum, n->lpos);
      mark_param(Irtokkind_char, n->dat.n);
   }
}

static void gen_exit(tree_t *t) {
   node_t *n;

   n = tree_chdat(t, 0);
   mark_inst(Ir_inst_exit, n->lnum, n->lpos);
   mark_param(Irtokkind_char, n->dat.n);
}

static void gen_exeunt(tree_t *t) {
   node_t *n;
   int clen;

   clen = tree_clen(t);
   if (!clen)
      mark_inst(Ir_inst_exeunt, n->lnum, n->lpos);
   else
      for (int i = 0; i < clen; i++) {
         n = tree_chdat(t, i);
         mark_inst(Ir_inst_exit, n->lnum, n->lpos);
         mark_param(Irtokkind_char, n->dat.n);
      }
}

static void gen_line(tree_t *t) {
   node_t *n;

   n = tree_chdat(t, 0);  /* NODEKIND_CHAR */
   speaker = n->dat.n;
   putinst(n, IR_INST_SET);
   addstr(n, "speaker");
   putint(n, speaker);
}

static void gen_asgn(tree_t *t) {
   node_t *n;
   n = tree_dat(t);
   putinst(n, IR_INST_CHECK2);
   putinst(n, IR_INST_SET);
   addstr(n, "listener");
   putint(n, speaker);
}

static void gen_outn(tree_t *t) {
   node_t *n;
   n = tree_dat(t);
   putinst(n, IR_INST_OUT);
   putint(n, 0);
   place_char(n, speaker);
}
