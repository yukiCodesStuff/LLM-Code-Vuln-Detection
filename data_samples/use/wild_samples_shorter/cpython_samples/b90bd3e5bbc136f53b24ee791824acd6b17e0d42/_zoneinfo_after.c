static int
load_data(zoneinfo_state *state, PyZoneInfo_ZoneInfo *self, PyObject *file_obj)
{
    int rv = 0;
    PyObject *data_tuple = NULL;

    long *utcoff = NULL;
    long *dstoff = NULL;
        }
    }

    goto cleanup;
error:
    // These resources only need to be freed if we have failed, if we succeed
    // in initializing a PyZoneInfo_ZoneInfo object, we can rely on its dealloc