
#ifndef custom_malloc
#define custom_malloc( T, S, V ) {                                    \
	(V) = (T *)malloc(sizeof(T) * (S));                               \
	if (! (V)) {                                                      \
		u_printf("Memory allocation error in %s:%d, exiting...\n\n"); \
		exit(1);                                                      \
	}                                                                 \
}
#endif

#ifndef STRINGINT
#define STRINGINT(_string, _int) { \
  char *_tmp; \
  long _number = strtol (_string, &_tmp, 0); \
  errno = 0; \
  if ((errno != 0 && _number == 0) || _string == _tmp || \
      (errno == ERANGE && (_number == LONG_MAX || _number == LONG_MIN))) \
    { \
      u_fprintf (stderr,"`%s' out of range", _string);; \
      exit (EXIT_FAILURE); \
    } \
  else \
  _int = (int) _number; \
}
#endif

