//! Set or get the parent-death signal number of the calling process

#![cfg_attr(docsrs, feature(auto_doc_cfg, doc_cfg))]

use std::sync::OnceLock;

use arrayvec::ArrayVec;
use either::Either;
use pyo3::exceptions::{PyOSError, PyValueError};
use pyo3::prelude::*;
use rustix::process::{parent_process_death_signal, set_parent_process_death_signal, Signal};

/// A Python module implemented in Rust.
#[pymodule(name = "_pdeathsignal")]
fn pdeathsignal(m: &Bound<'_, PyModule>) -> PyResult<()> {
    m.add_class::<WrappedSignal>()?;
    m.add_function(wrap_pyfunction!(get, m)?)?;
    m.add_function(wrap_pyfunction!(set, m)?)?;
    Ok(())
}

/// A signal number
#[pyclass(frozen, freelist = 32)]
#[pyo3(name = "Signal")]
#[derive(Debug, Clone, Copy)]
struct WrappedSignal(Signal);

#[pymethods]
impl WrappedSignal {
    #[classattr]
    #[pyo3(name = "SIGHUP")]
    fn sighup(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Hup)
    }

    #[classattr]
    #[pyo3(name = "SIGINT")]
    fn sigint(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Int)
    }

    #[classattr]
    #[pyo3(name = "SIGQUIT")]
    fn sigquit(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Quit)
    }

    #[classattr]
    #[pyo3(name = "SIGILL")]
    fn sigill(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Ill)
    }

    #[classattr]
    #[pyo3(name = "SIGTRAP")]
    fn sigtrap(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Trap)
    }

    #[classattr]
    #[pyo3(name = "SIGABRT")]
    fn sigabrt(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Abort)
    }

    #[classattr]
    #[pyo3(name = "SIGBUS")]
    fn sigbus(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Bus)
    }

    #[classattr]
    #[pyo3(name = "SIGFPE")]
    fn sigfpe(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Fpe)
    }

    #[classattr]
    #[pyo3(name = "SIGKILL")]
    fn sigkill(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Kill)
    }

    #[classattr]
    #[pyo3(name = "SIGUSR1")]
    fn sigusr1(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Usr1)
    }

    #[classattr]
    #[pyo3(name = "SIGSEGV")]
    fn sigsegv(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Segv)
    }

    #[classattr]
    #[pyo3(name = "SIGUSR2")]
    fn sigusr2(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Usr2)
    }

    #[classattr]
    #[pyo3(name = "SIGPIPE")]
    fn sigpipe(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Pipe)
    }

    #[classattr]
    #[pyo3(name = "SIGALRM")]
    fn sigalrm(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Alarm)
    }

    #[classattr]
    #[pyo3(name = "SIGTERM")]
    fn sigterm(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Term)
    }

    #[classattr]
    #[pyo3(name = "SIGSTKFLT")]
    fn sigstkflt(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Stkflt)
    }

    #[classattr]
    #[pyo3(name = "SIGCHLD")]
    fn sigchld(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Child)
    }

    #[classattr]
    #[pyo3(name = "SIGCONT")]
    fn sigcont(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Cont)
    }

    #[classattr]
    #[pyo3(name = "SIGSTOP")]
    fn sigstop(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Stop)
    }

    #[classattr]
    #[pyo3(name = "SIGTSTP")]
    fn sigtstp(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Tstp)
    }

    #[classattr]
    #[pyo3(name = "SIGTTIN")]
    fn sigttin(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Ttin)
    }

    #[classattr]
    #[pyo3(name = "SIGTTOU")]
    fn sigttou(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Ttou)
    }

    #[classattr]
    #[pyo3(name = "SIGURG")]
    fn sigurg(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Urg)
    }

    #[classattr]
    #[pyo3(name = "SIGXCPU")]
    fn sigxcpu(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Xcpu)
    }

    #[classattr]
    #[pyo3(name = "SIGXFSZ")]
    fn sigxfsz(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Xfsz)
    }

    #[classattr]
    #[pyo3(name = "SIGVTALRM")]
    fn sigvtalrm(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Vtalarm)
    }

    #[classattr]
    #[pyo3(name = "SIGPROF")]
    fn sigprof(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Prof)
    }

    #[classattr]
    #[pyo3(name = "SIGWINCH")]
    fn sigwinch(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Winch)
    }

    #[classattr]
    #[pyo3(name = "SIGIO")]
    fn sigio(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Io)
    }

    #[classattr]
    #[pyo3(name = "SIGPWR")]
    fn sigpwr(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Power)
    }

    #[classattr]
    #[pyo3(name = "SIGSYS")]
    fn sigsys(py: Python<'_>) -> PyResult<Py<Self>> {
        Self::from_signal(py, Signal::Sys)
    }

    fn __str__(&self) -> &'static str {
        match self.0 {
            Signal::Hup => "SIGHUP",
            Signal::Int => "SIGINT",
            Signal::Quit => "SIGQUIT",
            Signal::Ill => "SIGILL",
            Signal::Trap => "SIGTRAP",
            Signal::Abort => "SIGABRT",
            Signal::Bus => "SIGBUS",
            Signal::Fpe => "SIGFPE",
            Signal::Kill => "SIGKILL",
            Signal::Usr1 => "SIGUSR1",
            Signal::Segv => "SIGSEGV",
            Signal::Usr2 => "SIGUSR2",
            Signal::Pipe => "SIGPIPE",
            Signal::Alarm => "SIGALRM",
            Signal::Term => "SIGTERM",
            Signal::Stkflt => "SIGSTKFLT",
            Signal::Child => "SIGCHLD",
            Signal::Cont => "SIGCONT",
            Signal::Stop => "SIGSTOP",
            Signal::Tstp => "SIGTSTP",
            Signal::Ttin => "SIGTTIN",
            Signal::Ttou => "SIGTTOU",
            Signal::Urg => "SIGURG",
            Signal::Xcpu => "SIGXCPU",
            Signal::Xfsz => "SIGXFSZ",
            Signal::Vtalarm => "SIGVTALRM",
            Signal::Prof => "SIGPROF",
            Signal::Winch => "SIGWINCH",
            Signal::Io => "SIGIO",
            Signal::Power => "SIGPWR",
            Signal::Sys => "SIGSYS",
        }
    }

    fn __repr__(&self) -> &'static str {
        match self.0 {
            Signal::Hup => "pdeathsignal.Signal.SIGHUP",
            Signal::Int => "pdeathsignal.Signal.SIGINT",
            Signal::Quit => "pdeathsignal.Signal.SIGQUIT",
            Signal::Ill => "pdeathsignal.Signal.SIGILL",
            Signal::Trap => "pdeathsignal.Signal.SIGTRAP",
            Signal::Abort => "pdeathsignal.Signal.SIGABRT",
            Signal::Bus => "pdeathsignal.Signal.SIGBUS",
            Signal::Fpe => "pdeathsignal.Signal.SIGFPE",
            Signal::Kill => "pdeathsignal.Signal.SIGKILL",
            Signal::Usr1 => "pdeathsignal.Signal.SIGUSR1",
            Signal::Segv => "pdeathsignal.Signal.SIGSEGV",
            Signal::Usr2 => "pdeathsignal.Signal.SIGUSR2",
            Signal::Pipe => "pdeathsignal.Signal.SIGPIPE",
            Signal::Alarm => "pdeathsignal.Signal.SIGALRM",
            Signal::Term => "pdeathsignal.Signal.SIGTERM",
            Signal::Stkflt => "pdeathsignal.Signal.SIGSTKFLT",
            Signal::Child => "pdeathsignal.Signal.SIGCHLD",
            Signal::Cont => "pdeathsignal.Signal.SIGCONT",
            Signal::Stop => "pdeathsignal.Signal.SIGSTOP",
            Signal::Tstp => "pdeathsignal.Signal.SIGTSTP",
            Signal::Ttin => "pdeathsignal.Signal.SIGTTIN",
            Signal::Ttou => "pdeathsignal.Signal.SIGTTOU",
            Signal::Urg => "pdeathsignal.Signal.SIGURG",
            Signal::Xcpu => "pdeathsignal.Signal.SIGXCPU",
            Signal::Xfsz => "pdeathsignal.Signal.SIGXFSZ",
            Signal::Vtalarm => "pdeathsignal.Signal.SIGVTALRM",
            Signal::Prof => "pdeathsignal.Signal.SIGPROF",
            Signal::Winch => "pdeathsignal.Signal.SIGWINCH",
            Signal::Io => "pdeathsignal.Signal.SIGIO",
            Signal::Power => "pdeathsignal.Signal.SIGPWR",
            Signal::Sys => "pdeathsignal.Signal.SIGSYS",
        }
    }

    fn __index__(&self) -> i32 {
        self.0 as i32
    }

    fn __int__(&self) -> i32 {
        self.0 as i32
    }

    fn __pos__(&self) -> i32 {
        self.0 as i32
    }

    fn __neg__(&self) -> i32 {
        -(self.0 as i32)
    }

    #[new]
    fn __new__(
        value: Either<Py<WrappedSignal>, i32>,
        py: Python<'_>,
    ) -> PyResult<Py<WrappedSignal>> {
        let signal = match value {
            Either::Left(value) => return Ok(value),
            Either::Right(signal) => signal,
        };
        match Signal::from_raw(signal) {
            Some(signal) => WrappedSignal::from_signal(py, signal),
            None => Err(PyValueError::new_err((format!(
                "Illegal signal number {signal}"
            ),))),
        }
    }

    #[staticmethod]
    fn get(py: Python<'_>) -> PyResult<Option<Py<WrappedSignal>>> {
        do_get(py)
    }

    fn set(&self) -> PyResult<()> {
        do_set(Some(self.0))
    }
}

/// Get the parent-death signal number of the calling process
///
/// C.f. <https://www.man7.org/linux/man-pages//man2/PR_SET_PDEATHSIG.2const.html>
#[pyfunction]
#[pyo3(name = "get")]
fn get(py: Python<'_>) -> PyResult<Option<Py<WrappedSignal>>> {
    do_get(py)
}

/// Set the parent-death signal number of the calling process
///
/// C.f. <https://www.man7.org/linux/man-pages/man2/PR_GET_PDEATHSIG.2const.html>
#[pyfunction]
#[pyo3(name = "set", signature = (signal, /))]
fn set(signal: Option<Either<WrappedSignal, i32>>) -> PyResult<()> {
    do_set(match signal {
        None | Some(Either::Right(0)) => None,
        Some(Either::Left(WrappedSignal(signal))) => Some(signal),
        Some(Either::Right(signal)) => match Signal::from_raw(signal) {
            Some(signal) => Some(signal),
            None => {
                return Err(PyValueError::new_err((format!(
                    "Illegal signal number {signal}"
                ),)));
            },
        },
    })
}

fn do_get(py: Python<'_>) -> PyResult<Option<Py<WrappedSignal>>> {
    match parent_process_death_signal() {
        Ok(Some(signal)) => Ok(Some(WrappedSignal::from_signal(py, signal)?)),
        Ok(None) => Ok(None),
        Err(err) => Err(PyOSError::new_err((err.raw_os_error(), err.to_string()))),
    }
}

fn do_set(signal: Option<Signal>) -> PyResult<()> {
    set_parent_process_death_signal(signal)
        .map_err(|err| PyOSError::new_err((err.raw_os_error(), err.to_string())))
}

impl WrappedSignal {
    fn from_signal(py: Python<'_>, signal: Signal) -> PyResult<Py<Self>> {
        static SIGNALS: OnceLock<PyResult<ArrayVec<Py<WrappedSignal>, SIGNAL_COUNT>>> =
            OnceLock::new();
        match SIGNALS.get_or_init(|| make_signals(py)) {
            Ok(signals) => Ok(signals[signal as i32 as usize].clone_ref(py)),
            Err(err) => Err(err.clone_ref(py)),
        }
    }
}

#[cold]
fn make_signals(py: Python<'_>) -> Result<ArrayVec<Py<WrappedSignal>, SIGNAL_COUNT>, PyErr> {
    (0..SIGNAL_COUNT)
        .map(|signal| Signal::from_raw(signal as i32))
        .map(|signal| signal.unwrap_or(Signal::Hup))
        .map(|signal| Py::new(py, WrappedSignal(signal)))
        .collect::<PyResult<ArrayVec<_, SIGNAL_COUNT>>>()
}

const SIGNAL_COUNT: usize = 32;
