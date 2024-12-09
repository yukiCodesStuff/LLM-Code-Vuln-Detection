
PyAPI_FUNC(PyObject*) _Py_Get_Getpath_CodeObject(void);

extern int _Py_global_config_int_max_str_digits;  // TODO(gpshead): move this into PyConfig in 3.12 after the backports ship.


/* --- Function used for testing ---------------------------------- */

PyAPI_FUNC(PyObject*) _Py_GetConfigsAsDict(void);