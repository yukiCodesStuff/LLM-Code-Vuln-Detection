#include <linux/io.h>
#include <linux/of_irq.h>
#include <linux/of_device.h>
#include <linux/pinctrl/consumer.h>

/*
 * GPIO unit register offsets.
	struct resource *res;
	struct irq_chip_generic *gc;
	struct irq_chip_type *ct;
	unsigned int ngpios;
	int soc_variant;
	int i, cpu, id;

		return id;
	}

	mvchip->soc_variant = soc_variant;
	mvchip->chip.label = dev_name(&pdev->dev);
	mvchip->chip.dev = &pdev->dev;
	mvchip->chip.request = mvebu_gpio_request;