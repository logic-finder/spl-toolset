#include "dbmaker.h"
#include "dbmaker.internals.h"

GENERIC_COMPARE(A)
GENERIC_COMPARE(B)

// fixme: header 제외하고 LE BE 제한 없애기
// 관련해 기존에 endrev32 적용하고있던거 헤더제외하고 제거
extern void dbmake(void) {
   /*
    * DATABASE STRUCTURE
    *    Metadata Section     : total 21 bytes
    *       header            - 4 bytes; BE; contains "SPDB"
    *       archive flag      - 1 byte
    *       name sect. pos.   - 4 bytes; LE
    *       adj. sect. pos.   - 4 bytes; LE
    *       noun sect. pos.   - 4 bytes; LE
    *       cmp. sect. pos.   - 4 bytes; LE
    *
    *    Name Section         : total 8 + (65 * n) bytes
    *       header            - 4 bytes; BE; contains "NAME"
    *       entry count       - 4 bytes; LE
    *       entry...          - each 65 bytes
    *          length         - 1 byte
    *          string         - 64 bytes
    *
    *    Adjective Section    : total 8 + (65 * n) bytes
    *       header            - 4 bytes; BE; contains "ADJ "
    *       entry count       - 4 bytes; LE
    *       entry...          - each 65 bytes
    *          length         - 1 byte
    *          string         - 64 bytes
    *
    *    Noun Section         : total 8 + (66 * n) bytes
    *       header            - 4 bytes; BE; contains "NOUN"
    *       entry count       - 4 bytes; LE
    *       entry...          - each 66 bytes
    *          kind           - 1 byte, contains 0 or 1
    *          length         - 1 byte
    *          string         - 64 bytes
    *
    *    Comparative Section  : total 8 + (66 * n) bytes
    *       header            - 4 bytes; BE; contains "COMP"
    *       entry count       - 4 bytes; LE
    *       entry...          - each 66 bytes
    *          kind           - 1 byte, contains 0 or 1
    *          length         - 1 byte
    *          string         - 64 bytes
    *
    * NOTE
    *    The name and adj. section have the identical form
    *    and so do the noun and comp section. In light of
    *    this, let's call the former and the latter 'type A'
    *    and 'type B', respectively.
    */
   FILE *fp;
   uint32_t tbytes;

   le = isle(), be = !le;
   dupflg = false;
   fp = safe_fopen(DBTEMPNAME, "wb");

   // Make DB
   print_srcfiles();
   safe_fseek(fp, MTDT_SIZ, SEEK_SET);

   safe_fputs(stdout, ENPREFIX "writing name section...");
   write_namesect(fp);
   DONE(SECTKIND_NAME, \t\t);

   safe_fputs(stdout, ENPREFIX "writing adjective section...");
   write_adjsect(fp);
   DONE(SECTKIND_ADJ, \t);

   safe_fputs(stdout, ENPREFIX "writing noun section...");
   write_nounsect(fp);
   DONE(SECTKIND_NOUN, \t\t);

   safe_fputs(stdout, ENPREFIX "writing comparative section...");
   write_compsect(fp);
   DONE(SECTKIND_COMP, \t);

   rewind(fp);

   safe_fputs(stdout, ENPREFIX "writing metadata section...");
   tbytes = write_metadata(fp);
   safe_vprintf(
      "\t" Cbgreen "done!" Creset
      " (" Cbwhite "%" PRIu32 Creset " bytes in total)\n",
      tbytes
   );

   // Rename
   safe_fclose(fp);
   (void) remove(DBFILENAME);
   safe_rename(DBTEMPNAME, DBFILENAME);
}

static void handle_dberr(void) {
   safe_fputc(stdout, '\n');
   exit(EXIT_FAILURE);
}

static void print_srcfiles(void) {
   safe_fputs(stdout, ENPREFIX "use the following files to make a database:\n");
   SRCFILE(name, 0);
   SRCFILE(adjective, 1);
   SRCFILE(noun, 2);
   SRCFILE(comparative, 3);
}

static uint32_t write_metadata(FILE *fp) {
   uint32_t header;
   uint8_t arcflg;
   uint32_t pos;

   header = HEADER_METADATA;
   arcflg = Arcflg_f;

   // header
   if (le) header = endrev32(header);
   safe_fwrite(&header, 1, MTDT_HD, fp);

   // archive flag
   safe_fwrite(&arcflg, MTDT_AF, 1, fp);

   // section positions
   int dtsizs[SECTNUM] = {
      NAME_DTSIZ, ADJ_DTSIZ, NOUN_DTSIZ, COMP_DTSIZ
   };
   int mtdtsizs[SECTNUM] = {
      NAME_MTDTSIZ, ADJ_MTDTSIZ, NOUN_MTDTSIZ, COMP_MTDTSIZ
   };

   pos = MTDT_SIZ;
   for (int i = 0; i < SECTNUM; i++) {
      if (be) pos = endrev32(pos);
      safe_fwrite(&pos, MTDT_SP, 1, fp);
      pos += mtdtsizs[i];
      pos += ecnts[i] * dtsizs[i];
   }

   return pos;
}

static void write_sect_type_A(
   FILE *fp,
   const char *sectname,
   sectkind_t sectkind,
   uint32_t header,
   int hd_len,
   int ec_len,
   int dt_l_len,
   int dt_s_len,
   array_sorter_t *compare
) {
   FILE *src;
   fpos_t ecnt_pos, eos_pos;
   char *ln;
   int llen, residual;
   uint32_t ecnt;
   array_t *records;
   record_A_t temp, *r;

   src = safe_fopen(ov.mak[sectkind], "rb");
   records = array_create();
   ecnt = 0;

   // Write header
   if (le) header = endrev32(header);
   safe_fwrite(&header, 1, hd_len, fp);
   safe_fgetpos(fp, &ecnt_pos);

   // Store entries
   while (!readln(src, &ln, &llen)) {
      if (lastch(ln) == '\n') {
         llen--;  /* why -1? since ln has \n */
         ln[llen] = '\0';
      }

      if (llen > dt_s_len)
         VERR("%s entry #%d too long", sectname, ecnt + 1);

      ecnt++;
      temp.len = llen;
      temp.run = ln;
      temp.lnum = ecnt;
      array_append(records, &temp, sizeof temp);
   }
   if (!ecnt) VERR("empty %s file", sectname);
   safe_fclose(src);

   // Sort!
   array_sort(records, compare);
   if (dupflg) handle_dberr();

   // Write the records
   safe_fseek(fp, ec_len, SEEK_CUR);
   for (uint32_t i = 0; i < ecnt; i++) {
      r = array_peek(records, i);
      safe_fwrite(&r->len, dt_l_len, 1, fp);
      safe_fwrite(r->run, 1, r->len, fp);
      residual = dt_s_len - r->len;
      for (int k = 0; k < residual; k++)
         safe_fputc(fp, '\0');  /* zero-padding */
      free(r->run);  /* free ln */
   }
   array_destroy(records);
   safe_fgetpos(fp, &eos_pos);  /* end of section */

   // Write entry count
   safe_fsetpos(fp, &ecnt_pos);
   if (be) ecnt = endrev32(ecnt);
   safe_fwrite(&ecnt, ec_len, 1, fp);
   ecnts[sectkind] = ecnt;

   // Prepare the next section
   safe_fsetpos(fp, &eos_pos);
}

static void write_sect_type_B(
   FILE *fp,
   const char *sectname,
   sectkind_t sectkind,
   uint32_t header,
   int hd_len,
   int ec_len,
   int dt_k_len,
   int dt_l_len,
   int dt_s_len,
   array_sorter_t *compare
) {
   FILE *src;
   fpos_t ecnt_pos, eos_pos;
   char
      *ln,      // a line; form = <str>,<kind>
      **elems,  // = split(ln, ",")
      *str,     // elems[0]
      *kind;    // elems[1]
   int
      llen,    // length of ln
      elen,    // length of elems
      slen,    // length of <str>
      klen,    // length of <kind>
      maxlen,  // maximum length of ln = 66 chars long
      residual;
   uint32_t ecnt;
   array_t *records;
   record_B_t temp, *r;

   src = safe_fopen(ov.mak[sectkind], "rb");
   records = array_create();
   maxlen =
      dt_s_len     // 64
      + 1          // comma = 1 byte
      + dt_k_len;  // 1
   ecnt = 0;

   // Write header
   if (le) header = endrev32(header);
   safe_fwrite(&header, 1, hd_len, fp);
   safe_fgetpos(fp, &ecnt_pos);

   // Store entries
   while (!readln(src, &ln, &llen)) {
      if (lastch(ln) == '\n') {
         llen--;
         ln[llen] = '\0';  /* overwrite \n */
      }

      // fixme: replace all VERRs in this file with dberr function (use stdout, not stderr)
      if (llen > maxlen)
         VERR("%s entry #%d too long", sectname, ecnt + 1);

      elems = split(ln, ",", &elen);
      if (elen != 2)
         VERR("%s entry #%d has invalid form", sectname, ecnt + 1);

      str = elems[0];
      kind = elems[1];
      slen = strlen(str);
      klen = strlen(kind);
      if (klen != 1 || !match(kind[0], "01"))
         VERR("%s entry #%d has invalid value", sectname, ecnt + 1);

      ecnt++;
      temp.kind = kind[0] == '0' ? 0 : 1;
      temp.len = slen;
      temp.run = str;  /* to be freed later */
      temp.lnum = ecnt;
      array_append(records, &temp, sizeof temp);

      free(kind);
      free(elems);
      free(ln);
   }
   if (!ecnt) VERR("empty %s file", sectname);
   safe_fclose(src);

   // Sort!
   array_sort(records, compare);
   if (dupflg) handle_dberr();

   // Write the records
   safe_fseek(fp, ec_len, SEEK_CUR);
   for (uint32_t i = 0; i < ecnt; i++) {
      r = array_peek(records, i);
      safe_fwrite(&r->kind, dt_k_len, 1, fp);
      safe_fwrite(&r->len, dt_l_len, 1, fp);
      safe_fwrite(r->run, 1, r->len, fp);
      residual = dt_s_len - r->len;
      for (int k = 0; k < residual; k++)
         safe_fputc(fp, '\0');
      free(r->run);  /* free elems[0] */
   }
   array_destroy(records);
   safe_fgetpos(fp, &eos_pos);

   // Write entry count
   safe_fsetpos(fp, &ecnt_pos);
   if (be) ecnt = endrev32(ecnt);
   safe_fwrite(&ecnt, ec_len, 1, fp);
   ecnts[sectkind] = ecnt;

   // Prepare the next section
   safe_fsetpos(fp, &eos_pos);
}

static void write_namesect(FILE *fp) {
   write_sect_type_A(
      fp,
      "name",
      SECTKIND_NAME,
      HEADER_NAMESECT,
      NAME_HD,
      NAME_EC,
      NAME_DT_L,
      NAME_DT_S,
      compare_name
   );
}

static void write_adjsect(FILE *fp) {
   write_sect_type_A(
      fp,
      "adj",
      SECTKIND_ADJ,
      HEADER_ADJSECT,
      ADJ_HD,
      ADJ_EC,
      ADJ_DT_L,
      ADJ_DT_S,
      compare_adj
   );
}

static void write_nounsect(FILE *fp) {
   write_sect_type_B(
      fp,
      "noun",
      SECTKIND_NOUN,
      HEADER_NOUNSECT,
      NOUN_HD,
      NOUN_EC,
      NOUN_DT_K,
      NOUN_DT_L,
      NOUN_DT_S,
      compare_noun
   );
}

static void write_compsect(FILE *fp) {
   write_sect_type_B(
      fp,
      "comp",
      SECTKIND_COMP,
      HEADER_COMPSECT,
      COMP_HD,
      COMP_EC,
      COMP_DT_K,
      COMP_DT_L,
      COMP_DT_S,
      compare_comp
   );
}

static int compare_name(
   const void *_lhs,
   const void *_rhs
) {
   return compare_rec_A(_lhs, _rhs, "name");
}

static int compare_adj(
   const void *_lhs,
   const void *_rhs
) {
   return compare_rec_A(_lhs, _rhs, "adjective");
}

static int compare_noun(
   const void *_lhs,
   const void *_rhs
) {
   return compare_rec_B(_lhs, _rhs, "noun");
}

static int compare_comp(
   const void *_lhs,
   const void *_rhs
) {
   return compare_rec_B(_lhs, _rhs, "comparative");
}
