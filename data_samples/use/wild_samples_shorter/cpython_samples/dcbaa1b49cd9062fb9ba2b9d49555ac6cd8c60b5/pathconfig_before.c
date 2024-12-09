

_PyPathConfig _Py_path_config = _PyPathConfig_INIT;
#ifdef MS_WINDOWS
wchar_t *_Py_dll_path = NULL;
#endif


static int
copy_wstr(wchar_t **dst, const wchar_t *src)
    _PyMem_SetDefaultAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

    pathconfig_clear(&_Py_path_config);
#ifdef MS_WINDOWS
    PyMem_RawFree(_Py_dll_path);
    _Py_dll_path = NULL;
#endif

    PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &old_alloc);
}

}


#ifdef MS_WINDOWS
/* Initialize _Py_dll_path on Windows. Do nothing on other platforms. */
static PyStatus
_PyPathConfig_InitDLLPath(void)
{
    if (_Py_dll_path != NULL) {
        /* Already set: nothing to do */
        return _PyStatus_OK();
    }

    PyMemAllocatorEx old_alloc;
    _PyMem_SetDefaultAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

    _Py_dll_path = _Py_GetDLLPath();

    PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

    if (_Py_dll_path == NULL) {
        return _PyStatus_NO_MEMORY();
    }
    return _PyStatus_OK();
}
#endif


static PyStatus
pathconfig_set_from_config(_PyPathConfig *pathconfig, const PyConfig *config)
{
    PyStatus status;
PyStatus
_PyConfig_WritePathConfig(const PyConfig *config)
{
#ifdef MS_WINDOWS
    PyStatus status = _PyPathConfig_InitDLLPath();
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }
#endif

    return pathconfig_set_from_config(&_Py_path_config, config);
}


{
    PyStatus status;

#ifdef MS_WINDOWS
    status = _PyPathConfig_InitDLLPath();
    if (_PyStatus_EXCEPTION(status)) {
        Py_ExitStatusException(status);
    }
#endif

    if (_Py_path_config.module_search_path == NULL) {
        status = pathconfig_global_read(&_Py_path_config);
        if (_PyStatus_EXCEPTION(status)) {
            Py_ExitStatusException(status);