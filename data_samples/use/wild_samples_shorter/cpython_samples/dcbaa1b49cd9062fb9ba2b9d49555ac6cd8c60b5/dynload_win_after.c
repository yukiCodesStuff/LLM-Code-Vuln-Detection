    char funcname[258], *import_python;
    const wchar_t *wpathname;

    _Py_CheckPython3();

    wpathname = _PyUnicode_AsUnicode(pathname);
    if (wpathname == NULL)
        return NULL;