#include "spla.internals.h"

extern optflg_t of;
extern optval_t ov;

int main(int argc, char **argv) {
   parse_args(argv);
   if (!ov.src)
      ERR("no object file given; terminating");
   // loaded the object file ...
   // setup the virtual environment ...
   // readobj(ov.src);
}
