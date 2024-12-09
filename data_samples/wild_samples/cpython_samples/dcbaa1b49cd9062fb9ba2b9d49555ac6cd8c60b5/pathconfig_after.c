extern "C" {
#endif


_PyPathConfig _Py_path_config = _PyPathConfig_INIT;


static int
copy_wstr(wchar_t **dst, const wchar_t *src)
{
    assert(*dst == NULL);
    if (src != NULL) {
        *dst = _PyMem_RawWcsdup(src);
        if (*dst == NULL) {
            return -1;
        }
    }
    else {
        *dst = NULL;
    }
    return 0;
}
{
    PyMemAllocatorEx old_alloc;
    _PyMem_SetDefaultAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

    pathconfig_clear(&_Py_path_config);

    PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &old_alloc);
}


static wchar_t*
_PyWideStringList_Join(const PyWideStringList *list, wchar_t sep)
{
    size_t len = 1;   /* NUL terminator */
    for (Py_ssize_t i=0; i < list->length; i++) {
        if (i != 0) {
            len++;
        }
        len += wcslen(list->items[i]);
    }
        if (i != 0) {
            *str++ = sep;
        }
        len = wcslen(path);
        memcpy(str, path, len * sizeof(wchar_t));
        str += len;
    }
    *str = L'\0';

    return text;
}


static PyStatus
pathconfig_set_from_config(_PyPathConfig *pathconfig, const PyConfig *config)
{
    PyStatus status;
    PyMemAllocatorEx old_alloc;
    _PyMem_SetDefaultAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

    if (config->module_search_paths_set) {
        PyMem_RawFree(pathconfig->module_search_path);
        pathconfig->module_search_path = _PyWideStringList_Join(&config->module_search_paths, DELIM);
        if (pathconfig->module_search_path == NULL) {
            goto no_memory;
        }
    }

#define COPY_CONFIG(PATH_ATTR, CONFIG_ATTR) \
        if (config->CONFIG_ATTR) { \
            PyMem_RawFree(pathconfig->PATH_ATTR); \
            pathconfig->PATH_ATTR = NULL; \
            if (copy_wstr(&pathconfig->PATH_ATTR, config->CONFIG_ATTR) < 0) { \
                goto no_memory; \
            } \
        }
        if (pathconfig->module_search_path == NULL) {
            goto no_memory;
        }
    }

#define COPY_CONFIG(PATH_ATTR, CONFIG_ATTR) \
        if (config->CONFIG_ATTR) { \
            PyMem_RawFree(pathconfig->PATH_ATTR); \
            pathconfig->PATH_ATTR = NULL; \
            if (copy_wstr(&pathconfig->PATH_ATTR, config->CONFIG_ATTR) < 0) { \
                goto no_memory; \
            } \
        }

    COPY_CONFIG(program_full_path, executable);
    COPY_CONFIG(prefix, prefix);
    COPY_CONFIG(exec_prefix, exec_prefix);
    COPY_CONFIG(program_name, program_name);
    COPY_CONFIG(home, home);
#ifdef MS_WINDOWS
    COPY_CONFIG(base_executable, base_executable);
#endif

#undef COPY_CONFIG

    status = _PyStatus_OK();
    goto done;

no_memory:
    status = _PyStatus_NO_MEMORY();

done:
    PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &old_alloc);
    return status;
}


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
        }
    }