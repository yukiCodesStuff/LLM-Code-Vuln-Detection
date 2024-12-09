			goto error;
		}

		allow_write_access(interpreter);
		fput(interpreter);
		interpreter = NULL;
	}

	retval = 0;

error:
	if (interpreter) {
		allow_write_access(interpreter);
		fput(interpreter);
	}
	kfree(interpreter_name);
	kfree(exec_params.phdrs);
	kfree(exec_params.loadmap);
	kfree(interp_params.phdrs);