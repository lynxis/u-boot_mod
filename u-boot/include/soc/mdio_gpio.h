
#ifndef COMMON_MDIO_GPIO
#define COMMON_MDIO_GPIO

#include <linux/types.h>

#define MDC_GPIO 17
#define MDIO_GPIO 16

uint16_t mdio_read(int phy, uint16_t reg);
void mdio_write(int phy, uint16_t reg, uint16_t value);

#endif /* COMMON_MDIO_GPIO */
