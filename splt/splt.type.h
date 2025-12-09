#ifndef SPLT_H
#define SPLT_H

#include <ctype.h>
#include <string.h>
#include "msg.h"
#include "lex.h"
#include "parse.h"
#include "global.h"
#include "wrapper.h"
#include "loadfile.h"
#include "argparse.h"
#include "colorcode.h"
#include "optprocessor.h"

static int count_tree_node(tree_t *root);
static tree_callback_t cleanup_node;

/* Miscellaneous */
static arr_iterator_t print_token;
static tree_callback_t print_node;

#endif
