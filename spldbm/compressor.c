#include "compressor.h"
#include "compressor.internals.h"

extern void archive(void) {
   work_template(
      ARCHIVE_PREFIX,
      ov.arc,
      rle,
      Arcflg_t
   );
}

extern void restore(void) {
   work_template(
      RESTORE_PREFIX,
      ov.res,
      rrle,
      Arcflg_f
   );
}

static bool is_archived(FILE *src) {
   uint8_t af;
   int ret;

   safe_fseek(src, MTDT_HD, SEEK_SET);
   ret = fread(&af, MTDT_AF, 1, src);
   if (ret < 1)
      ERR("fread error");
   if (af != 0 && af != 1)
      ERR("bad archive flag value");

   return af == Arcflg_t ? true : false;
}

static void work_template(
   const char *prefix,
   const char *srcname,
   worker_t *work,
   uint8_t af
) {
   FILE *src, *dest;
   char *destname;
   bool archived;

   src = safe_fopen(srcname, "rb");
   archived = is_archived(src);
   if ((af == 1 && archived)) {
      fmtwrt(ENPREFIX "this DB is already archived; terminating\n");
      exit(2);
   }
   else
   if ((af == 0 && !archived)) {
      fmtwrt(ENPREFIX "this DB is not archived; terminating\n");
      exit(3);
   }
   rewind(src);  /* is_archived moved forward the file position */

   destname = safe_malloc(
      strlen(srcname)
      + strlen(prefix)
      + 1
   );
   strcpy(destname, srcname); // fixme: consider memcpy
   strcat(destname, prefix);
   dest = safe_fopen(destname, "wb");

   write_metadata(src, dest, af);
   (*work)(src, dest);

   free(destname);
   safe_fclose(src);
   safe_fclose(dest);
}

static void write_metadata(FILE *src, FILE *dest, uint8_t af) {
   int ch;

   // Write metadata section
   for (int i = 0; i < MTDT_SIZ; i++) {
      ch = getc(src);
      if (ch == EOF && ferror(src))
         ERR("getc error");
      sfputc(dest, ch);
   }

   // Update the archive flag
   safe_fseek(dest, MTDT_HD, SEEK_SET);
   sfwrite(&af, MTDT_AF, 1, dest);

   // Prepare the next work
   safe_fseek(dest, MTDT_SIZ, SEEK_SET);
}

static void rle(FILE *src, FILE *dest) {
   int ch, cmp, orig_siz, arc_siz;
   uint8_t cnt;

   safe_fputs(stdout, ENPREFIX "using run-length encoding...\n");

   cmp = -1;
   orig_siz = arc_siz = MTDT_SIZ;
   for (;;) {
      ch = getc(src);
      if (ch != cmp || ch == EOF || cnt == UINT8_MAX) {
         if (ferror(src)) ERR("getc error");
         if (cmp != -1) {
            sfwrite(&cnt, 1, 1, dest);
            sfwrite(&cmp, 1, 1, dest);
            orig_siz += cnt;
            arc_siz += 2;
         }
         if (feof(src)) break;
         cnt = 1;
         cmp = ch;
      }
      else cnt++;
   }

   fmtwrt(ENPREFIX
      "archiving done! %d bytes -> %d bytes (%d%%)\n",
      orig_siz, arc_siz, (arc_siz * 100 / orig_siz)
   );
}

static void rrle(FILE *src, FILE *dest) {
   enum { Cnt, Ch };
   uint8_t pair[2];
   size_t ret;
   int orig_siz, res_siz;

   safe_fputs(stdout, ENPREFIX "decoding run-length encoding...\n");

   orig_siz = res_siz = MTDT_SIZ;
   for (;;) {
      ret = fread(pair, 1, 2, src);
      if (ret == 1)
         ERR("invalid archive file; perhaps corrupted?");
      if (ret == 0) {
         if (ferror(src)) ERR("fread error");
         break;  /* EOF */
      }
      orig_siz += 2;
      res_siz += pair[Cnt];
      while (pair[Cnt]--)
         sfputc(dest, pair[Ch]);
   }
   fmtwrt(ENPREFIX
      "restoring done! %d bytes -> %d bytes (%d%%)\n",
      orig_siz, res_siz, (res_siz * 100 / orig_siz)
   );
}
