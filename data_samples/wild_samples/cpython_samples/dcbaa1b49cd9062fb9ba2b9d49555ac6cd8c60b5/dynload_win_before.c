{
    dl_funcptr p;
    char funcname[258], *import_python;
    const wchar_t *wpathname;

#ifndef _DEBUG
    _Py_CheckPython3();
#endif

    wpathname = _PyUnicode_AsUnicode(pathname);
    if (wpathname == NULL)
        return NULL;

    PyOS_snprintf(funcname, sizeof(funcname), "%.20s_%.200s", prefix, shortname);

    {
        HINSTANCE hDLL = NULL;
        unsigned int old_mode;

        /* Don't display a message box when Python can't load a DLL */
        old_mode = SetErrorMode(SEM_FAILCRITICALERRORS);

        /* bpo-36085: We use LoadLibraryEx with restricted search paths
           to avoid DLL preloading attacks and enable use of the
           AddDllDirectory function. We add SEARCH_DLL_LOAD_DIR to
           ensure DLLs adjacent to the PYD are preferred. */
        Py_BEGIN_ALLOW_THREADS
        hDLL = LoadLibraryExW(wpathname, NULL,
                              LOAD_LIBRARY_SEARCH_DEFAULT_DIRS |
                              LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);
        Py_END_ALLOW_THREADS

        /* restore old error mode settings */
        SetErrorMode(old_mode);

        if (hDLL==NULL){
            PyObject *message;
            unsigned int errorCode;

            /* Get an error string from Win32 error code */
            wchar_t theInfo[256]; /* Pointer to error text
                                  from system */
            int theLength; /* Length of error text */

            errorCode = GetLastError();

            theLength = FormatMessageW(
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS, /* flags */
                NULL, /* message source */
                errorCode, /* the message (error) ID */
                MAKELANGID(LANG_NEUTRAL,
                           SUBLANG_DEFAULT),
                           /* Default language */
                theInfo, /* the buffer */
                sizeof(theInfo) / sizeof(wchar_t), /* size in wchars */
                NULL); /* no additional format args. */

            /* Problem: could not get the error message.
               This should not happen if called correctly. */
            if (theLength == 0) {
                message = PyUnicode_FromFormat(
                    "DLL load failed with error code %u while importing %s",
                    errorCode, shortname);
            } else {
                /* For some reason a \r\n
                   is appended to the text */
                if (theLength >= 2 &&
                    theInfo[theLength-2] == '\r' &&
                    theInfo[theLength-1] == '\n') {
                    theLength -= 2;
                    theInfo[theLength] = '\0';
                }
                message = PyUnicode_FromFormat(
                    "DLL load failed while importing %s: ", shortname);

                PyUnicode_AppendAndDel(&message,
                    PyUnicode_FromWideChar(
                        theInfo,
                        theLength));
            }
            if (message != NULL) {
                PyObject *shortname_obj = PyUnicode_FromString(shortname);
                PyErr_SetImportError(message, shortname_obj, pathname);
                Py_XDECREF(shortname_obj);
                Py_DECREF(message);
            }
            return NULL;
        } else {
            char buffer[256];

            PyOS_snprintf(buffer, sizeof(buffer),
#ifdef _DEBUG
                          "python%d%d_d.dll",
#else
                          "python%d%d.dll",
#endif
                          PY_MAJOR_VERSION,PY_MINOR_VERSION);
            import_python = GetPythonImport(hDLL);

            if (import_python &&
                _stricmp(buffer,import_python)) {
                PyErr_Format(PyExc_ImportError,
                             "Module use of %.150s conflicts "
                             "with this version of Python.",
                             import_python);
                Py_BEGIN_ALLOW_THREADS
                FreeLibrary(hDLL);
                Py_END_ALLOW_THREADS
                return NULL;
            }
        }
        Py_BEGIN_ALLOW_THREADS
        p = GetProcAddress(hDLL, funcname);
        Py_END_ALLOW_THREADS
    }

    return p;
}