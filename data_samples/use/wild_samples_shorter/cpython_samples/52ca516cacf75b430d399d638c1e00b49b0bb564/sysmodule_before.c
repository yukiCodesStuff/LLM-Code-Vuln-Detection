}

void
PySys_SetArgv(int argc, char **argv)
{
#if defined(HAVE_REALPATH)
    char fullpath[MAXPATHLEN];
#elif defined(MS_WINDOWS) && !defined(MS_WINCE)
        Py_FatalError("no mem for sys.argv");
    if (PySys_SetObject("argv", av) != 0)
        Py_FatalError("can't assign sys.argv");
    if (path != NULL) {
        char *argv0 = argv[0];
        char *p = NULL;
        Py_ssize_t n = 0;
        PyObject *a;
    Py_DECREF(av);
}


/* APIs to write to sys.stdout or sys.stderr using a printf-like interface.
   Adapted from code submitted by Just van Rossum.
