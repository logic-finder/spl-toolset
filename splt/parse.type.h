#ifndef PARSE_TYPE_H
#define PARSE_TYPE_H

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "global.h"
#include "strutil.h"
#include "wrapper.h"
#include "lineutil.h"

#define RET_EOT  0  // end of statement
#define RET_EOS  1  // end of scene
#define RET_EOA  2  // end of act
#define RET_EOE  3  // end of everything (finale)

typedef struct stmt {
   int (*seeker)(line_t *lines, int cnt);
   int (*parser)(line_t *lines, int cnt, tree_t *scene);
} stmt_t;

static void traverse(tree_t *t);  /* for debug */
static void parse_title(line_t *lines, int cnt, tree_t *pt);
static void parse_dp(line_t *lines, int cnt, tree_t *pt);
static tree_t *parse_act(line_t *lines, int cnt, tree_t *pt);
static tree_t *parse_scene(line_t *lines, int cnt, tree_t *act);
static int parse_stmt(line_t *lines, int cnt, tree_t *scene);

#endif
