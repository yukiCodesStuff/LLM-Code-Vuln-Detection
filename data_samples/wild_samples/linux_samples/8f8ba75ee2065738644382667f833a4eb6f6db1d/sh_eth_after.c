	switch (mdp->phy_interface) {
	case PHY_INTERFACE_MODE_GMII:
		value = 0x2;
		break;
	case PHY_INTERFACE_MODE_MII:
		value = 0x1;
		break;
	case PHY_INTERFACE_MODE_RMII:
		value = 0x0;
		break;
	default:
		pr_warn("PHY interface mode was not setup. Set to MII.\n");
		value = 0x1;
		break;
	}

	sh_eth_write(ndev, value, RMII_MII);
}
#endif

/* There is CPU dependent code */
#if defined(CONFIG_CPU_SUBTYPE_SH7724) || defined(CONFIG_ARCH_R8A7779)
#define SH_ETH_RESET_DEFAULT	1
static void sh_eth_set_duplex(struct net_device *ndev)
{
	struct sh_eth_private *mdp = netdev_priv(ndev);

	if (mdp->duplex) /* Full */
		sh_eth_write(ndev, sh_eth_read(ndev, ECMR) | ECMR_DM, ECMR);
	else		/* Half */
		sh_eth_write(ndev, sh_eth_read(ndev, ECMR) & ~ECMR_DM, ECMR);
}
{
	struct sh_eth_private *mdp = netdev_priv(ndev);
	unsigned int bits = ECMR_RTM;

#if defined(CONFIG_ARCH_R8A7779)
	bits |= ECMR_ELB;
#endif

	switch (mdp->speed) {
	case 10: /* 10BASE */
		sh_eth_write(ndev, sh_eth_read(ndev, ECMR) & ~bits, ECMR);
		break;
	case 100:/* 100BASE */
		sh_eth_write(ndev, sh_eth_read(ndev, ECMR) | bits, ECMR);
		break;
	default:
		break;
	}
}