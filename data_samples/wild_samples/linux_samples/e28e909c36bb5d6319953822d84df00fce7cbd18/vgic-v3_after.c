/*
 * Copyright (C) 2013 ARM Limited, All Rights Reserved.
 * Author: Marc Zyngier <marc.zyngier@arm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/cpu.h>
#include <linux/kvm.h>
#include <linux/kvm_host.h>
#include <linux/interrupt.h>
#include <linux/io.h>

#include <linux/irqchip/arm-gic-v3.h>
#include <linux/irqchip/arm-gic-common.h>

#include <asm/kvm_emulate.h>
#include <asm/kvm_arm.h>
#include <asm/kvm_asm.h>
#include <asm/kvm_mmu.h>

static u32 ich_vtr_el2;

static struct vgic_lr vgic_v3_get_lr(const struct kvm_vcpu *vcpu, int lr)
{
	struct vgic_lr lr_desc;
	u64 val = vcpu->arch.vgic_cpu.vgic_v3.vgic_lr[lr];

	if (vcpu->kvm->arch.vgic.vgic_model == KVM_DEV_TYPE_ARM_VGIC_V3)
		lr_desc.irq = val & ICH_LR_VIRTUAL_ID_MASK;
	else
		lr_desc.irq = val & GICH_LR_VIRTUALID;

	lr_desc.source = 0;
	if (lr_desc.irq <= 15 &&
	    vcpu->kvm->arch.vgic.vgic_model == KVM_DEV_TYPE_ARM_VGIC_V2)
		lr_desc.source = (val >> GICH_LR_PHYSID_CPUID_SHIFT) & 0x7;

	lr_desc.state = 0;

	if (val & ICH_LR_PENDING_BIT)
		lr_desc.state |= LR_STATE_PENDING;
	if (val & ICH_LR_ACTIVE_BIT)
		lr_desc.state |= LR_STATE_ACTIVE;
	if (val & ICH_LR_EOI)
		lr_desc.state |= LR_EOI_INT;
	if (val & ICH_LR_HW) {
		lr_desc.state |= LR_HW;
		lr_desc.hwirq = (val >> ICH_LR_PHYS_ID_SHIFT) & GENMASK(9, 0);
	}

	return lr_desc;
}
{
	struct vgic_lr lr_desc;
	u64 val = vcpu->arch.vgic_cpu.vgic_v3.vgic_lr[lr];

	if (vcpu->kvm->arch.vgic.vgic_model == KVM_DEV_TYPE_ARM_VGIC_V3)
		lr_desc.irq = val & ICH_LR_VIRTUAL_ID_MASK;
	else
		lr_desc.irq = val & GICH_LR_VIRTUALID;

	lr_desc.source = 0;
	if (lr_desc.irq <= 15 &&
	    vcpu->kvm->arch.vgic.vgic_model == KVM_DEV_TYPE_ARM_VGIC_V2)
		lr_desc.source = (val >> GICH_LR_PHYSID_CPUID_SHIFT) & 0x7;

	lr_desc.state = 0;

	if (val & ICH_LR_PENDING_BIT)
		lr_desc.state |= LR_STATE_PENDING;
	if (val & ICH_LR_ACTIVE_BIT)
		lr_desc.state |= LR_STATE_ACTIVE;
	if (val & ICH_LR_EOI)
		lr_desc.state |= LR_EOI_INT;
	if (val & ICH_LR_HW) {
		lr_desc.state |= LR_HW;
		lr_desc.hwirq = (val >> ICH_LR_PHYS_ID_SHIFT) & GENMASK(9, 0);
	}