#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "splc.h"
#include "global.h"

#define OBJFILE_FI 0x53504C4FUL

#define OBJFILE_HD_FI 4
#define OBJFILE_HD_DF 1
#define OBJFILE_HD_SP 4
#define OBJFILE_HDSIZ (OBJFILE_HD_FI + OBJFILE_HD_DF + 4 * OBJFILE_HD_SP)

#define OBJFILE_DI_EC 4
#define OBJFILE_DI_ET_OP SPL_INT_SIZ
#define OBJFILE_DI_ET_SL 4
#define OBJFILE_DI_ET_SP 4
#define OBJFILE_DI_ETSIZ (OBJFILE_DI_ET_OP + OBJFILE_DI_ET_SL + OBJFILE_DI_ET_SP)

void assemble(compile_ctx_t *cctx);

#endif
