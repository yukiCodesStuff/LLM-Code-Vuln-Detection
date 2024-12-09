}

void
PySys_SetArgvEx(int argc, char **argv, int updatepath)
{
#if defined(HAVE_REALPATH)
    char fullpath[MAXPATHLEN];
#elif defined(MS_WINDOWS) && !defined(MS_WINCE)
        Py_FatalError("no mem for sys.argv");
    if (PySys_SetObject("argv", av) != 0)
        Py_FatalError("can't assign sys.argv");
    if (updatepath && path != NULL) {
        char *argv0 = argv[0];
        char *p = NULL;
        Py_ssize_t n = 0;
        PyObject *a;
    Py_DECREF(av);
}

void
PySys_SetArgv(int argc, char **argv)
{
    PySys_SetArgvEx(argc, argv, 1);
}


/* APIs to write to sys.stdout or sys.stderr using a printf-like interface.
   Adapted from code submitted by Just van Rossum.
