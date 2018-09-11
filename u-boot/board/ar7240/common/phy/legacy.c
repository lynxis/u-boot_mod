
#include <common.h>
#include <config.h>
#include <phy.h>
#include <linux/types.h>
#include <malloc.h>
#include <miiphy.h>
#include <phy_interface.h>

#ifdef CONFIG_MDIO_GPIO
#include <soc/mdio_gpio.h>
#else
#include "../phy.h"
#endif

int gen_phy_is_up(int unit);
int gen_phy_speed(int unit);
int gen_phy_setup(int unit, struct eth_device *dev);
int gen_phy_is_fdx(int unit);

struct mii_dev bus;
struct phy_device *phydev;
static int initialized = 0;

static int ag_mdio_read(struct mii_dev *bus, int addr, int devad, int reg) {
#ifdef CONFIG_MDIO_GPIO
	int value = mdio_read(addr, reg);
#else
	int value = ag7240_miiphy_read("eth0", addr, reg);
#endif

	return value;
}

static int ag_mdio_write(struct mii_dev *bus, int addr, int devad, int reg, u16 value) {
#ifdef CONFIG_MDIO_GPIO
	mdio_write(addr, reg, value);
#else
	ag7240_miiphy_write("eth0", addr, reg, value);
#endif
	return 0;
}

int gen_phy_is_up(int unit) {
	if (unit != 0)
		return 0;

	genphy_update_link(phydev);
	genphy_parse_link(phydev);

	return phydev->link;
}

int gen_phy_speed(int unit) {
	if (unit != 0)
		return 0;

	genphy_update_link(phydev);
	genphy_parse_link(phydev);

	switch (phydev->speed) {
	case SPEED_1000:
		return _1000BASET;
	case SPEED_100:
		return _100BASET;
	case SPEED_10:
		return _10BASET;
	default:
		return 0;
	}
}

int gen_phy_setup(int unit, struct eth_device *dev) {
	int i;

	if (unit != 0)
		return 0;

	if (initialized)
		return 0;

	initialized = 1;
	memset(&bus, 0, sizeof(struct mii_dev));

	/* register all drvs */
	phy_init();

	phydev = NULL;

        bus.read = ag_mdio_read;
        bus.write = ag_mdio_write;

	milisecdelay(10);

	for (i=0; i<10; i++) {
		phydev = phy_connect(&bus, i, dev, PHY_INTERFACE_MODE_RGMII_ID);
		if (phydev) {
			break;
		}
	}
	if (!phydev) {
		initialized = 0;
		return 0;
	}

	phy_config(phydev);
	phy_startup(phydev);

	return 0;
}

int gen_phy_is_fdx(int unit) {
	if (unit != 0)
		return 0;

	genphy_update_link(phydev);
	genphy_parse_link(phydev);

	switch (phydev->duplex) {
	case DUPLEX_FULL:
		return 1;
	case DUPLEX_HALF:
		return 0;
	default:
		return 0;
	}

	return 0;
}
