		state->flags = 0;
		switch (cx->type) {
			case ACPI_STATE_C1:
			if (cx->entry_method != ACPI_CSTATE_FFH)
				state->flags |= CPUIDLE_FLAG_TIME_INVALID;

			state->enter = acpi_idle_enter_c1;
			state->enter_dead = acpi_idle_play_dead;
			drv->safe_state_index = count;