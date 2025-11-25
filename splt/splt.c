#include <ctype.h>
#include <string.h>
#include "parse.h"
#include "global.h"
#include "wrapper.h"
#include "loadfile.h"
#include "argparse.h"

int main(int argc, const char **argv) {
   optflg_t of = {0};
   optval_t ov;

   parse_args(argv, &of, &ov);

   line_t *lines;
   int cnt;

   lines = loadfile(ov.src, &cnt);
   if (!lines)
      ERR("syntax error: empty source file.");
   fmtwrt(ENPREFIX "load the source file: \033[0;32m%s\033[0m\n", ov.src);

   tree_t *pt;

   pt = tree_plant(NULL, 0);
   strcpy(pt->tag, "root");

   parse(&of, &ov, lines, cnt, pt);

   // prune pt
   return 0;
}
