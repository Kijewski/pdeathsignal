"""Set or get the parent-death signal number of the calling process"""

class Signal:
    """A signal number"""

    def __init__(self, signal: Signal | int) -> Signal:
        """Convert an integer to a Signal"""
    SIGHUP: Signal = ...
    SIGINT: Signal = ...
    SIGQUIT: Signal = ...
    SIGILL: Signal = ...
    SIGTRAP: Signal = ...
    SIGABRT: Signal = ...
    SIGBUS: Signal = ...
    SIGFPE: Signal = ...
    SIGKILL: Signal = ...
    SIGUSR1: Signal = ...
    SIGSEGV: Signal = ...
    SIGUSR2: Signal = ...
    SIGPIPE: Signal = ...
    SIGALRM: Signal = ...
    SIGTERM: Signal = ...
    SIGSTKFLT: Signal = ...
    SIGCHLD: Signal = ...
    SIGCONT: Signal = ...
    SIGSTOP: Signal = ...
    SIGTSTP: Signal = ...
    SIGTTIN: Signal = ...
    SIGTTOU: Signal = ...
    SIGURG: Signal = ...
    SIGXCPU: Signal = ...
    SIGXFSZ: Signal = ...
    SIGVTALRM: Signal = ...
    SIGPROF: Signal = ...
    SIGWINCH: Signal = ...
    SIGIO: Signal = ...
    SIGPWR: Signal = ...
    SIGSYS: Signal = ...

    def set(self):
        """Set the parent-death signal number of the calling process"""

    @staticmethod
    def get() -> Signal | None:
        """Get the parent-death signal number of the calling process"""

def set(signal: Signal | int):
    """Set the parent-death signal number of the calling process"""

def get() -> Signal | None:
    """Get the parent-death signal number of the calling process"""
