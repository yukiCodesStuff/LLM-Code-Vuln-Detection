        1) {
        goto end;
    }
end:
    return rv;
}

/* Given a file-like object, this populates a ZoneInfo object
 *
 * The current version calls into a Python function to read the data from
 * file into Python objects, and this translates those Python objects into
 * C values and calculates derived values (e.g. dstoff) in C.
 *
 * This returns 0 on success and -1 on failure.
 *
 * The function will never return while `self` is partially initialized —
 * the object only needs to be freed / deallocated if this succeeds.
 */
static int
load_data(zoneinfo_state *state, PyZoneInfo_ZoneInfo *self, PyObject *file_obj)
{
    int rv = 0;
    PyObject *data_tuple = NULL;

    long *utcoff = NULL;
    long *dstoff = NULL;
    size_t *trans_idx = NULL;
    unsigned char *isdst = NULL;

    self->trans_list_utc = NULL;
    self->trans_list_wall[0] = NULL;
    self->trans_list_wall[1] = NULL;
    self->trans_ttinfos = NULL;
    self->_ttinfos = NULL;
    self->file_repr = NULL;

    size_t ttinfos_allocated = 0;

    data_tuple = PyObject_CallMethod(state->_common_mod, "load_data", "O",
                                     file_obj);

    if (data_tuple == NULL) {
        goto error;
    }

    if (!PyTuple_CheckExact(data_tuple)) {
        PyErr_Format(PyExc_TypeError, "Invalid data result type: %r",
                     data_tuple);
        goto error;
    }

    // Unpack the data tuple
    PyObject *trans_idx_list = PyTuple_GetItem(data_tuple, 0);
    if (trans_idx_list == NULL) {
        goto error;
    }

    PyObject *trans_utc = PyTuple_GetItem(data_tuple, 1);
    if (trans_utc == NULL) {
        goto error;
    }

    PyObject *utcoff_list = PyTuple_GetItem(data_tuple, 2);
    if (utcoff_list == NULL) {
        goto error;
    }

    PyObject *isdst_list = PyTuple_GetItem(data_tuple, 3);
    if (isdst_list == NULL) {
        goto error;
    }

    PyObject *abbr = PyTuple_GetItem(data_tuple, 4);
    if (abbr == NULL) {
        goto error;
    }

    PyObject *tz_str = PyTuple_GetItem(data_tuple, 5);
    if (tz_str == NULL) {
        goto error;
    }

    // Load the relevant sizes
    Py_ssize_t num_transitions = PyTuple_Size(trans_utc);
    if (num_transitions < 0) {
        goto error;
    }

    Py_ssize_t num_ttinfos = PyTuple_Size(utcoff_list);
    if (num_ttinfos < 0) {
        goto error;
    }

    self->num_transitions = (size_t)num_transitions;
    self->num_ttinfos = (size_t)num_ttinfos;

    // Load the transition indices and list
    self->trans_list_utc =
        PyMem_Malloc(self->num_transitions * sizeof(int64_t));
    if (self->trans_list_utc == NULL) {
        goto error;
    }
    trans_idx = PyMem_Malloc(self->num_transitions * sizeof(Py_ssize_t));
    if (trans_idx == NULL) {
        goto error;
    }

    for (size_t i = 0; i < self->num_transitions; ++i) {
        PyObject *num = PyTuple_GetItem(trans_utc, i);
        if (num == NULL) {
            goto error;
        }
        self->trans_list_utc[i] = PyLong_AsLongLong(num);
        if (self->trans_list_utc[i] == -1 && PyErr_Occurred()) {
            goto error;
        }

        num = PyTuple_GetItem(trans_idx_list, i);
        if (num == NULL) {
            goto error;
        }

        Py_ssize_t cur_trans_idx = PyLong_AsSsize_t(num);
        if (cur_trans_idx == -1) {
            goto error;
        }

        trans_idx[i] = (size_t)cur_trans_idx;
        if (trans_idx[i] > self->num_ttinfos) {
            PyErr_Format(
                PyExc_ValueError,
                "Invalid transition index found while reading TZif: %zd",
                cur_trans_idx);

            goto error;
        }
    }

    // Load UTC offsets and isdst (size num_ttinfos)
    utcoff = PyMem_Malloc(self->num_ttinfos * sizeof(long));
    isdst = PyMem_Malloc(self->num_ttinfos * sizeof(unsigned char));

    if (utcoff == NULL || isdst == NULL) {
        goto error;
    }
    for (size_t i = 0; i < self->num_ttinfos; ++i) {
        PyObject *num = PyTuple_GetItem(utcoff_list, i);
        if (num == NULL) {
            goto error;
        }

        utcoff[i] = PyLong_AsLong(num);
        if (utcoff[i] == -1 && PyErr_Occurred()) {
            goto error;
        }

        num = PyTuple_GetItem(isdst_list, i);
        if (num == NULL) {
            goto error;
        }

        int isdst_with_error = PyObject_IsTrue(num);
        if (isdst_with_error == -1) {
            goto error;
        }
        else {
            isdst[i] = (unsigned char)isdst_with_error;
        }
    }

    dstoff = PyMem_Calloc(self->num_ttinfos, sizeof(long));
    if (dstoff == NULL) {
        goto error;
    }

    // Derive dstoff and trans_list_wall from the information we've loaded
    utcoff_to_dstoff(trans_idx, utcoff, dstoff, isdst, self->num_transitions,
                     self->num_ttinfos);

    if (ts_to_local(trans_idx, self->trans_list_utc, utcoff,
                    self->trans_list_wall, self->num_ttinfos,
                    self->num_transitions)) {
        goto error;
    }

    // Build _ttinfo objects from utcoff, dstoff and abbr
    self->_ttinfos = PyMem_Malloc(self->num_ttinfos * sizeof(_ttinfo));
    if (self->_ttinfos == NULL) {
        goto error;
    }
    for (size_t i = 0; i < self->num_ttinfos; ++i) {
        PyObject *tzname = PyTuple_GetItem(abbr, i);
        if (tzname == NULL) {
            goto error;
        }

        ttinfos_allocated++;
        int rc = build_ttinfo(state, utcoff[i], dstoff[i], tzname,
                              &(self->_ttinfos[i]));
        if (rc) {
            goto error;
        }
    }

    // Build our mapping from transition to the ttinfo that applies
    self->trans_ttinfos =
        PyMem_Calloc(self->num_transitions, sizeof(_ttinfo *));
    if (self->trans_ttinfos == NULL) {
        goto error;
    }
    for (size_t i = 0; i < self->num_transitions; ++i) {
        size_t ttinfo_idx = trans_idx[i];
        assert(ttinfo_idx < self->num_ttinfos);
        self->trans_ttinfos[i] = &(self->_ttinfos[ttinfo_idx]);
    }

    // Set ttinfo_before to the first non-DST transition
    for (size_t i = 0; i < self->num_ttinfos; ++i) {
        if (!isdst[i]) {
            self->ttinfo_before = &(self->_ttinfos[i]);
            break;
        }
    }

    // If there are only DST ttinfos, pick the first one, if there are no
    // ttinfos at all, set ttinfo_before to NULL
    if (self->ttinfo_before == NULL && self->num_ttinfos > 0) {
        self->ttinfo_before = &(self->_ttinfos[0]);
    }

    if (tz_str != Py_None && PyObject_IsTrue(tz_str)) {
        if (parse_tz_str(state, tz_str, &(self->tzrule_after))) {
            goto error;
        }
    }
    else {
        if (!self->num_ttinfos) {
            PyErr_Format(PyExc_ValueError, "No time zone information found.");
            goto error;
        }

        size_t idx;
        if (!self->num_transitions) {
            idx = self->num_ttinfos - 1;
        }
        else {
            idx = trans_idx[self->num_transitions - 1];
        }

        _ttinfo *tti = &(self->_ttinfos[idx]);
        build_tzrule(state, tti->tzname, NULL, tti->utcoff_seconds, 0, NULL,
                     NULL, &(self->tzrule_after));

        // We've abused the build_tzrule constructor to construct an STD-only
        // rule mimicking whatever ttinfo we've picked up, but it's possible
        // that the one we've picked up is a DST zone, so we need to make sure
        // that the dstoff is set correctly in that case.
        if (PyObject_IsTrue(tti->dstoff)) {
            _ttinfo *tti_after = &(self->tzrule_after.std);
            Py_SETREF(tti_after->dstoff, Py_NewRef(tti->dstoff));
        }
    }

    // Determine if this is a "fixed offset" zone, meaning that the output of
    // the utcoffset, dst and tzname functions does not depend on the specific
    // datetime passed.
    //
    // We make three simplifying assumptions here:
    //
    // 1. If tzrule_after is not std_only, it has transitions that might occur
    //    (it is possible to construct TZ strings that specify STD and DST but
    //    no transitions ever occur, such as AAA0BBB,0/0,J365/25).
    // 2. If self->_ttinfos contains more than one _ttinfo object, the objects
    //    represent different offsets.
    // 3. self->ttinfos contains no unused _ttinfos (in which case an otherwise
    //    fixed-offset zone with extra _ttinfos defined may appear to *not* be
    //    a fixed offset zone).
    //
    // Violations to these assumptions would be fairly exotic, and exotic
    // zones should almost certainly not be used with datetime.time (the
    // only thing that would be affected by this).
    if (self->num_ttinfos > 1 || !self->tzrule_after.std_only) {
        self->fixed_offset = 0;
    }
    else if (self->num_ttinfos == 0) {
        self->fixed_offset = 1;
    }
    else {
        int constant_offset =
            ttinfo_eq(&(self->_ttinfos[0]), &self->tzrule_after.std);
        if (constant_offset < 0) {
            goto error;
        }
        else {
            self->fixed_offset = constant_offset;
        }
    }

    goto cleanup;
error:
    // These resources only need to be freed if we have failed, if we succeed
    // in initializing a PyZoneInfo_ZoneInfo object, we can rely on its dealloc
    // method to free the relevant resources.
    if (self->trans_list_utc != NULL) {
        PyMem_Free(self->trans_list_utc);
        self->trans_list_utc = NULL;
    }

    for (size_t i = 0; i < 2; ++i) {
        if (self->trans_list_wall[i] != NULL) {
            PyMem_Free(self->trans_list_wall[i]);
            self->trans_list_wall[i] = NULL;
        }
    }

    if (self->_ttinfos != NULL) {
        for (size_t i = 0; i < ttinfos_allocated; ++i) {
            xdecref_ttinfo(&(self->_ttinfos[i]));
        }
        PyMem_Free(self->_ttinfos);
        self->_ttinfos = NULL;
    }

    if (self->trans_ttinfos != NULL) {
        PyMem_Free(self->trans_ttinfos);
        self->trans_ttinfos = NULL;
    }

    rv = -1;
cleanup:
    Py_XDECREF(data_tuple);

    if (utcoff != NULL) {
        PyMem_Free(utcoff);
    }

    if (dstoff != NULL) {
        PyMem_Free(dstoff);
    }

    if (isdst != NULL) {
        PyMem_Free(isdst);
    }

    if (trans_idx != NULL) {
        PyMem_Free(trans_idx);
    }

    return rv;
}
        else {
            self->fixed_offset = constant_offset;
        }
    }

    goto cleanup;
error:
    // These resources only need to be freed if we have failed, if we succeed
    // in initializing a PyZoneInfo_ZoneInfo object, we can rely on its dealloc
    // method to free the relevant resources.
    if (self->trans_list_utc != NULL) {
        PyMem_Free(self->trans_list_utc);
        self->trans_list_utc = NULL;
    }

    for (size_t i = 0; i < 2; ++i) {
        if (self->trans_list_wall[i] != NULL) {
            PyMem_Free(self->trans_list_wall[i]);
            self->trans_list_wall[i] = NULL;
        }