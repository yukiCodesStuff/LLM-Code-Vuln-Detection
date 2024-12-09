{
	struct amdgpu_vm *vm;

	spin_lock(&adev->vm_manager.pasid_lock);

	vm = idr_find(&adev->vm_manager.pasid_idr, pasid);
	if (vm)
		*task_info = vm->task_info;

	spin_unlock(&adev->vm_manager.pasid_lock);
}