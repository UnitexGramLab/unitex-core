
#ifndef custom_malloc
#define custom_malloc( T, S, V )                                      \
	(V) = (T *)malloc(sizeof(T) * (S));                               \
	if (! (V)) {                                                      \
		u_printf("Memory allocation error in %s:%d, exiting...\n\n"); \
		exit(1);                                                      \
	} else (void)0 
#endif

