    if (r == (size_t)-1 || r >= PATH_MAX) {
        errno = EINVAL;
        return NULL;
    }
    return resolved_path;
}
#endif

void
PySys_SetArgvEx(int argc, wchar_t **argv, int updatepath)
{
#if defined(HAVE_REALPATH)
    wchar_t fullpath[MAXPATHLEN];
#elif defined(MS_WINDOWS) && !defined(MS_WINCE)
    wchar_t fullpath[MAX_PATH];
#endif
    PyObject *av = makeargvobject(argc, argv);
    PyObject *path = PySys_GetObject("path");
    if (av == NULL)
        Py_FatalError("no mem for sys.argv");
    if (PySys_SetObject("argv", av) != 0)
        Py_FatalError("can't assign sys.argv");
    if (updatepath && path != NULL) {
        wchar_t *argv0 = argv[0];
        wchar_t *p = NULL;
        Py_ssize_t n = 0;
        PyObject *a;
        extern int _Py_wreadlink(const wchar_t *, wchar_t *, size_t);
#ifdef HAVE_READLINK
        wchar_t link[MAXPATHLEN+1];
        wchar_t argv0copy[2*MAXPATHLEN+1];
        int nr = 0;
        if (argc > 0 && argv0 != NULL && wcscmp(argv0, L"-c") != 0)
            nr = _Py_wreadlink(argv0, link, MAXPATHLEN);
        if (nr > 0) {
            /* It's a symlink */
            link[nr] = '\0';
            if (link[0] == SEP)
                argv0 = link; /* Link to absolute path */
            else if (wcschr(link, SEP) == NULL)
                ; /* Link without path */
            else {
                /* Must join(dirname(argv0), link) */
                wchar_t *q = wcsrchr(argv0, SEP);
                if (q == NULL)
                    argv0 = link; /* argv0 without path */
                else {
                    /* Must make a copy */
                    wcscpy(argv0copy, argv0);
                    q = wcsrchr(argv0copy, SEP);
                    wcscpy(q+1, link);
                    argv0 = argv0copy;
                }
            }
        }
#endif /* HAVE_READLINK */
#if SEP == '\\' /* Special case for MS filename syntax */
        if (argc > 0 && argv0 != NULL && wcscmp(argv0, L"-c") != 0) {
            wchar_t *q;
#if defined(MS_WINDOWS) && !defined(MS_WINCE)
            /* This code here replaces the first element in argv with the full
            path that it represents. Under CE, there are no relative paths so
            the argument must be the full path anyway. */
            wchar_t *ptemp;
            if (GetFullPathNameW(argv0,
                               sizeof(fullpath)/sizeof(fullpath[0]),
                               fullpath,
                               &ptemp)) {
                argv0 = fullpath;
            }
#endif
            p = wcsrchr(argv0, SEP);
            /* Test for alternate separator */
            q = wcsrchr(p ? p : argv0, '/');
            if (q != NULL)
                p = q;
            if (p != NULL) {
                n = p + 1 - argv0;
                if (n > 1 && p[-1] != ':')
                    n--; /* Drop trailing separator */
            }
        }
#else /* All other filename syntaxes */
        if (argc > 0 && argv0 != NULL && wcscmp(argv0, L"-c") != 0) {
#if defined(HAVE_REALPATH)
            if (_wrealpath(argv0, fullpath)) {
                argv0 = fullpath;
            }
#endif
            p = wcsrchr(argv0, SEP);
        }
        if (p != NULL) {
            n = p + 1 - argv0;
#if SEP == '/' /* Special case for Unix filename syntax */
            if (n > 1)
                n--; /* Drop trailing separator */
#endif /* Unix */
        }
#endif /* All others */
        a = PyUnicode_FromWideChar(argv0, n);
        if (a == NULL)
            Py_FatalError("no mem for sys.path insertion");
        if (PyList_Insert(path, 0, a) < 0)
            Py_FatalError("sys.path.insert(0) failed");
        Py_DECREF(a);
    }
    Py_DECREF(av);
}
{
#if defined(HAVE_REALPATH)
    wchar_t fullpath[MAXPATHLEN];
#elif defined(MS_WINDOWS) && !defined(MS_WINCE)
    wchar_t fullpath[MAX_PATH];
#endif
    PyObject *av = makeargvobject(argc, argv);
    PyObject *path = PySys_GetObject("path");
    if (av == NULL)
        Py_FatalError("no mem for sys.argv");
    if (PySys_SetObject("argv", av) != 0)
        Py_FatalError("can't assign sys.argv");
    if (updatepath && path != NULL) {
        wchar_t *argv0 = argv[0];
        wchar_t *p = NULL;
        Py_ssize_t n = 0;
        PyObject *a;
        extern int _Py_wreadlink(const wchar_t *, wchar_t *, size_t);
#ifdef HAVE_READLINK
        wchar_t link[MAXPATHLEN+1];
        wchar_t argv0copy[2*MAXPATHLEN+1];
        int nr = 0;
        if (argc > 0 && argv0 != NULL && wcscmp(argv0, L"-c") != 0)
            nr = _Py_wreadlink(argv0, link, MAXPATHLEN);
        if (nr > 0) {
            /* It's a symlink */
            link[nr] = '\0';
            if (link[0] == SEP)
                argv0 = link; /* Link to absolute path */
            else if (wcschr(link, SEP) == NULL)
                ; /* Link without path */
            else {
                /* Must join(dirname(argv0), link) */
                wchar_t *q = wcsrchr(argv0, SEP);
                if (q == NULL)
                    argv0 = link; /* argv0 without path */
                else {
                    /* Must make a copy */
                    wcscpy(argv0copy, argv0);
                    q = wcsrchr(argv0copy, SEP);
                    wcscpy(q+1, link);
                    argv0 = argv0copy;
                }
            }
        }
#endif /* HAVE_READLINK */
#if SEP == '\\' /* Special case for MS filename syntax */
        if (argc > 0 && argv0 != NULL && wcscmp(argv0, L"-c") != 0) {
            wchar_t *q;
#if defined(MS_WINDOWS) && !defined(MS_WINCE)
            /* This code here replaces the first element in argv with the full
            path that it represents. Under CE, there are no relative paths so
            the argument must be the full path anyway. */
            wchar_t *ptemp;
            if (GetFullPathNameW(argv0,
                               sizeof(fullpath)/sizeof(fullpath[0]),
                               fullpath,
                               &ptemp)) {
                argv0 = fullpath;
            }
#endif
            p = wcsrchr(argv0, SEP);
            /* Test for alternate separator */
            q = wcsrchr(p ? p : argv0, '/');
            if (q != NULL)
                p = q;
            if (p != NULL) {
                n = p + 1 - argv0;
                if (n > 1 && p[-1] != ':')
                    n--; /* Drop trailing separator */
            }
        }
#else /* All other filename syntaxes */
        if (argc > 0 && argv0 != NULL && wcscmp(argv0, L"-c") != 0) {
#if defined(HAVE_REALPATH)
            if (_wrealpath(argv0, fullpath)) {
                argv0 = fullpath;
            }
#endif
            p = wcsrchr(argv0, SEP);
        }
        if (p != NULL) {
            n = p + 1 - argv0;
#if SEP == '/' /* Special case for Unix filename syntax */
            if (n > 1)
                n--; /* Drop trailing separator */
#endif /* Unix */
        }
#endif /* All others */
        a = PyUnicode_FromWideChar(argv0, n);
        if (a == NULL)
            Py_FatalError("no mem for sys.path insertion");
        if (PyList_Insert(path, 0, a) < 0)
            Py_FatalError("sys.path.insert(0) failed");
        Py_DECREF(a);
    }
        if (p != NULL) {
            n = p + 1 - argv0;
#if SEP == '/' /* Special case for Unix filename syntax */
            if (n > 1)
                n--; /* Drop trailing separator */
#endif /* Unix */
        }
#endif /* All others */
        a = PyUnicode_FromWideChar(argv0, n);
        if (a == NULL)
            Py_FatalError("no mem for sys.path insertion");
        if (PyList_Insert(path, 0, a) < 0)
            Py_FatalError("sys.path.insert(0) failed");
        Py_DECREF(a);
    }
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
sys_pyfile_write(const char *text, PyObject *file)
{
    PyObject *unicode = NULL, *writer = NULL, *args = NULL, *result = NULL;
    int err;

    unicode = PyUnicode_FromString(text);
    if (unicode == NULL)
        goto error;

    writer = PyObject_GetAttrString(file, "write");
    if (writer == NULL)
        goto error;

    args = PyTuple_Pack(1, unicode);
    if (args == NULL)
        goto error;

    result = PyEval_CallObject(writer, args);
    if (result == NULL) {
        goto error;
    } else {
        err = 0;
        goto finally;
    }

error:
    err = -1;
finally:
    Py_XDECREF(unicode);
    Py_XDECREF(writer);
    Py_XDECREF(args);
    Py_XDECREF(result);
    return err;
}