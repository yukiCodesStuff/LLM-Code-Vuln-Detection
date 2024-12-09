{
	struct amdgpu_vm *vm;
	unsigned long flags;

	spin_lock_irqsave(&adev->vm_manager.pasid_lock, flags);

	vm = idr_find(&adev->vm_manager.pasid_idr, pasid);
	if (vm)
		*task_info = vm->task_info;

	spin_unlock_irqrestore(&adev->vm_manager.pasid_lock, flags);
}