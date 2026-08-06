#include "dbhandler.h"
#include "dbhandler.internals.h"

extern void dbload(void) {
   dbcheck();
   load_section(SECTKIND_NAME, NAME_DTSIZ);
   load_section(SECTKIND_ADJ ,  ADJ_DTSIZ);
   load_section(SECTKIND_NOUN, NOUN_DTSIZ);
   load_section(SECTKIND_COMP, COMP_DTSIZ);
   safe_vprintf(ENPREFIX
      "loaded the database " Cbyellow DBFILENAME Creset " ("
      Cbwhite "%d" Creset " names, " Cbwhite "%d" Creset " adjs, "
      Cbwhite "%d" Creset " nouns, " Cbwhite "%d" Creset " cmps)\n",
      ecnts[SECTKIND_NAME], ecnts[SECTKIND_ADJ],
      ecnts[SECTKIND_NOUN], ecnts[SECTKIND_COMP]
   );
   safe_fclose(db);
}

extern void dbunload(void) {
   for (int i = 0; i < SECTNUM; i++)
      free(sects[i]);
}

static void load_section(sectkind_t kind, size_t esiz) {
   unsigned int ret;

   sects[kind] = safe_malloc(ecnts[kind] * esiz);
   safe_fseek(db, secpos[kind], SEEK_SET);
   ret = fread(sects[kind], esiz, ecnts[kind], db);
   if (ret < ecnts[kind])
      ERR("fread error");
}

static void dbcheck(void) {
   int ret;
   uint32_t header;
   uint8_t af;

   le = isle(), be = !le;
   db = safe_fopen(DBFILENAME, "rb");

   // Check metadata section header
   ret = fread(&header, MTDT_HD, 1, db);
   if (ret < 1) ERR("fread error");
   if (le) header = endrev32(header);
   if (header != HEADER_METADATA)
      dberr(msgs.sys.db.corrupted);

   // Check metadata section archive flag
   ret = fread(&af, MTDT_AF, 1, db);
   if (ret < 1) ERR("fread error");
   if (af == Arcflg_t)
      dberr(msgs.sys.db.archived);

   // Read section positions
   ret = fread(secpos, MTDT_SP, SECTNUM, db);
   if (ret < SECTNUM) ERR("fread error");
   if (be) for (int i = 0; i < SECTNUM; i++)
      secpos[i] = endrev32(secpos[i]);

   // Check section header & read entry count
   check_secthead(SECTKIND_NAME, NAME_HD, HEADER_NAMESECT);
   read_ecnt(SECTKIND_NAME, NAME_EC);

   check_secthead(SECTKIND_ADJ, ADJ_HD, HEADER_ADJSECT);
   read_ecnt(SECTKIND_ADJ, ADJ_EC);

   check_secthead(SECTKIND_NOUN, NOUN_HD, HEADER_NOUNSECT);
   read_ecnt(SECTKIND_NOUN, NOUN_EC);

   check_secthead(SECTKIND_COMP, COMP_HD, HEADER_COMPSECT);
   read_ecnt(SECTKIND_COMP, COMP_EC);

   // Update section positions
   secpos[SECTKIND_NAME] += NAME_MTDTSIZ;
   secpos[SECTKIND_ADJ ] +=  ADJ_MTDTSIZ;
   secpos[SECTKIND_NOUN] += NOUN_MTDTSIZ;
   secpos[SECTKIND_COMP] += COMP_MTDTSIZ;
}

static void check_secthead(
   sectkind_t kind,
   int hdsiz,
   uint32_t against
) {
   int ret;
   uint32_t header;

   safe_fseek(db, secpos[kind], SEEK_SET);
   ret = fread(&header, hdsiz, 1, db);
   if (ret < 1) ERR("fread error");
   if (le) header = endrev32(header);
   if (header != against)
      dberr(msgs.sys.db.corrupted);
}

static void read_ecnt(sectkind_t kind, int ecntsiz) {
   int ret;

   ret = fread(&ecnts[kind], ecntsiz, 1, db);
   if (ret < 1) ERR("fread error");
   if (be) ecnts[kind] = endrev32(ecnts[kind]);
}

extern bool query_name(const char *key) {
   void *record = bsearch(
      key,
      sects[SECTKIND_NAME],
      ecnts[SECTKIND_NAME],
      NAME_DTSIZ,
      compare_rec_A
   );
   return record ? true : false;
}

extern bool query_adj(const char *key) {
   void *record = bsearch(
      key,
      sects[SECTKIND_ADJ],
      ecnts[SECTKIND_ADJ],
      ADJ_DTSIZ,
      compare_rec_A
   );
   return record ? true : false;
}

extern bool query_noun(const char *key, int *ret) {
   void *record = bsearch(
      key,
      sects[SECTKIND_NOUN],
      ecnts[SECTKIND_NOUN],
      NOUN_DTSIZ,
      compare_rec_B
   );
   if (!record)
      return false;
   *ret = ((char *) record)[0];
   return true;
}

extern bool query_comp(const char *key, int *ret) {
   void *record = bsearch(
      key,
      sects[SECTKIND_COMP],
      ecnts[SECTKIND_COMP],
      COMP_DTSIZ,
      compare_rec_B
   );
   if (!record)
      return false;
   *ret = ((char *) record)[0];
   return true;
}

static int compare_rec_A(const void *key, const void *elem) {
   /* elem = a pointer to an array member whose size is 65 bytes */
   const char *e;
   int len;

   e = elem;  /* treat elem as a (char *) */
   len = e[0];

   return strncmp((char *) key, e + 1, len);
}

static int compare_rec_B(const void *key, const void *elem) {
   const char *e;
   int len;

   e = elem;
   len = e[1];  /* e[0] is kind */

   return strncmp((char *) key, e + 2, len);
}

static inline void dberr(const char *reason) {
   safe_vprintf(
      Cbred "<DB error>" Creset "%s\n",
      reason
   );
   exit(EXIT_FAILURE);
}
