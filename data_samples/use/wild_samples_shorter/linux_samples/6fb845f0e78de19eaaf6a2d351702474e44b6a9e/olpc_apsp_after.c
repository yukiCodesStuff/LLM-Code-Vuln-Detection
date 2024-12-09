#include <linux/of.h>
#include <linux/slab.h>
#include <linux/delay.h>

/*
 * The OLPC XO-1.75 and XO-4 laptops do not have a hardware PS/2 controller.
 * Instead, the OLPC firmware runs a bit-banging PS/2 implementation on an
	struct serio *kbio;
	struct serio *padio;
	void __iomem *base;
	int open_count;
	int irq;
};

	struct olpc_apsp *priv = port->port_data;
	unsigned int tmp;
	unsigned long l;

	if (priv->open_count++ == 0) {
		l = readl(priv->base + COMMAND_FIFO_STATUS);
		if (!(l & CMD_STS_MASK)) {
			dev_err(priv->dev, "SP cannot accept commands.\n");
			return -EIO;
		}

		/* Enable interrupt 0 by clearing its bit */
		/* Disable interrupt 0 */
		tmp = readl(priv->base + PJ_INTERRUPT_MASK);
		writel(tmp | INT_0, priv->base + PJ_INTERRUPT_MASK);
	}
}

static int olpc_apsp_probe(struct platform_device *pdev)
	if (priv->irq < 0)
		return priv->irq;

	/* KEYBOARD */
	kb_serio = kzalloc(sizeof(struct serio), GFP_KERNEL);
	if (!kb_serio)
		return -ENOMEM;