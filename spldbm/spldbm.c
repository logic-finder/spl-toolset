#include "spldbm.type.h"

int main(int argc, const char **argv) {
   if (argc == 1)
      ERR("type --help to see a manual page");

   parse_args(argv);

        if (of.mak) dbmake();
   else if (of.arc) archive();
   else if (of.res) restore();
   else if (of.hlp) ;
   else if (of.vsn) ;

   // ov.mak 요소 및 그 자체 해제

   return 0;
}
