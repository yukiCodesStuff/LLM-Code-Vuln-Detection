typedef enum {
    /* Py_Initialize() API: backward compatibility with Python 3.6 and 3.7 */
    _PyConfig_INIT_COMPAT = 1,
    _PyConfig_INIT_PYTHON = 2,
    _PyConfig_INIT_ISOLATED = 3
} _PyConfigInitEnum;

PyAPI_FUNC(void) _PyConfig_InitCompatConfig(PyConfig *config);
extern PyStatus _PyConfig_Copy(
    PyConfig *config,
    const PyConfig *config2);
extern PyStatus _PyConfig_InitPathConfig(
    PyConfig *config,
    int compute_path_config);
extern PyStatus _PyConfig_InitImportConfig(PyConfig *config);
extern PyStatus _PyConfig_Read(PyConfig *config, int compute_path_config);
extern PyStatus _PyConfig_Write(const PyConfig *config,
    struct pyruntimestate *runtime);
extern PyStatus _PyConfig_SetPyArgv(
    PyConfig *config,
    const _PyArgv *args);

PyAPI_FUNC(PyObject*) _PyConfig_AsDict(const PyConfig *config);
PyAPI_FUNC(int) _PyConfig_FromDict(PyConfig *config, PyObject *dict);

extern void _Py_DumpPathConfig(PyThreadState *tstate);

PyAPI_FUNC(PyObject*) _Py_Get_Getpath_CodeObject(void);


/* --- Function used for testing ---------------------------------- */

PyAPI_FUNC(PyObject*) _Py_GetConfigsAsDict(void);

#ifdef __cplusplus
}
#endif
#endif /* !Py_INTERNAL_CORECONFIG_H */