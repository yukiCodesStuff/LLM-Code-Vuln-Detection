		switch (cx->type) {
			case ACPI_STATE_C1:

			state->enter = acpi_idle_enter_c1;
			state->enter_dead = acpi_idle_play_dead;
			drv->safe_state_index = count;
			break;

			case ACPI_STATE_C2:
			state->enter = acpi_idle_enter_simple;
			state->enter_dead = acpi_idle_play_dead;
			drv->safe_state_index = count;
			break;

			case ACPI_STATE_C3:
			state->enter = pr->flags.bm_check ?
					acpi_idle_enter_bm :
					acpi_idle_enter_simple;
			break;
		}