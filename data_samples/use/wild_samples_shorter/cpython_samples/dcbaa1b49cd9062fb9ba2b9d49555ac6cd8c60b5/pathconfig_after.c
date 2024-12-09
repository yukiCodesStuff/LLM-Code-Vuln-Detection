

_PyPathConfig _Py_path_config = _PyPathConfig_INIT;


static int
copy_wstr(wchar_t **dst, const wchar_t *src)
    _PyMem_SetDefaultAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

    pathconfig_clear(&_Py_path_config);

    PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &old_alloc);
}

}


static PyStatus
pathconfig_set_from_config(_PyPathConfig *pathconfig, const PyConfig *config)
{
    PyStatus status;
PyStatus
_PyConfig_WritePathConfig(const PyConfig *config)
{
    return pathconfig_set_from_config(&_Py_path_config, config);
}


{
    PyStatus status;

    if (_Py_path_config.module_search_path == NULL) {
        status = pathconfig_global_read(&_Py_path_config);
        if (_PyStatus_EXCEPTION(status)) {
            Py_ExitStatusException(status);