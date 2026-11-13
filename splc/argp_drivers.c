#include "argp_drivers.h"
#include "argp_drivers.internals.h"

// TODO: it might be a good thing to get &cfg from the parameter

static bool drive_help(argtok_t *tok, argerr_t *err) {
   (void) tok, (void) err;
   cfg.hlp = true;
   return true;
}

static bool drive_version(argtok_t *tok, argerr_t *err) {
   (void) tok, (void) err;
   cfg.vsn++;  // assumes cfg.vsn has been initialized
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

   // --job 128
   //       ^~~
   //       tok->value

   // Check if tok->value begins with '-'
   if ('-' == tok->value[strspn(tok->value, whitespaces)]) {
      err->reason = "tok->value begins with '-'";
      goto err;
   }

   // Parse tok->value
   errno = 0;
   ret = strtoul(tok->value, &end, 10);

   if (errno == ERANGE) {
      err->reason = "a value out of range";
      goto err;
   }
   if (end == tok->value) {
      err->reason = "conversion not performed";
      goto err;
   }
   if (end[0] != '\0') {
      err->reason = "value not fully consumed";
      goto err;
   }

   cfg.job = ret;
   return true;

err:
   err->errhandle = argp_errhandle_default;
   return false;
}

static bool drive_output(argtok_t *tok, argerr_t *err) {
   (void) err;

   if (!(cfg.out = strdup(tok->value))) {
      err->reason = "failed to strdup tok->value";
      goto err;
   }

   return true;

err:
   err->errhandle = argp_errhandle_default;
   return false;
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
   size_t i;

   for (i = 0; i < accept_set_len; i++) {
      map = &accept_set[i];
      if (!strcmp(tok->value, map->spec)) {
         cfg.std = map->enum_val;
         break;
      }
   }
   if (i == accept_set_len) {
      goto err;
   }

   return true;

err:
   static const errhandle_unrecognized_value_ctx_t ctx = {
      .arr = accept_set,
      .nmemb = accept_set_len,
      .esiz = ESIZ(accept_set),
      .offset = offsetof(mapper_t, spec)
   };

   err->errhandle = errhandle_unrecognized_value;
   err->ctx = &ctx;
   err->ctx_siz = sizeof ctx;
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
   size_t i;

   for (i = 0; i < accept_set_len; i++) {
      map = &accept_set[i];
      if (!strcmp(tok->value, map->lang)) {
         cfg.lng = map->enum_val;
         break;
      }
   }
   if (i == accept_set_len) {
      goto err;
   }

   return true;

err:
   static const errhandle_unrecognized_value_ctx_t ctx = {
      .arr = accept_set,
      .nmemb = accept_set_len,
      .esiz = ESIZ(accept_set),
      .offset = offsetof(mapper_t, lang)
   };

   err->errhandle = errhandle_unrecognized_value;
   err->ctx = &ctx;
   err->ctx_siz = sizeof ctx;
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
   size_t i;

   // Note: since strcasecmp() is used, all the following
   // would be treated as the same:
   //    haskell Haskell HASKELL etc...

   for (i = 0; i < accept_set_len; i++) {
      map = &accept_set[i];
      if (!strcasecmp(tok->value, map->target)) {
         cfg.tgt = map->enum_val;
         break;
      }
   }
   if (i == accept_set_len) {
      goto err;
   }

   return true;

err:
   static const errhandle_unrecognized_value_ctx_t ctx = {
      .arr = accept_set,
      .nmemb = accept_set_len,
      .esiz = ESIZ(accept_set),
      .offset = offsetof(mapper_t, target)
   };

   err->errhandle = errhandle_unrecognized_value;
   err->ctx = &ctx;
   err->ctx_siz = sizeof ctx;
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
   size_t i;

   for (i = 0; i < accept_set_len; i++) {
      map = &accept_set[i];
      if (!strcmp(tok->value, map->request)) {
         *map->addr = true;
         break;
      }
   }
   if (i == accept_set_len) {
      goto err;
   }

   return true;

err:
   static const errhandle_unrecognized_value_ctx_t ctx = {
      .arr = accept_set,
      .nmemb = accept_set_len,
      .esiz = ESIZ(accept_set),
      .offset = offsetof(mapper_t, request)
   };

   err->errhandle = errhandle_unrecognized_value;
   err->ctx = &ctx;
   err->ctx_siz = sizeof ctx;
   return false;
}

static bool errhandle_unrecognized_value(
   argtok_t *tok,
   const char *reason,
   void *ctx
) {
   bool stat = false;
   errhandle_unrecognized_value_ctx_t *ectx = ctx;
   find_alikes_ret_t ret = { .alikes = NULL };

   const char *err;

   (void) reason;
   ret = find_alikes(
      tok->value,
      ectx->arr,
      ectx->nmemb,
      ectx->esiz,
      ectx->offset
   );
   if (!ret.alikes) {
      err = "failed to find alikes";
      goto cleanup;  // something wrong: size or alloc issue
   }

   // Set common error message
   err = "failed to printf";

   if (!log(
      "%s: unrecognized value " Cbwhite "'%s'" Creset ".",
      tok->field, tok->value
   )) {
      goto cleanup;
   }

   if (!log(" Do you mean ")) {
      goto cleanup;
   }

   if (ret.nmemb == 1) {
      if (!log(Cbgreen "'%s'" Creset "?\n", ret.alikes[0])) {
         goto cleanup;
      }
   }
   else {
      if (!log("one of the following?\n\t" Cbgreen)) {
         goto cleanup;
      }

      for (size_t i = 0; i < ret.nmemb; i++) {
         if (!log("%s  ", ret.alikes[i])) {
            goto cleanup;
         }
      }

      if (!log(Creset "\n")) {
         goto cleanup;
      }
   }

   stat = true;

cleanup:
   free(ret.alikes);

   if (!stat) {
      goto err;
   }

   return true;

err:
   FATAL(err);
   return false;
}
