// TODO: init opt, handle h,v (refer proofread)

#include "optprocessor.h"
#include "optprocessor.internals.h"

extern void process_opts(
   int argc,
   optflg_t *of,
   optval_t *ov
) {
   validate_argc(argc);
   if (!ov->src)
      ERR("no source file given; terminating");
   init_extvar(ov);
}

static void validate_argc(int argc) {
   if (argc == 1)
      ERR("executed with no argument\n"
         "Note. type -h or --help to see a manual");
}

static void init_extvar(optval_t *ov) {
   sfname = ov->src;
}
