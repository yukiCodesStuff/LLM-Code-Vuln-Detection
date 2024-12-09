			 struct amdgpu_task_info *task_info)
{
	struct amdgpu_vm *vm;

	spin_lock(&adev->vm_manager.pasid_lock);

	vm = idr_find(&adev->vm_manager.pasid_idr, pasid);
	if (vm)
		*task_info = vm->task_info;

	spin_unlock(&adev->vm_manager.pasid_lock);
}

/**
 * amdgpu_vm_set_task_info - Sets VMs task info.