    wchar_t *machine_path;   /* from HKEY_LOCAL_MACHINE */
    wchar_t *user_path;      /* from HKEY_CURRENT_USER */

    wchar_t *dll_path;

    const wchar_t *pythonpath_env;
} PyCalculatePath;


static int
change_ext(wchar_t *dest, const wchar_t *src, const wchar_t *ext)
{
    size_t src_len = wcsnlen_s(src, MAXPATHLEN+1);
    size_t i = src_len;
    if (i >= MAXPATHLEN+1) {
        Py_FatalError("buffer overflow in getpathp.c's reduce()");
    }

    while (i > 0 && src[i] != '.' && !is_sep(src[i]))
        --i;

    if (i == 0) {
        dest[0] = '\0';
        return -1;
    }

    if (is_sep(src[i])) {
        i = src_len;
    }

    if (wcsncpy_s(dest, MAXPATHLEN+1, src, i) ||
        wcscat_s(dest, MAXPATHLEN+1, ext))
    {
        dest[0] = '\0';
        return -1;
    }

}


#ifdef Py_ENABLE_SHARED

/* a string loaded from the DLL at startup.*/
extern const char *PyWin_DLLVersionString;
#endif /* Py_ENABLE_SHARED */


wchar_t*
_Py_GetDLLPath(void)
{
    wchar_t dll_path[MAXPATHLEN+1];
    memset(dll_path, 0, sizeof(dll_path));

#ifdef Py_ENABLE_SHARED
    extern HANDLE PyWin_DLLhModule;
    if (PyWin_DLLhModule) {
        if (!GetModuleFileNameW(PyWin_DLLhModule, dll_path, MAXPATHLEN)) {
            dll_path[0] = 0;
        }
    }
#else
    dll_path[0] = 0;
#endif

    return _PyMem_RawWcsdup(dll_path);
}


static PyStatus
get_program_full_path(_PyPathConfig *pathconfig)
{
    PyStatus status;
get_pth_filename(PyCalculatePath *calculate, wchar_t *filename,
                 const _PyPathConfig *pathconfig)
{
    if (calculate->dll_path[0]) {
        if (!change_ext(filename, calculate->dll_path, L"._pth") &&
            exists(filename))
        {
            return 1;
        }
    }
    if (pathconfig->program_full_path[0]) {
        if (!change_ext(filename, pathconfig->program_full_path, L"._pth") &&
            exists(filename))
        {
            return 1;
        }
    }
    return 0;
}

    wchar_t zip_path[MAXPATHLEN+1];
    memset(zip_path, 0, sizeof(zip_path));

    change_ext(zip_path,
               calculate->dll_path[0] ? calculate->dll_path : pathconfig->program_full_path,
               L".zip");

    calculate_home_prefix(calculate, argv0_path, zip_path, prefix);

    if (pathconfig->module_search_path == NULL) {
    calculate->home = pathconfig->home;
    calculate->path_env = _wgetenv(L"PATH");

    calculate->dll_path = _Py_GetDLLPath();
    if (calculate->dll_path == NULL) {
        return _PyStatus_NO_MEMORY();
    }

    calculate->pythonpath_env = config->pythonpath_env;

    return _PyStatus_OK();
}
{
    PyMem_RawFree(calculate->machine_path);
    PyMem_RawFree(calculate->user_path);
    PyMem_RawFree(calculate->dll_path);
}


/* Calculate the Python path configuration.

   - PyConfig.pythonpath_env: PYTHONPATH environment variable
   - _PyPathConfig.home: Py_SetPythonHome() or PYTHONHOME environment variable
   - DLL path: _Py_GetDLLPath()
   - PATH environment variable
   - __PYVENV_LAUNCHER__ environment variable
   - GetModuleFileNameW(NULL): fully qualified path of the executable file of
     the current process
_Py_CheckPython3(void)
{
    wchar_t py3path[MAXPATHLEN+1];
    wchar_t *s;
    if (python3_checked) {
        return hPython3 != NULL;
    }
    python3_checked = 1;

    /* If there is a python3.dll next to the python3y.dll,
       assume this is a build tree; use that DLL */
    if (_Py_dll_path != NULL) {
        wcscpy(py3path, _Py_dll_path);
    }
    else {
        wcscpy(py3path, L"");
    }
    s = wcsrchr(py3path, L'\\');
    if (!s) {
        s = py3path;
    }
    wcscpy(s, L"\\python3.dll");
    hPython3 = LoadLibraryExW(py3path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (hPython3 != NULL) {
        return 1;
    }

    /* Check sys.prefix\DLLs\python3.dll */
    wcscpy(py3path, Py_GetPrefix());
    wcscat(py3path, L"\\DLLs\\python3.dll");
    hPython3 = LoadLibraryExW(py3path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    return hPython3 != NULL;
}