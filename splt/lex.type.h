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

typedef int processor_t(va_list *ap);
typedef int checker_t(va_list *ap);

static void skip_space(void);
static void skip_nchar(int n);
static void read_token(void);
static void read_nchar(int n);

static inline void iterate_lines(processor_t *process, ...);
static processor_t process_skip;
static processor_t process_read;
static checker_t check_space;
static checker_t check_cntlessthan;
static checker_t check_token;

static void lexerr(teller_t *tell);
static teller_t tell_eoe;
static bool eoe(void);

static inline void save_state(void);

static void store_token(list_t *toks);
static void store_punct(list_t *toks);
static void store_string(list_t *toks, const char *type);

#endif
