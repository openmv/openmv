#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include <stdint.h>

#ifndef UNUSED
#define UNUSED(x) (void)x
#endif

#ifndef STDOUT_USART
#define STDOUT_USART 2
#endif

#ifndef STDERR_USART
#define STDERR_USART 2
#endif

#ifndef STDIN_USART
#define STDIN_USART 2
#endif

#undef errno
extern int errno;

/*
 * A pointer to a list of environment variables and their values.
 * For a minimal environment, this empty list is adequate.
 */
char *__env[1] = { 0 };
char **environ = __env;

int _write(int file, char *ptr, int len);

void _exit(int status) {
	UNUSED(status);

	_write(1, "exit", 4);
	while (1) {
		;
	}
}

int _close(int file) {
	UNUSED(file);

	return -1;
}

/*
 * Transfer control to a new process. Minimal implementation (for a system
 * without processes)
 */
int _execve(char *name, char **argv, char **env) {
	UNUSED(name);
	UNUSED(argv);
	UNUSED(env);

	errno = ENOMEM;
	return -1;
}

/*
 * Create a new process. Minimal implementation (for a system without processes)
 */
int _fork() {
	errno = EAGAIN;
	return -1;
}
/*
 * Status of an open file. Minimal implementation.
 */
int _fstat(int file, struct stat *st) {
	UNUSED(file);

	st->st_mode = S_IFCHR;
	return 0;
}

/*
 * Process-ID; this is sometimes used to generate strings unlikely to conflict
 * with other processes. Minimal implementation, for a system without processes.
 */
int _getpid() {
	return 1;
}

/*
 * Query whether output stream is a terminal. For consistency with the other
 * minimal implementation.
 */
int _isatty(int file) {
	UNUSED(file);

	switch (file) {
	case STDOUT_FILENO:
	case STDERR_FILENO:
	case STDIN_FILENO:
		return 1;
	default:
		//errno = ENOTTY;
		errno = EBADF;
		return 0;
	}
}

/*
 * Send a signal. Minimal implementation.
 */
int _kill(int pid, int sig) {
	UNUSED(pid);
	UNUSED(sig);

	errno = EINVAL;
	return (-1);
}

/*
 * Establish a new name for an existing file. Minimal implementation.
 */
int _link(char *old, char *new) {
	UNUSED(old);
	UNUSED(new);

	errno = EMLINK;
	return -1;
}

/*
 * Set position in a file. Minimal implementation.
 */
int _lseek(int file, int ptr, int dir) {
	UNUSED(file);
	UNUSED(ptr);
	UNUSED(dir);

	return 0;
}

/*
 * Increase program data space. Malloc and related functions depend on this.
 */
caddr_t _sbrk(int incr) {
	extern char _ebss; // Defined by the linker
	extern char _estack; // Defined by the linker
	static char *heap_end;
	char *prev_heap_end;

	if (heap_end == 0) {
		heap_end = &_ebss;
	}
	prev_heap_end = heap_end;

	if (heap_end + incr > &_estack){
		_write (STDERR_FILENO, "Heap and stack collision\n", 25);
		errno = ENOMEM;
		return  (caddr_t) - 1;
		//abort ();
	}

	heap_end += incr;
	return (caddr_t) prev_heap_end;

}

/*
 * Read a character to a file. `libc' subroutines will use this system routine
 * for input from all files, including stdin.
 * Returns -1 on error or blocks until the number of characters have been read.
 */
int _read(int file, char *ptr, int len) {
//	int n;
	int num = 0;
#if 0
	switch (file) {
	case STDIN_FILENO:
		for (n = 0; n < len; n++) {
#if   STDIN_USART == 1
			while ((USART1->ISR & USART_FLAG_RXNE) == (uint16_t)RESET) {}
			char c = (char)(USART1->TDR & (uint16_t)0x01FF);
#elif STDIN_USART == 2
			while ((USART2->ISR & USART_FLAG_RXNE) == (uint16_t) RESET) {}
			char c = (char) (USART2->TDR & (uint16_t) 0x01FF);
#elif STDIN_USART == 3
			while ((USART3->ISR & USART_FLAG_RXNE) == (uint16_t)RESET) {}
			char c = (char)(USART3->TDR & (uint16_t)0x01FF);
#endif
			*ptr++ = c;
			num++;
		}
		break;
	default:
		errno = EBADF;
		return -1;
	}
#endif
	return num;
}

/*
 * Status of a file (by name). Minimal implementation.
 * int _EXFUN(stat,( const char *__path, struct stat *__sbuf ));
 */
int _stat(const char *filepath, struct stat *st) {
	UNUSED(filepath);
	st->st_mode = S_IFCHR;
	return 0;
}

/*
 * Timing information for current process. Minimal implementation.
 */
clock_t _times(struct tms *buf) {
	UNUSED(buf);
	return -1;
}

/*
 * Remove a file's directory entry. Minimal implementation.
 */
int _unlink(char *name) {
	UNUSED(name);
	errno = ENOENT;
	return -1;
}

/*
 * Wait for a child process. Minimal implementation.
 */
int _wait(int *status) {
	UNUSED(status);
	errno = ECHILD;
	return -1;
}

/*
 * Write a character to a file. `libc' subroutines will use this system routine
 * for output to all files, including stdout.
 * Returns -1 on error or number of bytes sent.
 */
int _write(int file, char *ptr, int len) {
#if 0
 	int n;
	switch (file) {
	case STDOUT_FILENO: /*stdout*/
		for (n = 0; n < len; n++) {
#if STDOUT_USART == 1
			while ((USART1->ISR & USART_FLAG_TXE) == (uint16_t)RESET) {}
			USART1->TDR = (*ptr++ & (uint16_t)0x01FF);
#elif  STDOUT_USART == 2
			while ((USART2->ISR & USART_FLAG_TXE) == (uint16_t) RESET) {
			}
			USART2->TDR = (*ptr++ & (uint16_t) 0x01FF);
#elif  STDOUT_USART == 3
			while ((USART3->ISR & USART_FLAG_TXE) == (uint16_t)RESET) {}
			USART3->TDR = (*ptr++ & (uint16_t)0x01FF);
#endif
		}
		break;
	case STDERR_FILENO: /* stderr */
		for (n = 0; n < len; n++) {
#if STDERR_USART == 1
			while ((USART1->ISR & USART_FLAG_TXE) == (uint16_t)RESET) {}
			USART1->TDR = (*ptr++ & (uint16_t)0x01FF);
#elif  STDERR_USART == 2
			while ((USART2->ISR & USART_FLAG_TXE) == (uint16_t) RESET) {
			}
			USART2->TDR = (*ptr++ & (uint16_t) 0x01FF);
#elif  STDERR_USART == 3
			while ((USART3->ISR & USART_FLAG_TXE) == (uint16_t)RESET) {}
			USART3->TDR = (*ptr++ & (uint16_t)0x01FF);
#endif
		}
		break;
	default:
		errno = EBADF;
		return -1;
	}
#endif
	return len;
}
