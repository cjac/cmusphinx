/*
 * Procedure to "unlimit" the memory usage of your program.
 * The limit is set to the maximum of your system.
 * It returns the limit.
 */

#if ((! WIN32) && (! _HPUX_SOURCE))
#include <sys/time.h>
#include <sys/resource.h>
#endif

void unlimit ( void )
{
#if ((! WIN32) && (! _HPUX_SOURCE))
  struct rlimit rl;

  getrlimit(RLIMIT_DATA,&rl);
  rl.rlim_cur = rl.rlim_max;
  setrlimit(RLIMIT_DATA,&rl);
#endif
}
