#ifndef LEX_TYPE_H
#define LEX_TYPE_H

#include <ctype.h>
#include <stdarg.h>
#include <setjmp.h>
#include "msg.h"
#include "global.h"
#include "strutil.h"
#include "wrapper.h"
#include "colorcode.h"

/**********
 * MACROS *
 **********/
#define LONGJMP_ENV env_lex
#define JUMP(v) LONGJMP_TEMPLET(LONGJMP_ENV, (v))

/************
 * TYPEDEFS *
 ************/
typedef int processor_t(va_list *ap);
typedef int checker_t(va_list *ap);

/***********************
 * FUNCTION PROTOTYPES *
 ***********************/
/* Character Handling */
static void skip_space(void);
static void skip_nchar(int n);
static void read_token(void);
static void read_nchar(int n);

/* Utils */
static inline void save_state(void);
static void store_token(arr_t *toks);
static void store_punct(arr_t *toks);

/* Miscellaneous */
static inline void iterate_lines(processor_t *process, ...);
static processor_t process_skip;
static processor_t process_read;
static checker_t check_space;
static checker_t check_cntlessthan;
static checker_t check_token;
static void store_string(arr_t *toks, tokkind_t kind);

#endif
