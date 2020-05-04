import pyb, micropython, array, uctypes
micropython.alloc_emergency_exception_buf(100)

class MutexException(OSError):
    pass

class Mutex:
    @micropython.asm_thumb
    def _acquire(r0, r1):       # Spinlock: wait on the semaphore. Return on success.
        label(LOOP)
        ldr(r0, [r1, 0])        # Wait for lock to be zero
        cmp(r0, 0)
        bne(LOOP)               # Another process has the lock: spin on it
        cpsid(0)                # OK, we have lock  at this instant disable interrupts
        ldr(r0, [r1, 0])        # and re-check in case an interrupt occurred
        cmp(r0, 0)
        itt(ne)                 # if someone got in first re-enable ints
        cpsie(0)                # and start polling again
        b(LOOP)
        mov(r0, 1)              # We have an exclusive access
        str(r0, [r1, 0])        # set the lock
        cpsie(0)

    @micropython.asm_thumb
    def _attempt(r0, r1):       # Nonblocking. Try to lock. Return 0 on success, 1 on fail
        cpsid(0)                # disable interrupts
        ldr(r0, [r1, 0])
        cmp(r0, 0)
        bne(FAIL)               # Another process has the lock: fail
        mov(r2, 1)              # No lock
        str(r2, [r1, 0])        # set the lock
        label(FAIL)
        cpsie(0)                # enable interrupts

    def __init__(self):
        self.lock = array.array('i', (0,)) # 1 if a process has the lock else 0

# POSIX API pthread_mutex_lock() blocks the thread till resource is available.
    def __enter__(self):
        self._acquire(uctypes.addressof(self.lock))
        return self

    def __exit__(self, *_):
        self.lock[0] = 0

# POSIX pthread_mutex_unlock()
    def release(self):
        if self.lock[0] == 0:
            raise MutexException('Semaphore already released')
        self.lock[0] = 0

# POSIX pthread_mutex_trylock() API. When mutex is not available the function returns immediately
    def test(self):          # Nonblocking: try to acquire, return True if success.
        return self._attempt(uctypes.addressof(self.lock)) == 0

