#ifndef OBJ_MACRO_H
#define OBJ_MACRO_H

#define OBJ_HD_METADATA 0x53504C4F
#define OBJ_HD_SRCFSECT 0x53524346
#define OBJ_HD_CODESECT 0x434F4445
#
#define OBJ_SECTNUM 2
#
#define OBJ_MTDT_HD 4
#define OBJ_MTDT_SP 4
#define OBJ_MTDT_SIZ (OBJ_MTDT_HD + OBJ_MTDT_SP * OBJ_SECTNUM)
#
#define OBJ_SRCF_HD 4
#define OBJ_SRCF_LN_L 1

typedef enum obj_sectkind {
   OBJ_SECTKIND_SRCF,
   OBJ_SECTKIND_CODE
} obj_sectkind_t;

#endif
