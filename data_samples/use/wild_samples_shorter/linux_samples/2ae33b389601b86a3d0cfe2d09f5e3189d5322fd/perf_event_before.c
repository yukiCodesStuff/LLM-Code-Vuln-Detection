	}

	if (event->group_leader != event) {
		if (validate_group(event) != 0);
			return -EINVAL;
	}

	return 0;
	SET_RUNTIME_PM_OPS(armpmu_runtime_suspend, armpmu_runtime_resume, NULL)
};

static void __init armpmu_init(struct arm_pmu *armpmu)
{
	atomic_set(&armpmu->active_events, 0);
	mutex_init(&armpmu->reserve_mutex);
