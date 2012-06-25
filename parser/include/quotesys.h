/* quotesys.h -- declarations for quoting system arguments */

#include <stddef.h>

#ifdef __cplusplus
extern "C" {

#endif
#if defined __STDC__ || __GNUC__
# define __QUOTESYS_P(args) args
#else
# define __QUOTESYS_P(args) ()
#endif

size_t quote_system_arg __QUOTESYS_P ((char *, char const *));

#ifdef __cplusplus
}
#endif
