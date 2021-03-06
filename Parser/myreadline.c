
/* Readline interface for tokenizer.c and [raw_]input() in bltinmodule.c.
   By default, or when stdin is not a tty device, we have a super
   simple my_readline function using fgets.
   Optionally, we can use the GNU readline library.
   my_readline() has a different return value from GNU readline():
   - NULL if an interrupt occurred or if an error occurred
   - a malloc'ed empty string if EOF was read
   - a malloc'ed string ending in \n normally
*/

#include "Python.h"
#include "pycore_pystate.h"   // PyThreadState_GET()


PyThreadState* _PyOS_ReadlineTState = NULL;

int (*PyOS_InputHook)(void) = NULL;

/* This function restarts a fgets() after an EINTR error occurred
   except if _PyOS_InterruptOccurred() returns true. */

static int
my_fgets(PyThreadState* tstate, char *buf, int len, FILE *fp)
{
    while (1) {
        if (PyOS_InputHook != NULL) {
            (void)(PyOS_InputHook)();
        }

        errno = 0;
        clearerr(fp);
        char *p = fgets(buf, len, fp);
        if (p != NULL) {
            return 0; /* No error */
        }
        int err = errno;
        if (feof(fp)) {
            clearerr(fp);
            return -1; /* EOF */
        }

#ifdef EINTR
        if (err == EINTR) {
	    int s = 0;  // PyErr_CheckSignals();
            if (s < 0) {
                return 1;
            }
            /* try again */
            continue;
        }
#endif

	//        if (_PyOS_InterruptOccurred(tstate)) {
	//            return 1; /* Interrupt */
	//        }
        return -2; /* Error */
    }
    /* NOTREACHED */
}

/* Readline implementation using fgets() */

char *
PyOS_StdioReadline(FILE *sys_stdin, FILE *sys_stdout, const char *prompt)
{
    size_t n;
    char *p, *pr;
    PyThreadState *tstate = _PyOS_ReadlineTState;
    assert(tstate != NULL);

    n = 100;
    p = (char *)PyMem_Malloc(n);
    if (p == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    fflush(sys_stdout);
    if (prompt) {
        fprintf(stderr, "%s", prompt);
    }
    fflush(stderr);

    switch (my_fgets(tstate, p, (int)n, sys_stdin)) {
    case 0: /* Normal case */
        break;
    case 1: /* Interrupt */
        PyMem_Free(p);
        return NULL;
    case -1: /* EOF */
    case -2: /* Error */
    default: /* Shouldn't happen */
        *p = '\0';
        break;
    }

    n = strlen(p);
    while (n > 0 && p[n-1] != '\n') {
        size_t incr = n+2;
        if (incr > INT_MAX) {
            PyMem_Free(p);
            PyErr_SetString(PyExc_OverflowError, "input line too long");
            return NULL;
        }

        pr = (char *)PyMem_Realloc(p, n + incr);
        if (pr == NULL) {
            PyMem_Free(p);
            PyErr_NoMemory();
            return NULL;
        }
        p = pr;

        if (my_fgets(tstate, p+n, (int)incr, sys_stdin) != 0) {
            break;
        }
        n += strlen(p+n);
    }

    pr = (char *)PyMem_Realloc(p, n+1);
    if (pr == NULL) {
        PyMem_Free(p);
        PyErr_NoMemory();
        return NULL;
    }
    return pr;
}


/* By initializing this function pointer, systems embedding Python can
   override the readline function.

   Note: Python expects in return a buffer allocated with PyMem_Malloc. */

char *(*PyOS_ReadlineFunctionPointer)(FILE *, FILE *, const char *) = NULL;


/* Interface used by tokenizer.c and bltinmodule.c */

char *
PyOS_Readline(FILE *sys_stdin, FILE *sys_stdout, const char *prompt)
{
    char *rv, *res;
    size_t len;

    PyThreadState *tstate = PyThreadState_Get();
    if (_PyOS_ReadlineTState == tstate) {
        PyErr_SetString(PyExc_RuntimeError,
                        "can't re-enter readline");
        return NULL;
    }


    if (PyOS_ReadlineFunctionPointer == NULL) {
        PyOS_ReadlineFunctionPointer = PyOS_StdioReadline;
    }

    _PyOS_ReadlineTState = tstate;

    /* This is needed to handle the unlikely case that the
     * interpreter is in interactive mode *and* stdin/out are not
     * a tty.  This can happen, for example if python is run like
     * this: python -i < test1.py
     */
    if (!isatty (fileno (sys_stdin)) || !isatty (fileno (sys_stdout)))
        rv = PyOS_StdioReadline (sys_stdin, sys_stdout, prompt);
    else
        rv = (*PyOS_ReadlineFunctionPointer)(sys_stdin, sys_stdout,
                                             prompt);

    _PyOS_ReadlineTState = NULL;

    if (rv == NULL)
        return NULL;

    len = strlen(rv) + 1;
    res = PyMem_Malloc(len);
    if (res != NULL) {
        memcpy(res, rv, len);
    }
    else {
        PyErr_NoMemory();
    }
    PyMem_Free(rv);

    return res;
}
