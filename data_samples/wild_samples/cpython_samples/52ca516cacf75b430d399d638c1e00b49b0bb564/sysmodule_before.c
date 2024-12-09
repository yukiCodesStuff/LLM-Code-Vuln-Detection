            if (v == NULL) {
                Py_DECREF(av);
                av = NULL;
                break;
            }
            PyList_SetItem(av, i, v);
        }
    }
    return av;
}

void
PySys_SetArgv(int argc, char **argv)
{
#if defined(HAVE_REALPATH)
    char fullpath[MAXPATHLEN];
#elif defined(MS_WINDOWS) && !defined(MS_WINCE)
    char fullpath[MAX_PATH];
#endif
    PyObject *av = makeargvobject(argc, argv);
    PyObject *path = PySys_GetObject("path");
    if (av == NULL)
        Py_FatalError("no mem for sys.argv");
    if (PySys_SetObject("argv", av) != 0)
        Py_FatalError("can't assign sys.argv");
    if (path != NULL) {
        char *argv0 = argv[0];
        char *p = NULL;
        Py_ssize_t n = 0;
        PyObject *a;
#ifdef HAVE_READLINK
        char link[MAXPATHLEN+1];
        char argv0copy[2*MAXPATHLEN+1];
        int nr = 0;
        if (argc > 0 && argv0 != NULL && strcmp(argv0, "-c") != 0)
            nr = readlink(argv0, link, MAXPATHLEN);
        if (nr > 0) {
            /* It's a symlink */
            link[nr] = '\0';
            if (link[0] == SEP)
                argv0 = link; /* Link to absolute path */
            else if (strchr(link, SEP) == NULL)
                ; /* Link without path */
            else {
                /* Must join(dirname(argv0), link) */
                char *q = strrchr(argv0, SEP);
                if (q == NULL)
                    argv0 = link; /* argv0 without path */
                else {
                    /* Must make a copy */
                    strcpy(argv0copy, argv0);
                    q = strrchr(argv0copy, SEP);
                    strcpy(q+1, link);
                    argv0 = argv0copy;
                }
            }
        }
#endif /* HAVE_READLINK */
#if SEP == '\\' /* Special case for MS filename syntax */
        if (argc > 0 && argv0 != NULL && strcmp(argv0, "-c") != 0) {
            char *q;
#if defined(MS_WINDOWS) && !defined(MS_WINCE)
            /* This code here replaces the first element in argv with the full
            path that it represents. Under CE, there are no relative paths so
            the argument must be the full path anyway. */
            char *ptemp;
            if (GetFullPathName(argv0,
                               sizeof(fullpath),
                               fullpath,
                               &ptemp)) {
                argv0 = fullpath;
            }
#endif
            p = strrchr(argv0, SEP);
            /* Test for alternate separator */
            q = strrchr(p ? p : argv0, '/');
            if (q != NULL)
                p = q;
            if (p != NULL) {
                n = p + 1 - argv0;
                if (n > 1 && p[-1] != ':')
                    n--; /* Drop trailing separator */
            }
        }
#else /* All other filename syntaxes */
        if (argc > 0 && argv0 != NULL && strcmp(argv0, "-c") != 0) {
#if defined(HAVE_REALPATH)
            if (realpath(argv0, fullpath)) {
                argv0 = fullpath;
            }
#endif
            p = strrchr(argv0, SEP);
        }
        if (p != NULL) {
#ifndef RISCOS
            n = p + 1 - argv0;
#else /* don't include trailing separator */
            n = p - argv0;
#endif /* RISCOS */
#if SEP == '/' /* Special case for Unix filename syntax */
            if (n > 1)
                n--; /* Drop trailing separator */
#endif /* Unix */
        }
#endif /* All others */
        a = PyString_FromStringAndSize(argv0, n);
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
    char fullpath[MAXPATHLEN];
#elif defined(MS_WINDOWS) && !defined(MS_WINCE)
    char fullpath[MAX_PATH];
#endif
    PyObject *av = makeargvobject(argc, argv);
    PyObject *path = PySys_GetObject("path");
    if (av == NULL)
        Py_FatalError("no mem for sys.argv");
    if (PySys_SetObject("argv", av) != 0)
        Py_FatalError("can't assign sys.argv");
    if (path != NULL) {
        char *argv0 = argv[0];
        char *p = NULL;
        Py_ssize_t n = 0;
        PyObject *a;
#ifdef HAVE_READLINK
        char link[MAXPATHLEN+1];
        char argv0copy[2*MAXPATHLEN+1];
        int nr = 0;
        if (argc > 0 && argv0 != NULL && strcmp(argv0, "-c") != 0)
            nr = readlink(argv0, link, MAXPATHLEN);
        if (nr > 0) {
            /* It's a symlink */
            link[nr] = '\0';
            if (link[0] == SEP)
                argv0 = link; /* Link to absolute path */
            else if (strchr(link, SEP) == NULL)
                ; /* Link without path */
            else {
                /* Must join(dirname(argv0), link) */
                char *q = strrchr(argv0, SEP);
                if (q == NULL)
                    argv0 = link; /* argv0 without path */
                else {
                    /* Must make a copy */
                    strcpy(argv0copy, argv0);
                    q = strrchr(argv0copy, SEP);
                    strcpy(q+1, link);
                    argv0 = argv0copy;
                }
            }
        }
#endif /* HAVE_READLINK */
#if SEP == '\\' /* Special case for MS filename syntax */
        if (argc > 0 && argv0 != NULL && strcmp(argv0, "-c") != 0) {
            char *q;
#if defined(MS_WINDOWS) && !defined(MS_WINCE)
            /* This code here replaces the first element in argv with the full
            path that it represents. Under CE, there are no relative paths so
            the argument must be the full path anyway. */
            char *ptemp;
            if (GetFullPathName(argv0,
                               sizeof(fullpath),
                               fullpath,
                               &ptemp)) {
                argv0 = fullpath;
            }
#endif
            p = strrchr(argv0, SEP);
            /* Test for alternate separator */
            q = strrchr(p ? p : argv0, '/');
            if (q != NULL)
                p = q;
            if (p != NULL) {
                n = p + 1 - argv0;
                if (n > 1 && p[-1] != ':')
                    n--; /* Drop trailing separator */
            }
        }
#else /* All other filename syntaxes */
        if (argc > 0 && argv0 != NULL && strcmp(argv0, "-c") != 0) {
#if defined(HAVE_REALPATH)
            if (realpath(argv0, fullpath)) {
                argv0 = fullpath;
            }
#endif
            p = strrchr(argv0, SEP);
        }
        if (p != NULL) {
#ifndef RISCOS
            n = p + 1 - argv0;
#else /* don't include trailing separator */
            n = p - argv0;
#endif /* RISCOS */
#if SEP == '/' /* Special case for Unix filename syntax */
            if (n > 1)
                n--; /* Drop trailing separator */
#endif /* Unix */
        }
#endif /* All others */
        a = PyString_FromStringAndSize(argv0, n);
        if (a == NULL)
            Py_FatalError("no mem for sys.path insertion");
        if (PyList_Insert(path, 0, a) < 0)
            Py_FatalError("sys.path.insert(0) failed");
        Py_DECREF(a);
    }
        if (p != NULL) {
#ifndef RISCOS
            n = p + 1 - argv0;
#else /* don't include trailing separator */
            n = p - argv0;
#endif /* RISCOS */
#if SEP == '/' /* Special case for Unix filename syntax */
            if (n > 1)
                n--; /* Drop trailing separator */
#endif /* Unix */
        }
#endif /* All others */
        a = PyString_FromStringAndSize(argv0, n);
        if (a == NULL)
            Py_FatalError("no mem for sys.path insertion");
        if (PyList_Insert(path, 0, a) < 0)
            Py_FatalError("sys.path.insert(0) failed");
        Py_DECREF(a);
    }
    Py_DECREF(av);
}


/* APIs to write to sys.stdout or sys.stderr using a printf-like interface.
   Adapted from code submitted by Just van Rossum.

   PySys_WriteStdout(format, ...)
   PySys_WriteStderr(format, ...)

      The first function writes to sys.stdout; the second to sys.stderr.  When
      there is a problem, they write to the real (C level) stdout or stderr;
      no exceptions are raised.

      Both take a printf-style format string as their first argument followed
      by a variable length argument list determined by the format string.

      *** WARNING ***

      The format should limit the total size of the formatted output string to
      1000 bytes.  In particular, this means that no unrestricted "%s" formats
      should occur; these should be limited using "%.<N>s where <N> is a
      decimal number calculated so that <N> plus the maximum size of other
      formatted text does not exceed 1000 bytes.  Also watch out for "%f",
      which can print hundreds of digits for very large numbers.

 */

static void
mywrite(char *name, FILE *fp, const char *format, va_list va)
{
    PyObject *file;
    PyObject *error_type, *error_value, *error_traceback;

    PyErr_Fetch(&error_type, &error_value, &error_traceback);
    file = PySys_GetObject(name);
    if (file == NULL || PyFile_AsFile(file) == fp)
        vfprintf(fp, format, va);
    else {
        char buffer[1001];
        const int written = PyOS_vsnprintf(buffer, sizeof(buffer),
                                           format, va);
        if (PyFile_WriteString(buffer, file) != 0) {
            PyErr_Clear();
            fputs(buffer, fp);
        }
        if (written < 0 || (size_t)written >= sizeof(buffer)) {
            const char *truncated = "... truncated";
            if (PyFile_WriteString(truncated, file) != 0) {
                PyErr_Clear();
                fputs(truncated, fp);
            }
        }
    }