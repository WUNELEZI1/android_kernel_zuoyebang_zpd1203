// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#include <linux/preempt.h>
#include <mali_kbase.h>
#include <mali_kbase_xr_gpu_debug.h>

static u32 kbase_reg_read(struct kbase_device *kbdev, u32 offset)
{
	return readl(kbdev->reg + offset);
}

static void dump_iterator_registers(struct kbase_device *kbdev)
{
	if (!kbdev->pm.backend.gpu_powered)
		return;

	dev_err(kbdev->dev,
		"Compute  CTRL: %x STATUS: %x JASID: %u "
		"IRQ_RAW: %8x IRQ_STATUS: %8x "
		"EP_EVT_STATUS: %x BLOCKED_SB_ENTRY: %8x "
		"FAULT_STATUS %x FAULT_ADDR %llx",
		kbase_reg_read(kbdev, CSHW_IT_COMP_REG(0x0)),
		kbase_reg_read(kbdev, CSHW_IT_COMP_REG(0x4)),
		kbase_reg_read(kbdev, CSHW_IT_COMP_REG(0x8)),
		kbase_reg_read(kbdev, CSHW_IT_COMP_REG(0xD0)),
		kbase_reg_read(kbdev, CSHW_IT_COMP_REG(0xDC)),
		kbase_reg_read(kbdev, CSHW_IT_COMP_REG(0xA4)),
		kbase_reg_read(kbdev, CSHW_IT_COMP_REG(0xA0)),
		kbase_reg_read(kbdev, CSHW_IT_COMP_REG(0xE0)),
		kbase_reg_read(kbdev, CSHW_IT_COMP_REG(0xE8)) |
			((u64)kbase_reg_read(kbdev, CSHW_IT_COMP_REG(0xEC)) << 32));
	dev_err(kbdev->dev,
		"Fragment CTRL: %x STATUS: %x JASID: %u "
		"IRQ_RAW: %8x IRQ_STATUS: %8x "
		"EP_EVT_STATUS: %x BLOCKED_SB_ENTRY: %8x "
		"FAULT_STATUS %x FAULT_ADDR %llx",
		kbase_reg_read(kbdev, CSHW_IT_FRAG_REG(0x0)),
		kbase_reg_read(kbdev, CSHW_IT_FRAG_REG(0x4)),
		kbase_reg_read(kbdev, CSHW_IT_FRAG_REG(0x8)),
		kbase_reg_read(kbdev, CSHW_IT_FRAG_REG(0xD0)),
		kbase_reg_read(kbdev, CSHW_IT_FRAG_REG(0xDC)),
		kbase_reg_read(kbdev, CSHW_IT_FRAG_REG(0xA4)),
		kbase_reg_read(kbdev, CSHW_IT_FRAG_REG(0xA0)),
		kbase_reg_read(kbdev, CSHW_IT_FRAG_REG(0xE0)),
		kbase_reg_read(kbdev, CSHW_IT_FRAG_REG(0xE8)) |
			((u64)kbase_reg_read(kbdev, CSHW_IT_FRAG_REG(0xEC)) << 32));
	dev_err(kbdev->dev,
		"Tiler    CTRL: %x STATUS: %x JASID: %u "
		"IRQ_RAW: %8x IRQ_STATUS: %8x "
		"EP_EVT_STATUS: %x BLOCKED_SB_ENTRY: %8x "
		"FAULT_STATUS %x FAULT_ADDR %llx\n",
		kbase_reg_read(kbdev, CSHW_IT_TILER_REG(0x0)),
		kbase_reg_read(kbdev, CSHW_IT_TILER_REG(0x4)),
		kbase_reg_read(kbdev, CSHW_IT_TILER_REG(0x8)),
		kbase_reg_read(kbdev, CSHW_IT_TILER_REG(0xD0)),
		kbase_reg_read(kbdev, CSHW_IT_TILER_REG(0xDC)),
		kbase_reg_read(kbdev, CSHW_IT_TILER_REG(0xA4)),
		kbase_reg_read(kbdev, CSHW_IT_TILER_REG(0xA0)),
		kbase_reg_read(kbdev, CSHW_IT_TILER_REG(0xE0)),
		kbase_reg_read(kbdev, CSHW_IT_TILER_REG(0xE8)) |
			((u64)kbase_reg_read(kbdev, CSHW_IT_TILER_REG(0xEC)) << 32));
}

static void dump_cmd_ptr_instructions(struct kbase_context *kctx, u64 cmd_ptr)
{
	u64 address;

	dev_err(kctx->kbdev->dev, "Dumping instructions around the CMD_PTR, return");
	/* Start from 4 instructions back */
	for (address = cmd_ptr - (4 * sizeof(u64)); address < (cmd_ptr + (4 * sizeof(u64)));
	     address += sizeof(u64)) {
		struct kbase_vmap_struct mapping;
		u64 *ptr = kbase_vmap(kctx, address, sizeof(u64), &mapping);

		if (!ptr)
			continue;
		pr_err("0x%llx: %016llx\n", address, *ptr);
		kbase_vunmap(kctx, &mapping);
	}
}

static void dump_hwif_registers_pre_krake(struct kbase_context *kctx, unsigned int num_hw_ifs)
{
	struct kbase_device *kbdev = kctx->kbdev;
	unsigned int i;

	for (i = 0; i < num_hw_ifs; i++) {
		u64 cmd_ptr = kbase_reg_read(kbdev, CSHWIF_REG(i, 0x0)) |
			      ((u64)kbase_reg_read(kbdev, CSHWIF_REG(i, 0x4)) << 32);
		u64 cmd_ptr_end = kbase_reg_read(kbdev, CSHWIF_REG(i, 0x8)) |
				  ((u64)kbase_reg_read(kbdev, CSHWIF_REG(i, 0xC)) << 32);
		int as_nr = kbase_reg_read(kbdev, CSHWIF_REG(i, 0x34));

		if (!cmd_ptr)
			continue;

		dev_err(kbdev->dev, "Register dump of CSHWIF %d", i);
		dev_err(kbdev->dev,
			"CMD_PTR: %llx CMD_PTR_END: %llx "
			"STATUS: %x JASID: %x EMUL_INSTR: %llx WAIT_STATUS: %x "
			"SB_SET_SEL: %x SB_SEL: %x",
			cmd_ptr, cmd_ptr_end, kbase_reg_read(kbdev, CSHWIF_REG(i, 0x24)),
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0x34)),
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0x60)) |
				((u64)kbase_reg_read(kbdev, CSHWIF_REG(i, 0x64)) << 32),
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0x74)),
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0x78)),
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0x7C)));
		dev_err(kbdev->dev,
			"CMD_COUNTER: %x EVT_RAW: %x "
			"EVT_IRQ_STATUS: %x EVT_HALT_STATUS: %x "
			"FAULT_STATUS: %x FAULT_ADDR: %llx",
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0x80)),
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0x98)),
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0xA4)),
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0xAC)),
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0xB0)),
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0xB8)) |
				((u64)kbase_reg_read(kbdev, CSHWIF_REG(i, 0xBC)) << 32));
		dev_err(kbdev->dev, "ITER_COMPUTE: %x ITER_FRAGMENT: %x ITER_TILER: %x",
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0x28)),
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0x2C)),
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0x30)));
		dev_err(kbdev->dev, "\n");

		if ((cmd_ptr != cmd_ptr_end) && as_nr > 0) {
			if (kctx == kbdev->as_to_kctx[as_nr])
				dump_cmd_ptr_instructions(kctx, cmd_ptr);
		}
	}
}

static void dump_hwif_registers_krake(struct kbase_context *kctx, unsigned int num_hw_ifs)
{
	struct kbase_device *kbdev = kctx->kbdev;
	unsigned int i;

	for (i = 0; i < num_hw_ifs; i++) {
		u64 rb_start_ptr = kbase_reg_read(kbdev, CSHWIF_REG(i, 0x100)) |
				   ((u64)kbase_reg_read(kbdev, CSHWIF_REG(i, 0x104)) << 32);
		u64 rb_end_ptr, rb_read_ptr, rb_write_ptr, lb_start_ptr, lb_end_ptr, lb_read_ptr;
		u32 config, cb_stack, call_depth, jasid;

		if (!rb_start_ptr)
			continue;

		rb_end_ptr = kbase_reg_read(kbdev, CSHWIF_REG(i, 0x108)) |
			     ((u64)kbase_reg_read(kbdev, CSHWIF_REG(i, 0x10C)) << 32);
		rb_read_ptr = kbase_reg_read(kbdev, CSHWIF_REG(i, 0x110)) |
			      ((u64)kbase_reg_read(kbdev, CSHWIF_REG(i, 0x114)) << 32);
		rb_write_ptr = kbase_reg_read(kbdev, CSHWIF_REG(i, 0x118)) |
			       ((u64)kbase_reg_read(kbdev, CSHWIF_REG(i, 0x11C)) << 32);
		lb_start_ptr = kbase_reg_read(kbdev, CSHWIF_REG(i, 0x130)) |
			       ((u64)kbase_reg_read(kbdev, CSHWIF_REG(i, 0x134)) << 32);
		lb_end_ptr = kbase_reg_read(kbdev, CSHWIF_REG(i, 0x138)) |
			     ((u64)kbase_reg_read(kbdev, CSHWIF_REG(i, 0x13C)) << 32);
		lb_read_ptr = kbase_reg_read(kbdev, CSHWIF_REG(i, 0x140)) |
			      ((u64)kbase_reg_read(kbdev, CSHWIF_REG(i, 0x144)) << 32);
		cb_stack = kbase_reg_read(kbdev, CSHWIF_REG(i, 0x14C));
		config = kbase_reg_read(kbdev, CSHWIF_REG(i, 0x34));

		dev_err(kbdev->dev, "Register dump of CSHWIF %d", i);
		dev_err(kbdev->dev,
			"RB_START: %llx RB_END: %llx RB_READ: %llx RB_WRITE %llx "
			"LB_START: %llx LB_END: %llx LB_READ: %llx "
			"CB_STACK %x CONFIG(JASID) %x",
			rb_start_ptr, rb_end_ptr, rb_read_ptr, rb_write_ptr, lb_start_ptr,
			lb_end_ptr, lb_read_ptr, cb_stack, config);
		dev_err(kbdev->dev,
			"STATUS: %x EMUL_INSTR: %llx WAIT_STATUS: %x "
			"SB_SET_SEL: %x SB_SEL: %x "
			"PREFETCH_BUF_CFG %x PREFETCH_BUF_STATUS %x",
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0x24)),
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0x60)) |
				((u64)kbase_reg_read(kbdev, CSHWIF_REG(i, 0x64)) << 32),
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0x74)),
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0x78)),
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0x7C)),
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0x14)),
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0x1C)));
		dev_err(kbdev->dev,
			"CMD_COUNTER: %x EVT_RAW: %x EVT_IRQ_STATUS: %x EVT_HALT_STATUS: %x "
			"FAULT_STATUS: %x FAULT_ADDR: %llx",
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0x80)),
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0x98)),
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0xA4)),
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0xAC)),
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0xB0)),
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0xB8)) |
				((u64)kbase_reg_read(kbdev, CSHWIF_REG(i, 0xBC)) << 32));
		dev_err(kbdev->dev, "ITER_COMPUTE: %x ITER_FRAGMENT: %x ITER_TILER: %x",
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0x28)),
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0x2C)),
			kbase_reg_read(kbdev, CSHWIF_REG(i, 0x30)));
		dev_err(kbdev->dev, "\n");

		jasid = CSHWIF_CONFIG_JASID_GET(config);
		call_depth = CSHWIF_CB_STACK_CURRENT_GET(cb_stack);
		if (call_depth && jasid) {
			if (kctx == kbdev->as_to_kctx[jasid])
				dump_cmd_ptr_instructions(kctx, lb_read_ptr);
		}
	}
}

static void dump_hwif_registers(struct kbase_context *kctx)
{
	struct kbase_device *kbdev = kctx->kbdev;
	unsigned int num_hw_ifs;

	if (!kbdev->pm.backend.gpu_powered)
		return;

	num_hw_ifs = CSHW_FEATURES_NUM_CSHWIF_GET(kbase_reg_read(kbdev, CSHW_BASE + CSHW_FEATURES));
	dev_err(kbdev->dev, "Dumping data of %u HWIFs", num_hw_ifs);

	if (num_hw_ifs > 4)
		dump_hwif_registers_krake(kctx, num_hw_ifs);
	else
		dump_hwif_registers_pre_krake(kctx, num_hw_ifs);
}

static void print_group_queues_data(struct kbase_queue_group *const group)
{
	u64 insert[5];
	u64 extract[5];
	u64 ringbuff[5];
	unsigned int i;

	if (!group)
		return;

	for (i = 0; i < 5; i++) {
		struct kbase_queue *queue = group->bound_queues[i];
		if (queue) {
			u64 *input_addr = queue->user_io_addr;
			u64 *output_addr = queue->user_io_addr + PAGE_SIZE / sizeof(u64);

			insert[i] = input_addr[CS_INSERT_LO / sizeof(u64)];
			extract[i] = output_addr[CS_EXTRACT_LO / sizeof(u64)];
			ringbuff[i] = queue->base_addr;
		} else {
			insert[i] = 0;
			extract[i] = 0;
			ringbuff[i] = 0;
		}
	}

	dev_warn(
		group->kctx->kbdev->dev,
		"R0 %llx I0 %llx E0 %llx,"
		"R1 %llx I1 %llx E1 %llx,"
		"R2 %llx I2 %llx E2 %llx,"
		"R3 %llx I3 %llx E3 %llx,"
		"R4 %llx I4 %llx E4 %llx",
		ringbuff[0], insert[0], extract[0],
		ringbuff[1], insert[1], extract[1],
		ringbuff[2], insert[2], extract[2],
		ringbuff[3], insert[3], extract[3],
		ringbuff[4], insert[4], extract[4]);

		if (in_interrupt())
			return;

	for (i = 0; i < 5; i++) {
		struct kbase_queue *queue = group->bound_queues[i];
		u64 cs_insert = insert[i];
		u64 cs_extract = extract[i];
		const unsigned int instruction_size = sizeof(u64);
		size_t size_mask = (queue->queue_reg->nr_pages << PAGE_SHIFT) - 1;
		u64 start, stop;

		if (cs_insert == cs_extract)
			continue;

		cs_extract = ALIGN_DOWN(cs_extract, 8 * instruction_size);

		/* Go 32 instructions back */
		if (cs_extract > (32 * instruction_size))
			start = cs_extract - (32 * instruction_size);
		else
			start = 0;

		/* Print upto 64 instructions */
		stop = start + (64 * instruction_size);
		if (stop > cs_insert)
			stop = cs_insert;

		pr_err("\nQueue %u: Instructions from Extract offset %llx\n", i, start);
		while (start != stop) {
			u64 page_off = (start & size_mask) >> PAGE_SHIFT;
			u64 offset = (start & size_mask) & ~PAGE_MASK;
			struct page *page = as_page(queue->queue_reg->gpu_alloc->pages[page_off]);
			u64 *ringbuffer = vmap(&page, 1, VM_MAP, pgprot_noncached(PAGE_KERNEL));
			u64 *ptr = &ringbuffer[offset / 8];

			pr_err("%016llx %016llx %016llx %016llx %016llx %016llx %016llx %016llx\n",
			       ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7]);

			vunmap(ringbuffer);
			start += (8 * instruction_size);
		}
	}
}

void kbase_csf_dump_kctx_debug_info(struct kbase_context *kctx)
{
	struct kbase_device *kbdev = kctx->kbdev;
	struct kbasep_printer *kbpr;
	u32 num_groups = kbdev->csf.global_iface.group_num;
	u32 csg_nr;

	if (kbdev->pm.backend.gpu_powered) {
		dev_err(kbdev->dev, "GPU_STATUS=%x MCU status=%x",
			kbase_reg_read32(kbdev, GPU_CONTROL_ENUM(GPU_STATUS)),
			kbase_reg_read32(kbdev, GPU_CONTROL_ENUM(MCU_STATUS)));
		dev_err(kbdev->dev, "SHADER_READY=%llx SHADER_PWRACTIVE=%llx SHADER_PWRTRANS=%llx",
			kbase_reg_read64(kbdev, GPU_CONTROL_ENUM(SHADER_READY)),
			kbase_reg_read64(kbdev, GPU_CONTROL_ENUM(SHADER_PWRACTIVE)),
			kbase_reg_read64(kbdev, GPU_CONTROL_ENUM(SHADER_PWRTRANS)));

		dump_hwif_registers(kctx);
		dump_iterator_registers(kbdev);
	}

	for (csg_nr = 0; csg_nr < num_groups; csg_nr++) {
		struct kbase_queue_group *const group =
			kbdev->csf.scheduler.csg_slots[csg_nr].resident_group;

		if (!group || (group->kctx != kctx))
			continue;

		dev_err(kbdev->dev,
			"Dumping data for group %d of ctx %d_%d (%s) on slot %d in run_state %d",
			group->handle, group->kctx->tgid, group->kctx->id, group->kctx->comm,
			group->csg_nr, group->run_state);
		print_group_queues_data(group);
	}

	kbpr = kbasep_printer_buffer_init(kbdev, KBASEP_PRINT_TYPE_DEV_WARN);
	if (kbpr) {
		kbasep_csf_csg_active_dump_print(kctx->kbdev, kbpr);
		kbasep_csf_csg_dump_print(kctx, kbpr);
		kbasep_csf_sync_gpu_dump_print(kctx, kbpr);
		kbasep_csf_sync_kcpu_dump_print(kctx, kbpr);
		kbasep_csf_cpu_queue_dump_print(kctx, kbpr);
		kbasep_printer_buffer_flush(kbpr);
		kbasep_printer_term(kbpr);
	}
}
