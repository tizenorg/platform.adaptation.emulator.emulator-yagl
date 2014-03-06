#ifndef _YAGL_EXPORT_H_
#define _YAGL_EXPORT_H_

#include "yagl_types.h"

#define YAGL_API __attribute__ ((visibility("default")))

#define YAGL_ALIAS(name, aliasname) \
  extern __typeof (name) aliasname __attribute__ ((alias (#name)))

#endif
