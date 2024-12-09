{
    int flag;
    mallopt(M_DEBUG, flag);
    Py_RETURN_NONE;
}
#endif /* USE_MALLOPT */


/*[clinic input]
sys.get_int_max_str_digits

Set the maximum string digits limit for non-binary int<->str conversions.
[clinic start generated code]*/

static PyObject *
sys_get_int_max_str_digits_impl(PyObject *module)
/*[clinic end generated code: output=0042f5e8ae0e8631 input=8dab13e2023e60d5]*/
{
    PyInterpreterState *interp = _PyInterpreterState_GET();
    return PyLong_FromLong(interp->long_state.max_str_digits);
}