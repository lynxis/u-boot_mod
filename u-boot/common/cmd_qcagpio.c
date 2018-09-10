/*
 * Copyright (C) 2017 Piotr Dymacz <piotr@dymacz.pl>
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <config.h>
#include <common.h>
#include <command.h>
#include <soc/qca_soc_common.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <linux/ctype.h>

#include <soc/qca_gpios.h>

#if defined(CONFIG_CMD_GPIO)

int do_gpio(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u32 gpio_mask, gpio_num = 0;
	u32 output = 1;
	u32 func = 0;
	u32 val = 0;

#if (SOC_TYPE & QCA_AR933X_SOC)
	u32 func_max = gpio_funcs_1_cnt + gpio_funcs_2_cnt - 2;
	u32 func_en;
#else
	u32 func_max = gpio_in_funcs_cnt - 1;
	u32 func_old;
#endif

	/* We need at least one arg */
	if (argc == 1) {
usage:
		print_cmd_help(cmdtp);
		return 1;
	}

	switch (*argv[1]) {
	case 'f':
		if (argc < 4)
			goto usage;

#if (SOC_TYPE & QCA_AR933X_SOC)
		func = simple_strtoul(argv[2], NULL, 10);
		if (func > func_max) {
			printf_err("GPIO function is out of range (0~%d)\n",
				   func_max);

			return 1;
		}

		switch (*argv[3]) {
		case 'e':
			func_en = 1;
			break;
		case 'd':
			func_en = 0;
			break;
		default:
			goto usage;
		}

		gpio_set_func(func, func_en);

		return 0;
#else
		if (strcmp(argv[1], "fin") == 0)
			output = 0;
		else if (strcmp(argv[1], "fout") == 0)
			func_max = _GPIO_OUT_MUX_MAX;
		else
			goto usage;

		func = simple_strtoul(argv[3], NULL, 10);
		if (func > func_max) {
			printf_err("GPIO function is out of range (0~%d)\n",
				   func_max);

			return 1;
		}

		break;
#endif
	case 'j':
		if (argc < 2)
			goto usage;

		switch (*argv[2]) {
		case 'e':
			gpio_set_jtag(1);
			break;
		case 'd':
			gpio_set_jtag(0);
			break;
		default:
			goto usage;
		}

		return 0;
	case 'l':
		if (argc < 3)
			gpio_list_cfg();
		else if (*argv[2] == 'f')
#if (SOC_TYPE & QCA_AR933X_SOC)
			gpio_list_funcs();
#else
			gpio_list_in_out_funcs();
#endif
		else
			goto usage;

		return 0;
	case 's':
	case 'c':
	case 't':
	case 'i':
		if (argc < 3)
			goto usage;

		if (*argv[1] == 's')
			val = 1;

		break;
	default:
		goto usage;
	}

	gpio_num = simple_strtoul(argv[2], NULL, 10);
	if (gpio_num > _GPIO_NUM_MAX) {
		printf_err("GPIO must be in range 0~%d\n", _GPIO_NUM_MAX);
		return 1;
	}

	gpio_mask = (1 << gpio_num);

	switch (*argv[1]) {
	case 's':
	case 'c':
#if (SOC_TYPE & ~QCA_AR933X_SOC)
	case 'f':
		if (output)
			func_old = gpio_get_out_func(gpio_num);
#endif

		gpio_set_dir(gpio_mask, output);

#if (SOC_TYPE & ~QCA_AR933X_SOC)
		gpio_set_in_out_func(gpio_num, (u8)func, output);
#endif

		if ((*argv[1] == 's') || (*argv[1] == 'c'))
			gpio_set_val(gpio_mask, val);

#if (SOC_TYPE & ~QCA_AR933X_SOC)
		if (output && (func != func_old))
			gpio_func_gate(func_old, func);
#endif

		break;
	case 't':
		val = qca_soc_reg_read(QCA_GPIO_OE_REG);
#if (SOC_TYPE & QCA_AR933X_SOC) ||\
    (SOC_TYPE & QCA_QCA955X_SOC)
		if (!(gpio_mask & val)) {
#else
		if (gpio_mask & val) {
#endif
			printf_err("GPIO is not set as output\n");
			return 1;
		}

#if (SOC_TYPE & ~QCA_AR933X_SOC)
		func = (u32)gpio_get_out_func(gpio_num);
		if (func > 0) {
			printf_err("GPIO out function is not set to GPIO\n");
			return 1;
		}
#endif

		val = qca_soc_reg_read(QCA_GPIO_OUT_REG);
		if (gpio_mask & val)
			gpio_set_val(gpio_mask, 0);
		else
			gpio_set_val(gpio_mask, 1);

		break;
	case 'i':
		gpio_set_dir(gpio_mask, 0);

		if (!(gpio_mask & qca_soc_reg_read(QCA_GPIO_IN_REG)))
			return 1;
	}

	return 0;
}

U_BOOT_CMD(gpio, 4, 0, do_gpio,
	   "control GPIO\n",
	   "<c>lear|<i>nput|<s>et|<t>oggle <gpio>\n"
	   "\t- setup <gpio> as in/out and change/get its value\n"
#if (SOC_TYPE & QCA_AR933X_SOC)
	   "gpio <f>unction <func> <e>nable|<d>isable\n"
	   "\t- enable/disable function <func>\n"
#else
	   "gpio <fin|fout> <gpio> <func>\n"
	   "\t- set <gpio> in/out function to <func>\n"
#endif
	   "gpio <j>tag <e>nable|<d>isable\n"
	   "\t- enable/disable JTAG\n"
	   "gpio <l>ist\n"
	   "\t- show current GPIO configuration\n"
	   "gpio <l>ist <f>unctions\n"
#if (SOC_TYPE & QCA_AR933X_SOC)
	   "\t- show all GPIO functions\n"
#else
	   "\t- show all known GPIO in/out functions\n"
#endif
);

#endif /* CONFIG_CMD_GPIO */
