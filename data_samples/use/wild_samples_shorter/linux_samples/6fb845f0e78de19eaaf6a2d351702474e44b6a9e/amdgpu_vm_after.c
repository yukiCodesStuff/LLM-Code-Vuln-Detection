			 struct amdgpu_task_info *task_info)
{
	struct amdgpu_vm *vm;
	unsigned long flags;

	spin_lock_irqsave(&adev->vm_manager.pasid_lock, flags);

	vm = idr_find(&adev->vm_manager.pasid_idr, pasid);
	if (vm)
		*task_info = vm->task_info;

	spin_unlock_irqrestore(&adev->vm_manager.pasid_lock, flags);
}

/**
 * amdgpu_vm_set_task_info - Sets VMs task info.