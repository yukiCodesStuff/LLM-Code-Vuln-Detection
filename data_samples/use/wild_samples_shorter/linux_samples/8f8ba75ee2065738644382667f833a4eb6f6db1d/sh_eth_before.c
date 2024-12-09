#endif

/* There is CPU dependent code */
#if defined(CONFIG_CPU_SUBTYPE_SH7724)
#define SH_ETH_RESET_DEFAULT	1
static void sh_eth_set_duplex(struct net_device *ndev)
{
	struct sh_eth_private *mdp = netdev_priv(ndev);
static void sh_eth_set_rate(struct net_device *ndev)
{
	struct sh_eth_private *mdp = netdev_priv(ndev);

	switch (mdp->speed) {
	case 10: /* 10BASE */
		sh_eth_write(ndev, sh_eth_read(ndev, ECMR) & ~ECMR_RTM, ECMR);
		break;
	case 100:/* 100BASE */
		sh_eth_write(ndev, sh_eth_read(ndev, ECMR) | ECMR_RTM, ECMR);
		break;
	default:
		break;
	}