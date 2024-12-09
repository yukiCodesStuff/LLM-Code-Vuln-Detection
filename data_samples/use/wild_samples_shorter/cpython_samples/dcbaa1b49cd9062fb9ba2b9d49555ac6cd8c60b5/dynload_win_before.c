    char funcname[258], *import_python;
    const wchar_t *wpathname;

#ifndef _DEBUG
    _Py_CheckPython3();
#endif

    wpathname = _PyUnicode_AsUnicode(pathname);
    if (wpathname == NULL)
        return NULL;