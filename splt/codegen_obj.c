#include "gen_obj.type.h"

extern arr_t *ls;  // see global.h
static FILE *fp;
static bool le, be;
static uint32_t sectsizs[OBJ_SECTNUM];

void gen_obj(void) {
   /*
    * OBJECT FILE STRUCTURE
    *    Metadata Section     : total 12 bytes
    *       header            - 4 bytes; BE; contains "SPLO"
    *       src. sect. pos.   - 4 bytes; LE
    *       code sect. pos.   - 4 bytes; LE
    *
    *    Source File Section  : size depends on lines
    *       header            - 4 bytes; BE; contains "SRCF"
    *       line count        - 4 bytes; LE
    *       line...
    *          length         - 1 byte
    *          string         - length byte
    *
    *    Code Section         : size depends on codes
    *       header            - 4 bytes; BE; contains "CODE"
    *       dpsz              - 1 byte
    *       code...
    */

   // 코드 생성
   // 메타데이터 작성

   fp = sfopen(FP_NAME, "wb");
   le = isle(), be = !le;

   sfseek(fp, OBJ_MTDT_SIZ, SEEK_SET);
   write_srcfile(fp);
   write_code(fp);
}

static void write_srcfile(FILE *fp) {
   uint32_t header;
   int lines_siz;
   uint8_t line_len;
   line_t *line;
   uint32_t sectsiz;

   header = OBJ_HD_SRCFSECT;
   lines_siz = arr_size(ls);

   if (le) header = endrev32(header);
   sfwrite(&header, 1, OBJ_SRCF_HD, fp);

   sectsiz = 0;
   for (int i = 0; i < lines_siz; i++) {
      line = arr_peek(ls, i);
      line_len = line->len - 2;
      sfwrite(&line_len, OBJ_SRCF_LN_L, 1, fp);
      sfwrite(line->run, 1, line_len, fp);
      sectsiz += line_len;
   }
   sectsiz += lines_siz * OBJ_SRCF_LN_L;

   sectsiz += OBJ_SRCF_HD;
   sectsizs[OBJ_SECTKIND_SRCF] = sectsiz;
}

static void write_code(FILE *fp) {

}
