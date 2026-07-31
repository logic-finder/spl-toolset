#ifndef LEX_INTERNALS_H
#define LEX_INTERNALS_H

#include <ctype.h>
#include <stdarg.h>
#include <setjmp.h>
#include "msg.h"
#include "global.h"
#include "strutils.h"
#include "wrappers.h"
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

/******************************
 * IMPORTANT GLOBAL VARIABLES *
 ******************************/
/* Line Access */

// extern arr_t *ls;    // array of line_t (see global.h)

static int lls;      // length of ls

static int p;        // line number
static int q;        // position in line
static line_t *l;    // l = arr_peek(ls, p)

static int tp;       // temp. var. for p
static int tq;       // temp. var. for q

/* Line Contents Copy */
static char ch;      // to store a char
static char *buf;    // to store a string
static int idx;      // position in buf
static int max;      // size of buf

/* Miscellaneous */
static jmp_buf LONGJMP_ENV;  // for setjmp & longjmp
static bool eoe;             // end-of-everything
const tokkind_t tokkind;     // kind of token

#endif
