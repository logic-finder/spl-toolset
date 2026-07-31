#ifndef STRUTILS_INTERNALS_H
#define STRUTILS_INTERNALS_H

#include <ctype.h>
#include <stddef.h>
#include "wrappers.h"

// fixme: 이 변수 쓰는곳이 여기밖에 없는데 전역으로 돌릴이유 없을듯
// .h에 있는 extern 도 제거
const char *whitespaces = " \n\r\t\a\b\v\f";

#endif
