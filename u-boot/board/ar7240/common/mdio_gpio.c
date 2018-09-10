
#include <config.h>
#include <common.h>

#include <soc/mdio_gpio.h>
#include <soc/qca_gpios.h>

#define mdc_sleep() udelay(1000)
#define mdio_low() gpio_set_val((1 << MDIO_GPIO), 0)
#define mdio_high() gpio_set_val((1 << MDIO_GPIO), 1)

/* clk which stops at low, used for output */
static inline void mdo_clk() {
	gpio_set_val((1 << MDC_GPIO), 1);
	mdc_sleep();
	gpio_set_val((1 << MDC_GPIO), 0);
	mdc_sleep();
}

static inline void xmit_bit(int value)
{
	gpio_set_val((1 << MDIO_GPIO), value);
	mdo_clk();
}

static inline void preamble()
{
	int i;

	gpio_set_dir((1 << MDIO_GPIO), 1);
	gpio_set_val((1 << MDIO_GPIO), 1);

	for (i=0; i < 32; i++) {
		mdo_clk();
	}
}

uint16_t mdio_read(int phy, uint16_t reg)
{
	uint16_t value = 0;
	int i;

	gpio_set_in_out_func(MDIO_GPIO, 0, 1);
	gpio_set_in_out_func(MDC_GPIO, 0, 1);
	gpio_set_dir((1 << MDIO_GPIO), 1);
	gpio_set_dir((1 << MDC_GPIO), 1);

	preamble();

	/* start */
	xmit_bit(0);
	xmit_bit(1);

	/* read */
	xmit_bit(1);
	xmit_bit(0);

	/* phy address */
	for (i=4 ; i >= 0; i--)
		xmit_bit(phy & (1 << i));
	/* reg address */
	for (i=4 ; i >= 0; i--)
		xmit_bit(reg & (1 << i));

	/* allow device to drive */
	gpio_set_dir((1 << MDIO_GPIO), 0);

	/* turn around */
	mdo_clk();
	mdo_clk();

	/* data */
	for (i=15; i >= 0; i--) {
		if (gpio_get_value(MDIO_GPIO))
			value |= (1 << i);
		mdo_clk();
	}

	return value;
}

void mdio_write(int phy, uint16_t reg, uint16_t value)
{
	int i;

	gpio_set_in_out_func(MDIO_GPIO, 0, 1);
	gpio_set_in_out_func(MDC_GPIO, 0, 1);
	gpio_set_dir((1 << MDIO_GPIO), 1);
	gpio_set_dir((1 << MDC_GPIO), 1);

	preamble();

	/* start */
	xmit_bit(0);
	xmit_bit(1);

	/* write */
	xmit_bit(0);
	xmit_bit(1);

	/* phy address */
	for (i=4 ; i >= 0; i--)
		xmit_bit(phy & (1 << i));
	/* reg address */
	for (i=4 ; i >= 0; i--)
		xmit_bit(reg & (1 << i));

	/* turn over */
	mdio_high();
	mdo_clk();
	mdio_low();
	mdo_clk();

	/* data */
	for (i=15; i >= 0; i--)
		xmit_bit(value & (1 << i));

	/* put bus into HZ */
	gpio_set_dir((1 << MDIO_GPIO), 0);
}

