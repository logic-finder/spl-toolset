#include "codegen2c.h"
#include "codegen2c.internals.h"

extern void transpile2c(void) {
   char *destname;
   FILE *fp;

   destname = make_destname("hello.spl", C_EXTENSION);
   fp = safe_fopen(destname, "w");

   fmtwrt(ENPREFIX "transpiling into " Cbwhite "C" Creset "...");

   // pt에서 title도 주석으로 출력
   // 맨처음 dpsz 설정
   // irt 순회
   // scene
   tree_pre_traverse(irt, codegen_route, 0, NULL);

   fmtwrt(" " Cgreen "done!" Creset "\n");

   safe_fclose(fp);
   free(destname);
}

static void codegen_route(tree_t *t, int lv, void *ctx) {
   irnode_t *n;

   n = tree_dat(t);

   switch (n->kind) {
      case IrnodekindScene  : handle_scene (t); break;
      case IrnodekindOpcode : handle_opcode(t); break;
      default: ;
   }
}

static void handle_scene(tree_t *t) {
   tree_t *act;
   irnode_t *act_dat, *scene_dat;

   act = tree_parent(t);
   act_dat = tree_dat(act);
   scene_dat = tree_dat(t);

   // ffmtwrt(
}
