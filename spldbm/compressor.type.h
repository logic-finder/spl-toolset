#ifndef COMPRESSOR_TYPE_H
#define COMPRESSOR_TYPE_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "global.h"
#include "wrapper.h"

#define ARCHIVE_PREFIX ".archive"
#define RESTORE_PREFIX ".restore"

typedef void worker_t(FILE *src, FILE *dest);

static void work_template(
   const char *prefix,
   const char *srcname,
   worker_t *work,
   uint8_t af
);
static void write_metadata(FILE *src, FILE *dest, uint8_t af);
static worker_t rle, rrle;

#endif
