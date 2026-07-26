#ifndef COMPRESSOR_INTERNALS_H
#define COMPRESSOR_INTERNALS_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "global.h"
#include "wrappers.h"
#include "db.common.h"

#define ARCHIVE_PREFIX ".archive"
#define RESTORE_PREFIX ".restore"

typedef void worker_t(FILE *src, FILE *dest);

static void work_template(
   const char *prefix,
   const char *srcname,
   worker_t *work,
   uint8_t af
);
static bool is_archived(FILE *src);
static void write_metadata(FILE *src, FILE *dest, uint8_t af);
static worker_t rle, rrle;

#endif
