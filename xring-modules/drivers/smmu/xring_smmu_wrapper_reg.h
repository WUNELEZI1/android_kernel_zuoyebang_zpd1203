/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __XRING_SMMU_WRAPPER_REG_H__
#define __XRING_SMMU_WRAPPER_REG_H__

#include <linux/mutex.h>

/*
 * TCU register
 */
#define ARM_SMMU_TCU_IRPT_CLEAR			0x00
#define SC_SMMU_TCU_GERROR_IRPT_CLR		(1 << 4)
#define SC_SMMU_TCU_CMDQ_SYNC_IRPT_CLR		(1 << 3)
#define SC_SMMU_TCU_EVENTQ_IRPT_CLR		(1 << 2)
#define SC_SMMU_TCU_PMU_IRPT_CLR		(1 << 1)
#define SC_SMMU_TCU_RAS_IRPT_CLR		(1 << 0)

#define ARM_SMMU_TCU_IRPT_STATE			0x04
#define SC_SMMU_TCU_GERROR_IRPT_ST		(1 << 4)
#define SC_SMMU_TCU_CMDQ_SYNC_IRPT_ST		(1 << 3)
#define SC_SMMU_TCU_EVENTQ_IRPT_ST		(1 << 2)
#define SC_SMMU_TCU_RAS_IRPT_ST			(1 << 1)
#define SC_SMMU_TCU_PMU_IRPT_ST			(1 << 0)

#define ARM_SMMU_TCU_QCH_CG_CTRL		0x08
#define SC_SMMU_TCU_QCH_CG_CTRL_EN		(1 << 17)
#define SC_SMMU_TCU_QREQN_CG_SET		0x10100
#define SC_SMMU_TCU_QACTIVE_CG_THRESH(n)	((n) << 0)
#define SC_SMMU_TCU_QACTIVE_CG_THRESH_MASK	(0xFFFF << 0)

#define ARM_SMMU_TCU_QCH_CG_STATE		0x0C
#define SC_SMMU_TCU_QREQN_CG_ST			(1 << 8)
#define SC_SMMU_TCU_QACCEPTN_CG_ST		(1 << 7)
#define SC_SMMU_TCU_QDENY_CG_ST			(1 << 6)
#define SC_SMMU_TCU_QACTIVE_CG_ST		(1 << 5)
#define SC_SMMU_TCU_QCTRL_CLKEN_ST		(1 << 4)
#define SC_SMMU_TCU_QCH_CG_STATE(n)		((n) << 2)
#define SC_SMMU_TCU_QCH_CG_STATE_MASK		(0x3 << 2)
#define SC_SMMU_TCU_QCH_CG_STATE_NXT(n)		((n) << 0)
#define SC_SMMU_TCU_QCH_CG_STATE_NXT_MASK	(0x3 << 0)

#define ARM_SMMU_TCU_QCH_PD_CTRL		0x10
#define SC_SMMU_TCU_QREQN_PD			(1 << 0)

#define ARM_SMMU_TCU_QCH_PD_STATE		0x14
#define SC_SMMU_TCU_QACCEPTN_PD			(1 << 2)
#define SC_SMMU_TCU_QDENY_PD			(1 << 1)
#define SC_SMMU_TCU_QACATIVE_PD			(1 << 0)

#define ARM_SMMU_TCU_PMU_SNAP_REQ		0x18

#define ARM_SMMU_TCU_PMU_SNAP_ACK		0x1C

/*
 * TBU register
 */
#define ARM_SMMU_TBU_IRPT_CLEAR			0x00
#define SC_SMMU_TBU_PMU_IRPT_CLR		(1 << 1)
#define SC_SMMU_TBU_RAS_IRPT_CLR		(1 << 0)

#define ARM_SMMU_TBU_IRPT_STATE			0x04
#define SC_SMMU_TBU_PMU_IRPT_ST			(1 << 1)
#define SC_SMMU_TBU_RAS_IRPT_ST			(1 << 0)

#define ARM_SMMU_TBU_TIEOFF_CFG			0x08

#define ARM_SMMU_TBU_QCH_CG_CTRL		0x0C
#define SC_SMMU_TBU_QCH_CG_CTRL_EN		(1 << 17)
#define SC_SMMU_TBU_QREQN_CG_SET		0x10100
#define SC_SMMU_TBU_QACTIVE_CG_THRESH(n)	((n) << 0)
#define SC_SMMU_TBU_QACTIVE_CG_THRESH_MASK	(0xFFFF << 0)

#define ARM_SMMU_TBU_QCH_CG_STATE		0x10
#define SC_SMMU_TBU_QREQN_CG_ST			(1 << 8)
#define SC_SMMU_TBU_QACCEPTN_CG_ST		(1 << 7)
#define SC_SMMU_TBU_QDENY_CG_ST			(1 << 6)
#define SC_SMMU_TBU_QACTIVE_CG_ST		(1 << 5)
#define SC_SMMU_TBU_QCTRL_CLKEN_ST		(1 << 4)
#define SC_SMMU_TBU_QCH_CG_STATE(n)		((n) << 2)
#define SC_SMMU_TBU_QCH_CG_STATE_MASK		(0x3 << 2)
#define SC_SMMU_TBU_QCH_CG_STATE_NXT(n)		((n) << 0)
#define SC_SMMU_TBU_QCH_CG_STATE_NXT_MASK	(0x3 << 0)

#define ARM_SMMU_TBU_QCH_PD_CTRL		0x14
#define SC_SMMU_TBU_QREQN_PD			(1 << 0)

#define ARM_SMMU_TBU_QCH_PD_STATE		0x18
#define SC_SMMU_TBU_QACCEPTN_PD			(1 << 2)
#define SC_SMMU_TBU_QDENY_PD			(1 << 1)
#define SC_SMMU_TBU_QACATIVE_PD			(1 << 0)

#define ARM_SMMU_TBU_PMU_SNAP_REQ		0x1C

#define ARM_SMMU_TBU_PMU_SNAP_ACK		0x20

/*
 * NS SID SSID register
 */
//ISP
#define ARM_SMMU_SC_ISP_NS_SID0			0x00
#define ARM_SMMU_SC_ISP_NS_SSID0		0x04
#define ARM_SMMU_SC_ISP_NS_SSIDV0		0x08

#define ARM_SMMU_SC_ISP_NS_SID1			0x0C
#define ARM_SMMU_SC_ISP_NS_SSID1		0x10
#define ARM_SMMU_SC_ISP_NS_SSIDV1		0x14

#define ARM_SMMU_SC_ISP_NS_SID2			0x18
#define ARM_SMMU_SC_ISP_NS_SSID2		0x1C
#define ARM_SMMU_SC_ISP_NS_SSIDV2		0x20

//VENC
#define ARM_SMMU_SC_VENC_NS_SID0		0x80
#define ARM_SMMU_SC_VENC_NS_SSID0		0x84
#define ARM_SMMU_SC_VENC_NS_SSIDV0		0x88

#define ARM_SMMU_SC_VENC_NS_SID1		0x8C
#define ARM_SMMU_SC_VENC_NS_SSID1		0x90
#define ARM_SMMU_SC_VENC_NS_SSIDV1		0x94

#define ARM_SMMU_SC_VENC_NS_SID2		0x98
#define ARM_SMMU_SC_VENC_NS_SSID2		0x9C
#define ARM_SMMU_SC_VENC_NS_SSIDV2		0xA0

//VDEC
#define ARM_SMMU_SC_VDEC_NS_SID0		0x00
#define ARM_SMMU_SC_VDEC_NS_SSID0		0x04
#define ARM_SMMU_SC_VDEC_NS_SSIDV0		0x08

#define ARM_SMMU_SC_VDEC_NS_SID1		0x0C
#define ARM_SMMU_SC_VDEC_NS_SSID1		0x10
#define ARM_SMMU_SC_VDEC_NS_SSIDV1		0x14

//PCIE
#define ARM_SMMU_SC_PCIE_NS_SID0		0x58
#define ARM_SMMU_SC_PCIE_NS_SID1		0x5C


struct xring_smmu_tcu {
	char name[32];
	u64 start;
	u64 len;
	void __iomem *base;
	int ras_irq;
};

struct xring_smmu_tbu {
	char name[32];
	u64 start;
	u64 len;
	void __iomem *base;
	int ras_irq;
	bool use;
};

struct xring_smmu_id {
	char name[32];
	u64 start;
	u64 len;
	void __iomem *base;
};

#define XRING_SMMU_TBU_MAX_NUMBER 10
struct xring_smmu_wrapper {
	char name[32];
	u64 tbu_linkup_num;
	struct mutex mutex;
	struct xring_smmu_id id;
	struct xring_smmu_tcu tcu;
	struct xring_smmu_tbu tbu[XRING_SMMU_TBU_MAX_NUMBER];
	struct list_head list;
	struct arm_smmu_device *smmu;
};

#endif

