#include "argp_drivers.h"
#include "argp_drivers.internals.h"

/* Note. it might be a good thing to get &cfg from the parameter */

static bool drive_help(argtok_t *tok, argerr_t *err) {
   (void) tok, (void) err;
   cfg.hlp = true;
   return true;
}

static bool drive_debug(argtok_t *tok, argerr_t *err) {
   (void) tok, (void) err;
   cfg.dbg = true;
   return true;
}

static bool drive_kawaii(argtok_t *tok, argerr_t *err) {
   (void) tok, (void) err;
   cfg.kwi = true;
   return true;
}

static bool drive_version(argtok_t *tok, argerr_t *err) {
   (void) tok, (void) err;
   cfg.vsn = true;
   return true;
}

static bool drive_dump(argtok_t *tok, argerr_t *err) {
   (void) tok, (void) err;
   cfg.dmp = true;
   return true;
}

static bool drive_optimize(argtok_t *tok, argerr_t *err) {
   (void) tok, (void) err;
   cfg.opt = true;
   return true;
}

static bool drive_direct(argtok_t *tok, argerr_t *err) {
   (void) tok, (void) err;
   cfg.drt = true;
   return true;
}

static bool drive_verbose(argtok_t *tok, argerr_t *err) {
   (void) tok, (void) err;
   cfg.vbs = true;
   return true;
}

static bool drive_dryrun(argtok_t *tok, argerr_t *err) {
   (void) tok, (void) err;
   cfg.drn = true;
   return true;
}

static bool drive_job(argtok_t *tok, argerr_t *err) {
   char *end;
   unsigned long ret;

   errno = 0;
   ret = strtoul(tok->value, &end, 10);

   err->errhandle = errhandle_default;

   if (errno == ERANGE || ret > ULONG_MAX) {
      err->reason = "a value out of range";
      return false;
   }
   else
   if (end == tok->value) {
      err->reason = "conversion not performed";
      return false;
   }
   else
   if (end[0] != '\0') {
      err->reason = "value not fully consumed";
      return false;
   }

   cfg.job = ret;
   return true;
}

static bool drive_output(argtok_t *tok, argerr_t *err) {
   (void) err;
   cfg.out = strdup(tok->value);
   return true;
}

static bool drive_standard(argtok_t *tok, argerr_t *err) {
   typedef struct {
      const char *spec;
      stdopt_t enum_val;
   } mapper_t;

   static const mapper_t accept_set[] = {
      { "spl01", StdoptSpl01 },
      { "cor27", StdoptCor27 }
   };
   static const size_t accept_set_len = ARRLEN(accept_set);

   const mapper_t *map;

   for (size_t i = 0; i < accept_set_len; i++) {
      map = &accept_set[i];
      if (!strcmp(tok->value, map->spec)) {
         cfg.std = map->enum_val;
         return true;
      }
   }

   /* error path */
   err->errhandle = errhandle_unrecognized;
   return false;
}

static bool drive_language(argtok_t *tok, argerr_t *err) {
   typedef struct {
      const char *lang;
      lngopt_t enum_val;
   } mapper_t;

   static const mapper_t accept_set[] = {
      { "en", LngoptEn },
      { "ko", LngoptKo }
   };
   static const size_t accept_set_len = ARRLEN(accept_set);

   const mapper_t *map;

   for (size_t i = 0; i < accept_set_len; i++) {
      map = &accept_set[i];
      if (!strcmp(tok->value, map->lang)) {
         cfg.lng = map->enum_val;
         return true;
      }
   }

   /* error path */
   err->errhandle = errhandle_unrecognized;
   return false;
}

static bool drive_target(argtok_t *tok, argerr_t *err) {
   typedef struct {
      const char *target;
      tgtopt_t enum_val;
   } mapper_t;

   static const mapper_t accept_set[] = {
      { "c", TgtoptC },
   };
   static const size_t accept_set_len = ARRLEN(accept_set);

   const mapper_t *map;

   /* Note: since strcasecmp() is used, all the following
      would be treated as the same:
         haskell Haskell HASKELL etc... */

   for (size_t i = 0; i < accept_set_len; i++) {
      map = &accept_set[i];
      if (!strcasecmp(tok->value, map->target)) {
         cfg.tgt = map->enum_val;
         return true;
      }
   }

   /* error path */
   err->errhandle = errhandle_unrecognized;
   return false;
}

static bool drive_asmbly(argtok_t *tok, argerr_t *err) {
   (void) tok, (void) err;
   cfg.asmbly = true;
   return true;
}

static bool drive_disasm(argtok_t *tok, argerr_t *err) {
   (void) tok, (void) err;
   cfg.disasm = true;
   return true;
}

static bool drive_pedantic(argtok_t *tok, argerr_t *err) {
   (void) tok, (void) err;
   cfg.pdt = true;
   return true;
}

static bool drive_warning(argtok_t *tok, argerr_t *err) {
   typedef struct {
      const char *request;
      bool *addr;
   } mapper_t;

   static const mapper_t accept_set[] = {
      { "keyword-case" , &cfg.w_kc },
      { "be-predicate" , &cfg.w_bp }
   };
   static const size_t accept_set_len = ARRLEN(accept_set);

   const mapper_t *map;

   for (size_t i = 0; i < accept_set_len; i++) {
      map = &accept_set[i];
      if (!strcmp(tok->value, map->request)) {
         *map->addr = true;
         return true;
      }
   }

   /* error path */
   err->errhandle = errhandle_unrecognized;
   return false;
}

static void errhandle_unrecognized(argtok_t *tok, const char *reason, void *ctx) {
   (void) reason, (void) ctx;
   safe_vprintf("%s: unrecognized value '%s'\n", tok->field, tok->value);
}
