#ifndef PARSE_TYPE_H
#define PARSE_TYPE_H

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include "msg.h"
#include "global.h"
#include "strutil.h"
#include "wrapper.h"
#include "lineutil.h"
#include "colorcode.h"

#define RET_EOT  0  // end of statement
#define RET_EOS  1  // end of scene
#define RET_EOA  2  // end of act
#define RET_EOE  3  // end of everything

typedef int seeker_t(void);
typedef int parser_t(tree_t *scene);
typedef void teller_t(void);
typedef int processor_t(va_list *apil);
typedef int checker_t(va_list *ap);

typedef seeker_t *seeker_pt;
typedef parser_t *parser_pt;
typedef teller_t *teller_pt;
typedef processor_t *processor_pt;
typedef checker_t *checker_pt;

typedef struct stmthandler {
   seeker_pt seek;
   parser_pt parse;
} stmthandler_t;

static char *roman_numerals = "IVXLCDM";
static char *connective = " and ";

/* Seekers */
static seeker_t seek_act;
static seeker_t seek_scene;
static seeker_t seek_enter;
static seeker_t seek_exit;
static seeker_t seek_exeunt;
static seeker_t seek_line;
static seeker_t seek_asgn_i;
static seeker_t seek_asgn_ii;
static seeker_t seek_out_i;
static seeker_t seek_out_ii;
static seeker_t seek_in_i;
static seeker_t seek_in_ii;
static seeker_t seek_goto;
static seeker_t seek_antec_i;
static seeker_t seek_antec_ii;
static seeker_t seek_antec_iii;
static seeker_t seek_antec_iv;
static seeker_t seek_conse_i;
static seeker_t seek_conse_ii;
static seeker_t seek_push;
static seeker_t seek_pop;

/* Parsers */
static void parse_title(tree_t *pt);
static void parse_dp(tree_t *pt);
static tree_t *parse_act(tree_t *pt);
static tree_t *parse_scene(tree_t *act);
static parser_t parse_stmt;
static parser_t parse_enter;
static parser_t parse_exit;
static parser_t parse_exeunt;
static parser_t parse_line;
static parser_t parse_asgn_i;
static parser_t parse_asgn_ii;
static parser_t parse_out_i;
static parser_t parse_out_ii;
static parser_t parse_in_i;
static parser_t parse_in_ii;
static parser_t parse_goto;
static parser_t parse_antec_i;
static parser_t parse_antec_ii;
static parser_t parse_antec_iii;
static parser_t parse_antec_iv;
static parser_t parse_conse_i;
static parser_t parse_conse_ii;
static parser_t parse_push;
static parser_t parse_pop;

// Utils
static inline void save_state(void);
static inline void load_state(void);

static void leave_nchar(int n);
static void leave_space(void);
static void leave_until(char sentinel);
static void store_nchar(int n);
static void store_until(char sentinel);
static void store_until_space(void);
static void store_roman(void);

// Error messages
static void synerr(teller_t tell);

static teller_t tell;
static teller_t tell_eoe;

// Miscellaneous
static int seek_actlike(const char *type);
static int seek_enterlike(const char *type);
static int seek_anteclike(const char *type);
static int seek_conselike(const char *type);
static int seek_token(const char *type);
static int seek_iolike_i(const char *type);
static int seek_iolike_ii(char sentinel);

static int parse_antec_common(tree_t *line);
static void parse_anteclike(const char *type);

static tree_t *interpret_namelist(
   tree_t *scene,
   char *stmt_tag,
   char *name_tag
);
static void interpret_constant(tree_t *line);

static void store_template(checker_t checker, ...);
static inline void iterate_lines(processor_t process, ...);
static processor_t process_leave;
static processor_t process_store;
static checker_t check_cntlessthan;
// static checker_t check_idxlessthan;
static checker_t check_chnotequalto;
static checker_t check_space;
static checker_t check_notspace;
static checker_t check_roman;

static void traverse(tree_t *base);

#endif
