/*******************************************************************************

  NEXCOM Internation Co., Ltd. MDIO-CPLD Linux driver
  Copyright(c) 2015 John Zhang <johnzhang@nexcom.cn>

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

*******************************************************************************/
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/pagemap.h>
#include <linux/netdevice.h>
#include <linux/net_tstamp.h>
#include <linux/mii.h>
#include <linux/mdio.h>
#include <linux/phy.h>
//#include "e1000_hw.h"
//#include "e1000_phy.h"
#include "igb.h"

#define MDIO_DRV_NAME "i350_mdio"
#define MDIO_BUS_NAME "i350 MII Bus"
#define MDIO_DRV_VERSION "1.1.0"
#define MDIO_COPYRIGHT "Copyright(c) 2015-2017 Nexcom Co., Ltd."
#define MDIO_DRV_STRING MDIO_BUS_NAME
#define MDIO_DEFAULT_DEVICE_DESCR MDIO_BUS_NAME


#define MDIO_TIMEOUT			1000000
static int igb_num = 0;
/**
 * i350_mdio_write - access phy register
 * @bus: mdio bus
 * @phy_id: phy address
 * @regnum: register num
 * @value: register value
 *
 * Return 0 on success, negative on failure
 */
static int i350_mdio_write(struct mii_bus *bus,
			  int phy, int regnum, u16 data)
{
	int ret;
	u16 reg_val = 0;
	u16 pcib,slot,dev;
	pcib = igb_num > 3 ? 2:1;
	slot = 0;
	dev = igb_num % 4;
    struct pci_dev *pdev = pci_get_bus_and_slot(pcib,PCI_DEVFN(slot,dev));

    ret=i350_mdio_write_pci(pdev,phy,regnum,data);

    return ret;
}

/**
 * i350_mdio_read - access phy register
 * @bus: mdio bus
 * @phy: phy address
 * @regnum: register num
 *
 * Return phy register value
 */
static int i350_mdio_read(struct mii_bus *bus, int phy, int regnum)
{
	int ret;
	u16 reg_val = 0;
	u16 pcib,slot,dev;
	pcib = igb_num > 3 ? 2:1;
	slot = 0;
	dev = igb_num % 4;
    struct pci_dev *pdev = pci_get_bus_and_slot(pcib,PCI_DEVFN(slot,dev));

    reg_val=i350_mdio_read_pci(pdev,phy,regnum);
    return reg_val;
}

/**
 * i350_mdio_reset - reset mdio bus
 * @bus: mdio bus
 *
 * Return 0 on success, negative on failure
 */
static int i350_mdio_reset(struct mii_bus *bus)
{
    return 0;
}

/**
 * i350_mdio_probe - probe mdio device
 * @pdev: mdio pci device
 *
 * Return 0 on success, negative on failure
 */
static int i350_mdio_probe(struct mdio_device *mdiodev)
{
	struct mii_bus *new_bus;
	struct device *dev = &mdiodev->dev;
	int ret = -ENODEV;

	pr_info("NEXCOM I350 MDIO\n");

	new_bus = mdiobus_alloc();
	if (!new_bus) {
		pr_info("mdiobus_alloc fail!\n");
		return -ENOMEM;
	}
	new_bus->name = MDIO_BUS_NAME;
	new_bus->read = i350_mdio_read;
	new_bus->write = i350_mdio_write;
	new_bus->reset = i350_mdio_reset;
	new_bus->parent = dev;

	snprintf(new_bus->id, MII_BUS_ID_SIZE, "i350_mdio%d", igb_num++);

		/* Mask out all PHYs from auto probing. */
	new_bus->phy_mask = ~0;

	pr_info("NEXCOM I350 MDIO>>>>>>>\n");
		/* Register the MDIO bus */
	ret = mdiobus_register(new_bus);
	if (ret) {
		mdiobus_unregister(new_bus);
		return ret;
	}
	printk(KERN_INFO "%s\n","Nexcom mdio bus registered");
	return 0;
}

/**
 * i350_mdio_remove - remove mdio device
 * @pdev: mdio device
 *
 * Return 0 on success, negative on failure
 */
static void i350_mdio_remove(struct mdio_device *mdiodev)
{
	struct mii_bus *bus;

	bus = dev_get_drvdata(&mdiodev->dev);

	mdiobus_unregister(bus);
	mdiobus_free(bus);
}

static const struct of_device_id i350_mdio_match[] = {
	{.compatible = "intel,i350"},
	{},
};

MODULE_DEVICE_TABLE(of, i350_mdio_match);

static struct mdio_driver i350_mdio_driver = {
	.probe = i350_mdio_probe,
	.remove = i350_mdio_remove,
	.mdiodrv.driver = {
		   .name = MDIO_DRV_NAME,
		   .of_match_table = i350_mdio_match,
		   },
};

static int __init i350_init(void)
{
	return mdio_driver_register(&i350_mdio_driver);
}
module_init(i350_init);

static void __exit i350_cleanup(void)
{
	mdio_driver_unregister(&i350_mdio_driver);
}
module_exit(i350_cleanup);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Zhang<johnzhang@nexcom.cn>");
MODULE_DESCRIPTION("Nexcom IGB MDIO driver");
MODULE_ALIAS("mdio:" MDIO_DRV_NAME);
