			goto error;
		}

		fput(interpreter);
		interpreter = NULL;
	}

	retval = 0;

error:
	if (interpreter)
		fput(interpreter);
	kfree(interpreter_name);
	kfree(exec_params.phdrs);
	kfree(exec_params.loadmap);
	kfree(interp_params.phdrs);