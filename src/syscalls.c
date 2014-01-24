#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/times.h>
/**
 * Override _sbrk.
 * uses the whole main RAM block (128KB) for heap.
 */
caddr_t _sbrk(int incr) {
	extern char _heap_start; // Defined by the linker
	extern char _heap_end; // Defined by the linker
	static char *heap_end;
	char *prev_heap_end;

	if (heap_end == 0) {
		heap_end = &_heap_start;
	}
	prev_heap_end = heap_end;

	if (heap_end + incr > &_heap_end) {
		errno = ENOMEM;
		return  (caddr_t) - 1;
	}

	heap_end += incr;
	return (caddr_t) prev_heap_end;
}
