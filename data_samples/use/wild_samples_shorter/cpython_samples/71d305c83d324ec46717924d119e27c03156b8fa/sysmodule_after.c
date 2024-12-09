#endif

void
PySys_SetArgvEx(int argc, wchar_t **argv, int updatepath)
{
#if defined(HAVE_REALPATH)
    wchar_t fullpath[MAXPATHLEN];
#elif defined(MS_WINDOWS) && !defined(MS_WINCE)
        Py_FatalError("no mem for sys.argv");
    if (PySys_SetObject("argv", av) != 0)
        Py_FatalError("can't assign sys.argv");
    if (updatepath && path != NULL) {
        wchar_t *argv0 = argv[0];
        wchar_t *p = NULL;
        Py_ssize_t n = 0;
        PyObject *a;
    Py_DECREF(av);
}

void
PySys_SetArgv(int argc, wchar_t **argv)
{
    PySys_SetArgvEx(argc, argv, 1);
}

/* Reimplementation of PyFile_WriteString() no calling indirectly
   PyErr_CheckSignals(): avoid the call to PyObject_Str(). */

static int