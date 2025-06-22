// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_device.h>
#include <linux/module.h>
#include <linux/mailbox_controller.h>
#include <linux/cpumask.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/gfp.h>
#include <linux/kernel.h>

/*********************mbox basic param*******************/
#define MBOX_CHAN_MAX			32
#define MBOX_TX_CHAN_START		10
#define MBOX_CHAN_END			13
#define MBOX_START				0
#define MBOX_TX					0x01
#define MBOX_LAST_TXDONE		0
#define MBOX_IRQ_CHECK_START	2
/*********************mbox trans param*******************/
#define MBOX_MSG_LEN			14
#define MBOX_DATA_REG_LEN       6
#define MBOX_NDATA_REG_LEN      8
#define MODE_AUTO_ACK	1
#define MODE_AUTO_LINK			(1 << 1)
/* Mailbox Send Register states*/
#define MBOX_SEND_INACTIVE	0
#define MBOX_SEND_ACK       2
#define MBOX_SEND_INVALID	0x3

#define MBOX_SOURCE_CLEAR	0
#define MBOX_SEND  (0x1)

#define IPCM_SR(n)             (0x00 + (n)*0x40)
#define IPCM_DEST(n)           (0x04 + (n)*0x40)
#define IPCM_DCLR(n)           (0x08 + (n)*0x40)
#define IPCM_DSTATUS(n)        (0x0C + (n)*0x40)
#define IPCM_MODE(n)           (0x10 + (n)*0x40)
#define IPCM_MSET(n)           (0x14 + (n)*0x40)
#define IPCM_MCLR(n)           (0x18 + (n)*0x40)
#define IPCM_MSTATUS(n)        (0x1C + (n)*0x40)
#define IPCM_SEND(n)           (0x20 + (n)*0x40)
#define IPCM_DAT(n, i)         (0x24 + (n)*0x40 + (i)*0x4)
#define IPCM_MIS(n)            (0x800 + (n)*0x8)
#define IPCM_RIS(n)            (0x804 + (n)*0x8)
#define IPCM_NDAT(n, i)        (0xA00 + (n)*0x20 + (i)*0x4)

#define hpxerr(fmt, ...)         \
	pr_err("[%s][E]: " fmt, __func__, ##__VA_ARGS__)

#define hpxwarn(fmt, ...)        \
	pr_warn("[%s][W]: " fmt, __func__, ##__VA_ARGS__)

#define hpxinfo(fmt, ...)        \
	pr_info("[%s][I]: " fmt, __func__, ##__VA_ARGS__)

#define hpxdbg(fmt, ...)        \
	pr_debug("[%s][D]: " fmt, __func__, ##__VA_ARGS__)

static char *irq_name[4] = {"irq-map-chan-rpmsg", "irq-map-chan-resv0", "irq-map-chan-resv1", "irq-map-chan-xrp"};
static char *irq_num_name = "irq-num";

enum mbox_trans_dir_e {
	HPC_MBOX_TX = 0UL,
	HPC_MBOX_RX,
};

struct hpc_mbox_chan {
	/*
	 * Description for channel's hardware info:
	 * direction: tx or rx
	 * slot: channel id
	 * dst irq: IRQ identifier index number which used by MCU (dst->source channel id)
	 * ack irq: IRQ identifier index number with generating a
	 * TX/RX interrupt to application processor(equals to channel id)
	 * slot number
	 */
	unsigned int dir, dst_irq, ack_irq;

	//first_id-slot;
	unsigned int first_id;
	struct hpc_mbox *parent;
};

struct hpc_mbox {
	struct platform_device *pdev;
	struct device *dev;
	/* flag of enabling tx's irq mode */
	bool tx_irq_mode;
	/* region for mailbox */
	void __iomem *base;
	struct resource *reg;

	unsigned int chan_num;
	struct hpc_mbox_chan *mchan;
	void *irq_map_chan[MBOX_CHAN_MAX];
	struct mbox_chan *chan;
	struct mbox_controller controller;

	/*
	 * Description: irq info, indicate the irq include the channel
	 * irq_num：indicate the irq's num
	 * irq_array：store the irq num
	 * irq_inc_chan：indicate the irq include the chan
	 */

	int irq_num;
	int *irq_array;
	int **irq_inc_chan;
};

static void hpc_read_msg(u32 *message, struct hpc_mbox *mbox, int recv_chid)
{
	u32 len;
	int i;

	len = readl_relaxed(mbox->base + IPCM_DAT(recv_chid, 0));
	len = min_t(u32, len, MBOX_MSG_LEN);

	if (len <= MBOX_DATA_REG_LEN) {
		for (i = 0; i < len; i++) {
			message[i] = readl_relaxed(mbox->base +
				IPCM_DAT(recv_chid, i + 1));
		}
		return;
	}

	for (i = 0; i < MBOX_DATA_REG_LEN; i++) {
		message[i] = readl_relaxed(mbox->base +
			IPCM_DAT(recv_chid, i + 1));
	}

	for (i = 0; i < len - MBOX_DATA_REG_LEN; i++) {
		message[i + MBOX_DATA_REG_LEN] = readl_relaxed(mbox->base +
				IPCM_NDAT(recv_chid, i));
	}

}

static void hpc_mbox_set_state(struct hpc_mbox_chan *mchan,
	struct hpc_mbox *mbox, unsigned int slot)
{
	writel_relaxed(BIT(mchan->ack_irq), mbox->base + IPCM_SR(slot));
	writel_relaxed(MODE_AUTO_ACK, mbox->base + IPCM_MODE(slot));
	writel_relaxed(BIT(mchan->ack_irq) | BIT(mchan->dst_irq), mbox->base + IPCM_MSET(slot));
	writel_relaxed(BIT(mchan->dst_irq), mbox->base + IPCM_DEST(slot));
}

static irqreturn_t hpm_mbox_interrupt(int irq, void *pdev)
{
	struct hpc_mbox *mbox = pdev;
	struct hpc_mbox_chan *mchan;
	struct mbox_chan *chan;

	int irq_idx = 0;
	int i, state;
	int *chan_array;
	u32 msg[MBOX_MSG_LEN];

	for (i = 0; i < mbox->irq_num; i++) {
		if (irq == *(mbox->irq_array + i)) {
			irq_idx = i;
			break;
		}
	}
	state = readl_relaxed(mbox->base + IPCM_RIS(MBOX_IRQ_CHECK_START + irq_idx));
	if (!state) {
		hpxerr("%s: spurious interrupt, irq num %d\n", __func__, irq);
		return IRQ_HANDLED;
	}

	chan_array = mbox->irq_inc_chan[irq_idx];
	//chan_array comes from the dtsi <irq, chan_num, chan ....>
	for (i = 0; i < chan_array[1]; i++) {
		if (state & BIT(MBOX_TX_CHAN_START + chan_array[i + 2])) {
			//get an ack
			chan = mbox->irq_map_chan[MBOX_TX_CHAN_START + chan_array[i + 2]];
			if (!chan) {
				hpxerr("%s: unexpected irq  vector %d", __func__, state);
				return IRQ_HANDLED;
			}
			mchan = chan->con_priv;
			writel_relaxed(MBOX_SEND_INACTIVE, mbox->base + IPCM_SEND(mchan->first_id));
			writel_relaxed(MBOX_SOURCE_CLEAR, mbox->base + IPCM_SR(mchan->first_id));

			mbox_chan_txdone(chan, 0);
		}

		if (state & BIT(chan_array[i + 2])) {
			chan = mbox->irq_map_chan[chan_array[i + 2]];
			if (!chan) {
				hpxerr("%s: unexpected irq  vector %d", __func__, state);
				return IRQ_HANDLED;
			}
			mchan = chan->con_priv;
			hpc_read_msg(msg, mbox, mchan->first_id);
			mbox_chan_received_data(chan, (void *)msg);

			writel_relaxed(BIT(mchan->ack_irq),
					mbox->base + IPCM_DCLR(mchan->first_id));
			writel_relaxed(MBOX_SEND_ACK, mbox->base + IPCM_SEND(mchan->first_id));
		}
	}
	return IRQ_HANDLED;
}

static int hpc_mbox_send_data(struct mbox_chan *chan, void *msg)
{
	struct hpc_mbox_chan *mchan = chan->con_priv;
	struct hpc_mbox *mbox = mchan->parent;
	u32 *buf = msg;
	u32 len = *buf;
	int i;

	mchan->dir = MBOX_TX;
	hpc_mbox_set_state(mchan, mbox, mchan->first_id);
	len = min_t(u32, len, MBOX_MSG_LEN);

	writel_relaxed(len, mbox->base + IPCM_DAT(mchan->first_id, 0));
	if (len <= MBOX_DATA_REG_LEN) {
		for (i = 0; i < len; i++)
			writel_relaxed(buf[i + 1], mbox->base + IPCM_DAT(mchan->first_id, i + 1));
		writel_relaxed(MBOX_SEND, mbox->base + IPCM_SEND(mchan->first_id));

		return 0;
	}

	for (i = 0; i < MBOX_DATA_REG_LEN; i++)
		writel_relaxed(buf[i + 1], mbox->base + IPCM_DAT(mchan->first_id, i + 1));

	for (i = 0; i < MBOX_NDATA_REG_LEN; i++)
		writel_relaxed(buf[i + MBOX_DATA_REG_LEN + 1],
				mbox->base + IPCM_NDAT(mchan->first_id, i));

	writel_relaxed(MBOX_SEND, mbox->base + IPCM_SEND(mchan->first_id));

	return 0;
}

static int hpc_mbox_startup(struct mbox_chan *chan)
{
	/*before startup the mbox, send a message to check the msg*/
	int ret = 0;
	struct hpc_mbox_chan *mchan = chan->con_priv;

	mchan->dir = MBOX_START;

	//todo：check the client is online

	return ret;

}

static void hpc_mbox_shutdown(struct mbox_chan *chan)
{
	struct hpc_mbox_chan *mchan = chan->con_priv;
	struct hpc_mbox *mbox = mchan->parent;

	mbox->irq_map_chan[mchan->first_id] = NULL;
}

/*txdone_poll use this function*/
static bool hpc_mbox_last_tx_done(struct mbox_chan *chan)
{
	struct hpc_mbox_chan *mchan = chan->con_priv;
	struct hpc_mbox *mbox = mchan->parent;
	u32 state;

	state = readl_relaxed(mbox->base + IPCM_SR(mchan->first_id));
	return (state == MBOX_LAST_TXDONE);
}

static const struct mbox_chan_ops hpc_mbox_ops = {
	.send_data = hpc_mbox_send_data,
	.startup = hpc_mbox_startup,
	.shutdown = hpc_mbox_shutdown,
	.last_tx_done = hpc_mbox_last_tx_done,
};

static struct mbox_chan *hpc_mbox_xlate(
	struct mbox_controller *controller,
	const struct of_phandle_args *spec)
{
	struct hpc_mbox *mbox = dev_get_drvdata(controller->dev);
	struct hpc_mbox_chan *mchan;
	struct mbox_chan *chan;

	unsigned int chid = spec->args[0];
	unsigned int dst_irq = spec->args[1];
	unsigned int ack_irq = spec->args[2];

	/* Bounds checking */
	if (chid >= mbox->chan_num || dst_irq >= mbox->chan_num ||
		ack_irq >= mbox->chan_num) {
		hpxerr("Invalid channel idx %d dst_irq %d ack_irq %d\n", chid, dst_irq, ack_irq);
		return ERR_PTR(-EINVAL);
	}

	/* Is requested channel free? */
	chan = &mbox->chan[chid];
	if (mbox->irq_map_chan[chid] == (void *)chan) {
		hpxerr("Channel in use\n");
		return ERR_PTR(-EBUSY);
	}

	mchan = chan->con_priv;
	mchan->dst_irq = dst_irq;
	mchan->ack_irq = ack_irq;
	mchan->first_id = chid;
	mbox->irq_map_chan[chid] = (void *)chan;

	return chan;
}

static int hpc_mbox_irq_parse(struct device *dev, struct device_node *node, struct hpc_mbox *hmbox)
{
	int irq_num;
	int ret = 0, i, irq_tmp, ele_size;

	ret = of_property_read_u32(node, irq_num_name, &irq_num);
	if (ret != 0) {
		hpxerr("hpc irq info parse err, ");
		hpxerr("check the dtsi to make sure there have irq-num property\n");
		goto out;
	}
	hmbox->irq_num = irq_num;
	hmbox->irq_array = devm_kzalloc(dev, irq_num * (sizeof(int)), GFP_KERNEL);
	hmbox->irq_inc_chan = devm_kzalloc(dev, irq_num * sizeof(int *), GFP_KERNEL);

	for (i = 0; i < irq_num; i++) {
		/*get the irq number from dtsi*/
		irq_tmp = platform_get_irq(hmbox->pdev, i);
		hpxinfo("hpc parse irq, get the irq-%d num is %d", i, irq_tmp);
		if (!irq_tmp) {
			hpxerr("get mbox interrupts failed\n");
			return -EINVAL;
		}

		*(hmbox->irq_array + i) = irq_tmp;
		/*regist the irq */
		ret = devm_request_threaded_irq(dev, irq_tmp,
				hpm_mbox_interrupt, NULL, 0, dev_name(dev), hmbox);
		if (ret) {
			hpxerr("Failed to register a mailbox IRQ handler: %d\n", ret);
			return -ENODEV;
		}

		/*get the relation between the irq and chan*/
		ele_size = of_property_count_elems_of_size(node, irq_name[i], sizeof(int));
		hmbox->irq_inc_chan[i] = devm_kzalloc(dev, ele_size * sizeof(int), GFP_KERNEL);
		ret = of_property_read_u32_array(node, irq_name[i],
				hmbox->irq_inc_chan[i], ele_size);
		if (ret) {
			hpxerr("Failed to get the relation between irq and channel, err = %d\n", ret);
			return ret;
		}

		irq_set_affinity(irq_tmp, cpumask_of(1));
	}
out:
	return ret;
}

static int hpc_mbox_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct hpc_mbox *hmbox = NULL;
	struct device_node *node = dev->of_node;
	int i, err;

	hpxinfo("hpc mbox probe!\n");
	hmbox = devm_kzalloc(&pdev->dev, sizeof(*hmbox), GFP_KERNEL);
	if (!hmbox) {
		hpxerr("hpc mbox zalloc failed\n");
		return -ENOMEM;
	}

	hmbox->dev = dev;
	hmbox->pdev = pdev;
	hmbox->chan_num = MBOX_CHAN_MAX;
	hmbox->mchan = devm_kcalloc(dev, hmbox->chan_num, sizeof(*hmbox->mchan), GFP_KERNEL);
	if (!hmbox->mchan)
		return -ENOMEM;

	hmbox->chan = devm_kcalloc(dev, hmbox->chan_num, sizeof(*hmbox->chan), GFP_KERNEL);
	if (!hmbox->chan)
		return -ENOMEM;

	hmbox->reg = platform_get_resource(hmbox->pdev, IORESOURCE_MEM, 0);
	if (hmbox->reg == NULL)
		return -EINVAL;
	hmbox->base = devm_ioremap_resource(dev, hmbox->reg);

	err = hpc_mbox_irq_parse(&pdev->dev, node, hmbox);
	if (err) {
		hpxerr("Failed to parse irq info, %d\n", err);
		return err;
	}

	hmbox->controller.dev = dev;
	hmbox->controller.chans = &hmbox->chan[0];
	hmbox->controller.num_chans = hmbox->chan_num;
	hmbox->controller.ops = &hpc_mbox_ops;
	hmbox->controller.of_xlate = hpc_mbox_xlate;

	for (i = 0; i < hmbox->chan_num; i++) {
		hmbox->chan[i].con_priv = &hmbox->mchan[i];
		hmbox->irq_map_chan[i] = NULL;
		hmbox->mchan[i].parent = hmbox;
	}
	hmbox->tx_irq_mode = true;
	hmbox->controller.txdone_irq = true;
	hmbox->controller.txdone_poll = false;

	err = devm_mbox_controller_register(dev, &hmbox->controller);
	if (err) {
		hpxerr("Failed to register mailbox %d\n", err);
		return err;
	}

	platform_set_drvdata(pdev, hmbox);
	hpxinfo("end\n");

	return 0;
}

static int hpc_mbox_remove(struct platform_device *pdev)
{
	hpxinfo("hpc mbox remove!\n");
	hpxinfo("end\n");

	return 0;
}

static const struct of_device_id hpc_mbox_id_table[] = {
	{ .compatible = "xring,hpc-mbox" },
	{ /* Sentinel */ },
};
MODULE_DEVICE_TABLE(of, hpc_mbox_id_table);

static struct platform_driver hpc_mbox_driver = {
	.probe = hpc_mbox_probe,
	.remove = hpc_mbox_remove,
	.driver = {
		.name = "hpc-mbox",
		.of_match_table = of_match_ptr(hpc_mbox_id_table),
	},
};
module_platform_driver(hpc_mbox_driver);

MODULE_AUTHOR("High Performance Computing Group");
MODULE_DESCRIPTION("HPC Mailbox Driver");
MODULE_LICENSE("GPL v2");
