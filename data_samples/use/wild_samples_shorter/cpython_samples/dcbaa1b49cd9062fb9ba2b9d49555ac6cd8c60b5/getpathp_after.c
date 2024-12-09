    wchar_t *machine_path;   /* from HKEY_LOCAL_MACHINE */
    wchar_t *user_path;      /* from HKEY_CURRENT_USER */

    const wchar_t *pythonpath_env;
} PyCalculatePath;


static int
change_ext(wchar_t *dest, const wchar_t *src, const wchar_t *ext)
{
    if (src && src != dest) {
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

        if (wcsncpy_s(dest, MAXPATHLEN+1, src, i)) {
            dest[0] = '\0';
            return -1;
        }
    } else {
        wchar_t *s = wcsrchr(dest, L'.');
        if (s) {
            s[0] = '\0';
        }
    }

    if (wcscat_s(dest, MAXPATHLEN+1, ext)) {
        dest[0] = '\0';
        return -1;
    }

}


static int
get_dllpath(wchar_t *dllpath)
{
#ifdef Py_ENABLE_SHARED
    extern HANDLE PyWin_DLLhModule;
    if (PyWin_DLLhModule && GetModuleFileNameW(PyWin_DLLhModule, dllpath, MAXPATHLEN)) {
        return 0;
    }
#endif
    return -1;
}


#ifdef Py_ENABLE_SHARED

/* a string loaded from the DLL at startup.*/
extern const char *PyWin_DLLVersionString;
#endif /* Py_ENABLE_SHARED */


static PyStatus
get_program_full_path(_PyPathConfig *pathconfig)
{
    PyStatus status;
get_pth_filename(PyCalculatePath *calculate, wchar_t *filename,
                 const _PyPathConfig *pathconfig)
{
    if (get_dllpath(filename) &&
        !change_ext(filename, filename, L"._pth") &&
        exists(filename))
    {
        return 1;
    }
    if (pathconfig->program_full_path[0] &&
        !change_ext(filename, pathconfig->program_full_path, L"._pth") &&
        exists(filename))
    {
        return 1;
    }
    return 0;
}

    wchar_t zip_path[MAXPATHLEN+1];
    memset(zip_path, 0, sizeof(zip_path));

    if (get_dllpath(zip_path) || change_ext(zip_path, zip_path, L".zip"))
    {
        if (change_ext(zip_path, pathconfig->program_full_path, L".zip")) {
            zip_path[0] = L'\0';
        }
    }

    calculate_home_prefix(calculate, argv0_path, zip_path, prefix);

    if (pathconfig->module_search_path == NULL) {
    calculate->home = pathconfig->home;
    calculate->path_env = _wgetenv(L"PATH");

    calculate->pythonpath_env = config->pythonpath_env;

    return _PyStatus_OK();
}
{
    PyMem_RawFree(calculate->machine_path);
    PyMem_RawFree(calculate->user_path);
}


/* Calculate the Python path configuration.

   - PyConfig.pythonpath_env: PYTHONPATH environment variable
   - _PyPathConfig.home: Py_SetPythonHome() or PYTHONHOME environment variable
   - PATH environment variable
   - __PYVENV_LAUNCHER__ environment variable
   - GetModuleFileNameW(NULL): fully qualified path of the executable file of
     the current process
_Py_CheckPython3(void)
{
    wchar_t py3path[MAXPATHLEN+1];
    if (python3_checked) {
        return hPython3 != NULL;
    }
    python3_checked = 1;

    /* If there is a python3.dll next to the python3y.dll,
       use that DLL */
    if (!get_dllpath(py3path)) {
        reduce(py3path);
        join(py3path, PY3_DLLNAME);
        hPython3 = LoadLibraryExW(py3path, NULL, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        if (hPython3 != NULL) {
            return 1;
        }
    }

    /* If we can locate python3.dll in our application dir,
       use that DLL */
    hPython3 = LoadLibraryExW(PY3_DLLNAME, NULL, LOAD_LIBRARY_SEARCH_APPLICATION_DIR);
    if (hPython3 != NULL) {
        return 1;
    }

    /* For back-compat, also search {sys.prefix}\DLLs, though
       that has not been a normal install layout for a while */
    wcscpy(py3path, Py_GetPrefix());
    if (py3path[0]) {
        join(py3path, L"DLLs\\" PY3_DLLNAME);
        hPython3 = LoadLibraryExW(py3path, NULL, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    }
    return hPython3 != NULL;
}