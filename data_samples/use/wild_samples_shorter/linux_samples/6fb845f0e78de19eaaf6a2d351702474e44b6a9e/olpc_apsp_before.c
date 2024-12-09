#include <linux/of.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/clk.h>

/*
 * The OLPC XO-1.75 and XO-4 laptops do not have a hardware PS/2 controller.
 * Instead, the OLPC firmware runs a bit-banging PS/2 implementation on an
	struct serio *kbio;
	struct serio *padio;
	void __iomem *base;
	struct clk *clk;
	int open_count;
	int irq;
};

	struct olpc_apsp *priv = port->port_data;
	unsigned int tmp;
	unsigned long l;
	int error;

	if (priv->open_count++ == 0) {
		error = clk_prepare_enable(priv->clk);
		if (error)
			return error;

		l = readl(priv->base + COMMAND_FIFO_STATUS);
		if (!(l & CMD_STS_MASK)) {
			dev_err(priv->dev, "SP cannot accept commands.\n");
			clk_disable_unprepare(priv->clk);
			return -EIO;
		}

		/* Enable interrupt 0 by clearing its bit */
		/* Disable interrupt 0 */
		tmp = readl(priv->base + PJ_INTERRUPT_MASK);
		writel(tmp | INT_0, priv->base + PJ_INTERRUPT_MASK);

		clk_disable_unprepare(priv->clk);
	}
}

static int olpc_apsp_probe(struct platform_device *pdev)
	if (priv->irq < 0)
		return priv->irq;

	priv->clk = devm_clk_get(&pdev->dev, "sp");
	if (IS_ERR(priv->clk))
		return PTR_ERR(priv->clk);

	/* KEYBOARD */
	kb_serio = kzalloc(sizeof(struct serio), GFP_KERNEL);
	if (!kb_serio)
		return -ENOMEM;