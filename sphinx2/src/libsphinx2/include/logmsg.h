/*
 * logmsg.h - verbosity-controlled internal log messages.
 *
 * For some reason, not everything used the err.h interface (maybe
 * because it was just too verbose by far?).  These are just simple
 * fprintf() wrappers that allow libraries to keep sphinx output from
 * interfering with their own.
 */

void log_debug(char const *fmt, ...);
void log_info(char const *fmt, ...);
void log_warn(char const *fmt, ...);
void log_error(char const *fmt, ...);
