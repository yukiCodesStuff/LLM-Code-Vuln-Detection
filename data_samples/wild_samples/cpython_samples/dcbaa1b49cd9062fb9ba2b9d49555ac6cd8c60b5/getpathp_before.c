typedef struct {
    const wchar_t *path_env;           /* PATH environment variable */
    const wchar_t *home;               /* PYTHONHOME environment variable */

    /* Registry key "Software\Python\PythonCore\X.Y\PythonPath"
       where X.Y is the Python version (major.minor) */
    wchar_t *machine_path;   /* from HKEY_LOCAL_MACHINE */
    wchar_t *user_path;      /* from HKEY_CURRENT_USER */

    wchar_t *dll_path;

    const wchar_t *pythonpath_env;
} PyCalculatePath;


/* determine if "ch" is a separator character */
static int
is_sep(wchar_t ch)
{
#ifdef ALTSEP
    return ch == SEP || ch == ALTSEP;
#else
    return ch == SEP;
#endif
}
    if (i >= MAXPATHLEN+1) {
        Py_FatalError("buffer overflow in getpathp.c's reduce()");
    }

    while (i > 0 && !is_sep(dir[i]))
        --i;
    dir[i] = '\0';
}


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

    return 0;
}
        if (gotlandmark(prefix, landmark)) {
            return 1;
        }
        reduce(prefix);
    } while (prefix[0]);
    return 0;
}


#ifdef Py_ENABLE_SHARED

/* a string loaded from the DLL at startup.*/
extern const char *PyWin_DLLVersionString;

/* Load a PYTHONPATH value from the registry.
   Load from either HKEY_LOCAL_MACHINE or HKEY_CURRENT_USER.

   Works in both Unicode and 8bit environments.  Only uses the
   Ex family of functions so it also works with Windows CE.

   Returns NULL, or a pointer that should be freed.

   XXX - this code is pretty strange, as it used to also
   work on Win16, where the buffer sizes werent available
   in advance.  It could be simplied now Win16/Win32s is dead!
*/
static wchar_t *
getpythonregpath(HKEY keyBase, int skipcore)
{
    HKEY newKey = 0;
    DWORD dataSize = 0;
    DWORD numKeys = 0;
    LONG rc;
    wchar_t *retval = NULL;
    WCHAR *dataBuf = NULL;
    static const WCHAR keyPrefix[] = L"Software\\Python\\PythonCore\\";
    static const WCHAR keySuffix[] = L"\\PythonPath";
    size_t versionLen, keyBufLen;
    DWORD index;
    WCHAR *keyBuf = NULL;
    WCHAR *keyBufPtr;
    WCHAR **ppPaths = NULL;

    /* Tried to use sysget("winver") but here is too early :-( */
    versionLen = strlen(PyWin_DLLVersionString);
    /* Space for all the chars, plus one \0 */
    keyBufLen = sizeof(keyPrefix) +
                sizeof(WCHAR)*(versionLen-1) +
                sizeof(keySuffix);
    keyBuf = keyBufPtr = PyMem_RawMalloc(keyBufLen);
    if (keyBuf==NULL) {
        goto done;
    }

    memcpy_s(keyBufPtr, keyBufLen, keyPrefix, sizeof(keyPrefix)-sizeof(WCHAR));
    keyBufPtr += Py_ARRAY_LENGTH(keyPrefix) - 1;
    mbstowcs(keyBufPtr, PyWin_DLLVersionString, versionLen);
    keyBufPtr += versionLen;
    /* NULL comes with this one! */
    memcpy(keyBufPtr, keySuffix, sizeof(keySuffix));
    /* Open the root Python key */
    rc=RegOpenKeyExW(keyBase,
                    keyBuf, /* subkey */
            0, /* reserved */
            KEY_READ,
            &newKey);
    if (rc!=ERROR_SUCCESS) {
        goto done;
    }
    /* Find out how big our core buffer is, and how many subkeys we have */
    rc = RegQueryInfoKeyW(newKey, NULL, NULL, NULL, &numKeys, NULL, NULL,
                    NULL, NULL, &dataSize, NULL, NULL);
    if (rc!=ERROR_SUCCESS) {
        goto done;
    }
    if (skipcore) {
        dataSize = 0; /* Only count core ones if we want them! */
    }
    /* Allocate a temp array of char buffers, so we only need to loop
       reading the registry once
    */
    ppPaths = PyMem_RawCalloc(numKeys, sizeof(WCHAR *));
    if (ppPaths==NULL) {
        goto done;
    }
    /* Loop over all subkeys, allocating a temp sub-buffer. */
    for(index=0;index<numKeys;index++) {
        WCHAR keyBuf[MAX_PATH+1];
        HKEY subKey = 0;
        DWORD reqdSize = MAX_PATH+1;
        /* Get the sub-key name */
        DWORD rc = RegEnumKeyExW(newKey, index, keyBuf, &reqdSize,
                                 NULL, NULL, NULL, NULL );
        if (rc!=ERROR_SUCCESS) {
            goto done;
        }
        /* Open the sub-key */
        rc=RegOpenKeyExW(newKey,
                                        keyBuf, /* subkey */
                        0, /* reserved */
                        KEY_READ,
                        &subKey);
        if (rc!=ERROR_SUCCESS) {
            goto done;
        }
        /* Find the value of the buffer size, malloc, then read it */
        RegQueryValueExW(subKey, NULL, 0, NULL, NULL, &reqdSize);
        if (reqdSize) {
            ppPaths[index] = PyMem_RawMalloc(reqdSize);
            if (ppPaths[index]) {
                RegQueryValueExW(subKey, NULL, 0, NULL,
                                (LPBYTE)ppPaths[index],
                                &reqdSize);
                dataSize += reqdSize + 1; /* 1 for the ";" */
            }
        }
        RegCloseKey(subKey);
    }

    /* return null if no path to return */
    if (dataSize == 0) {
        goto done;
    }

    /* original datasize from RegQueryInfo doesn't include the \0 */
    dataBuf = PyMem_RawMalloc((dataSize+1) * sizeof(WCHAR));
    if (dataBuf) {
        WCHAR *szCur = dataBuf;
        /* Copy our collected strings */
        for (index=0;index<numKeys;index++) {
            if (index > 0) {
                *(szCur++) = L';';
                dataSize--;
            }
            if (ppPaths[index]) {
                Py_ssize_t len = wcslen(ppPaths[index]);
                wcsncpy(szCur, ppPaths[index], len);
                szCur += len;
                assert(dataSize > (DWORD)len);
                dataSize -= (DWORD)len;
            }
        }
        if (skipcore) {
            *szCur = '\0';
        }
        else {
            /* If we have no values, we don't need a ';' */
            if (numKeys) {
                *(szCur++) = L';';
                dataSize--;
            }
            /* Now append the core path entries -
               this will include the NULL
            */
            rc = RegQueryValueExW(newKey, NULL, 0, NULL,
                                  (LPBYTE)szCur, &dataSize);
            if (rc != ERROR_SUCCESS) {
                PyMem_RawFree(dataBuf);
                goto done;
            }
        }
        /* And set the result - caller must free */
        retval = dataBuf;
    }
done:
    /* Loop freeing my temp buffers */
    if (ppPaths) {
        for(index=0; index<numKeys; index++)
            PyMem_RawFree(ppPaths[index]);
        PyMem_RawFree(ppPaths);
    }
    if (newKey) {
        RegCloseKey(newKey);
    }
    PyMem_RawFree(keyBuf);
    return retval;
}
    if (newKey) {
        RegCloseKey(newKey);
    }
    PyMem_RawFree(keyBuf);
    return retval;
}
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
        if (pathconfig->module_search_path == NULL) {
            status = _PyStatus_NO_MEMORY();
            goto done;
        }
    }

    *found = 1;
    status = _PyStatus_OK();
    goto done;

done:
    PyMem_RawFree(buf);
    PyMem_RawFree(wline);
    fclose(sp_file);
    return status;
}


static int
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
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    /* Calculate zip archive path from DLL or exe path */
    wchar_t zip_path[MAXPATHLEN+1];
    memset(zip_path, 0, sizeof(zip_path));

    change_ext(zip_path,
               calculate->dll_path[0] ? calculate->dll_path : pathconfig->program_full_path,
               L".zip");

    calculate_home_prefix(calculate, argv0_path, zip_path, prefix);

    if (pathconfig->module_search_path == NULL) {
        status = calculate_module_search_path(calculate, pathconfig,
                                              argv0_path, prefix, zip_path);
        if (_PyStatus_EXCEPTION(status)) {
            return status;
        }
    }
{
    calculate->home = pathconfig->home;
    calculate->path_env = _wgetenv(L"PATH");

    calculate->dll_path = _Py_GetDLLPath();
    if (calculate->dll_path == NULL) {
        return _PyStatus_NO_MEMORY();
    }
{
    PyMem_RawFree(calculate->machine_path);
    PyMem_RawFree(calculate->user_path);
    PyMem_RawFree(calculate->dll_path);
}


/* Calculate the Python path configuration.

   Inputs:

   - PyConfig.pythonpath_env: PYTHONPATH environment variable
   - _PyPathConfig.home: Py_SetPythonHome() or PYTHONHOME environment variable
   - DLL path: _Py_GetDLLPath()
   - PATH environment variable
   - __PYVENV_LAUNCHER__ environment variable
   - GetModuleFileNameW(NULL): fully qualified path of the executable file of
     the current process
   - ._pth configuration file
   - pyvenv.cfg configuration file
   - Registry key "Software\Python\PythonCore\X.Y\PythonPath"
     of HKEY_CURRENT_USER and HKEY_LOCAL_MACHINE where X.Y is the Python
     version.

   Outputs, 'pathconfig' fields:

   - base_executable
   - program_full_path
   - module_search_path
   - prefix
   - exec_prefix
   - isolated
   - site_import

   If a field is already set (non NULL), it is left unchanged. */
PyStatus
_PyPathConfig_Calculate(_PyPathConfig *pathconfig, const PyConfig *config)
{
    PyStatus status;
    PyCalculatePath calculate;
    memset(&calculate, 0, sizeof(calculate));

    status = calculate_init(&calculate, pathconfig, config);
    if (_PyStatus_EXCEPTION(status)) {
        goto done;
    }
{
    PyMem_RawFree(calculate->machine_path);
    PyMem_RawFree(calculate->user_path);
    PyMem_RawFree(calculate->dll_path);
}


/* Calculate the Python path configuration.

   Inputs:

   - PyConfig.pythonpath_env: PYTHONPATH environment variable
   - _PyPathConfig.home: Py_SetPythonHome() or PYTHONHOME environment variable
   - DLL path: _Py_GetDLLPath()
   - PATH environment variable
   - __PYVENV_LAUNCHER__ environment variable
   - GetModuleFileNameW(NULL): fully qualified path of the executable file of
     the current process
   - ._pth configuration file
   - pyvenv.cfg configuration file
   - Registry key "Software\Python\PythonCore\X.Y\PythonPath"
     of HKEY_CURRENT_USER and HKEY_LOCAL_MACHINE where X.Y is the Python
     version.

   Outputs, 'pathconfig' fields:

   - base_executable
   - program_full_path
   - module_search_path
   - prefix
   - exec_prefix
   - isolated
   - site_import

   If a field is already set (non NULL), it is left unchanged. */
PyStatus
_PyPathConfig_Calculate(_PyPathConfig *pathconfig, const PyConfig *config)
{
    PyStatus status;
    PyCalculatePath calculate;
    memset(&calculate, 0, sizeof(calculate));

    status = calculate_init(&calculate, pathconfig, config);
    if (_PyStatus_EXCEPTION(status)) {
        goto done;
    }

    status = calculate_path(&calculate, pathconfig);

done:
    calculate_free(&calculate);
    return status;
}
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