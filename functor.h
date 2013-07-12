/* functors for streaming data through */
#ifndef TJF_FUNCTOR_H
#define TJF_FUNCTOR_H

#include <stdbool.h>
#include <stddef.h>

typedef void (func_process)(void* self, const void* data, const size_t nelems,
                            const bool is_signed, const size_t bpc);
typedef void (func_finished)(void* self);

struct processor {
  func_process* run;
  func_finished* fini;
};

#endif /* TJF_FUNCTOR_H */
