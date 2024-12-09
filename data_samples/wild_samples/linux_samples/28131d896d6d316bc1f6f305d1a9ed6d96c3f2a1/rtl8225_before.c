{
	u8 val;

	mutex_lock(&priv->io_mutex);
	usb_control_msg(priv->udev, usb_rcvctrlpipe(priv->udev, 0),
			RTL8187_REQ_GET_REG, RTL8187_REQT_READ,
			(unsigned long)addr, idx & 0x03,
			&priv->io_dmabuf->bits8, sizeof(val), HZ / 2);

	val = priv->io_dmabuf->bits8;
	mutex_unlock(&priv->io_mutex);

	return val;
}

u16 rtl818x_ioread16_idx(struct rtl8187_priv *priv,
				__le16 *addr, u8 idx)
{
	__le16 val;

	mutex_lock(&priv->io_mutex);
	usb_control_msg(priv->udev, usb_rcvctrlpipe(priv->udev, 0),
			RTL8187_REQ_GET_REG, RTL8187_REQT_READ,
			(unsigned long)addr, idx & 0x03,
			&priv->io_dmabuf->bits16, sizeof(val), HZ / 2);

	val = priv->io_dmabuf->bits16;
	mutex_unlock(&priv->io_mutex);

	return le16_to_cpu(val);
}

u32 rtl818x_ioread32_idx(struct rtl8187_priv *priv,
				__le32 *addr, u8 idx)
{
	__le32 val;

	mutex_lock(&priv->io_mutex);
	usb_control_msg(priv->udev, usb_rcvctrlpipe(priv->udev, 0),
			RTL8187_REQ_GET_REG, RTL8187_REQT_READ,
			(unsigned long)addr, idx & 0x03,
			&priv->io_dmabuf->bits32, sizeof(val), HZ / 2);

	val = priv->io_dmabuf->bits32;
	mutex_unlock(&priv->io_mutex);

	return le32_to_cpu(val);
}

void rtl818x_iowrite8_idx(struct rtl8187_priv *priv,
				u8 *addr, u8 val, u8 idx)
{
	mutex_lock(&priv->io_mutex);

	priv->io_dmabuf->bits8 = val;
	usb_control_msg(priv->udev, usb_sndctrlpipe(priv->udev, 0),
			RTL8187_REQ_SET_REG, RTL8187_REQT_WRITE,
			(unsigned long)addr, idx & 0x03,
			&priv->io_dmabuf->bits8, sizeof(val), HZ / 2);

	mutex_unlock(&priv->io_mutex);
}

void rtl818x_iowrite16_idx(struct rtl8187_priv *priv,
				__le16 *addr, u16 val, u8 idx)
{
	mutex_lock(&priv->io_mutex);

	priv->io_dmabuf->bits16 = cpu_to_le16(val);
	usb_control_msg(priv->udev, usb_sndctrlpipe(priv->udev, 0),
			RTL8187_REQ_SET_REG, RTL8187_REQT_WRITE,
			(unsigned long)addr, idx & 0x03,
			&priv->io_dmabuf->bits16, sizeof(val), HZ / 2);

	mutex_unlock(&priv->io_mutex);
}

void rtl818x_iowrite32_idx(struct rtl8187_priv *priv,
				__le32 *addr, u32 val, u8 idx)
{
	mutex_lock(&priv->io_mutex);

	priv->io_dmabuf->bits32 = cpu_to_le32(val);
	usb_control_msg(priv->udev, usb_sndctrlpipe(priv->udev, 0),
			RTL8187_REQ_SET_REG, RTL8187_REQT_WRITE,
			(unsigned long)addr, idx & 0x03,
			&priv->io_dmabuf->bits32, sizeof(val), HZ / 2);

	mutex_unlock(&priv->io_mutex);
}

static void rtl8225_write_bitbang(struct ieee80211_hw *dev, u8 addr, u16 data)
{
	struct rtl8187_priv *priv = dev->priv;
	u16 reg80, reg84, reg82;
	u32 bangdata;
	int i;

	bangdata = (data << 4) | (addr & 0xf);

	reg80 = rtl818x_ioread16(priv, &priv->map->RFPinsOutput) & 0xfff3;
	reg82 = rtl818x_ioread16(priv, &priv->map->RFPinsEnable);

	rtl818x_iowrite16(priv, &priv->map->RFPinsEnable, reg82 | 0x7);

	reg84 = rtl818x_ioread16(priv, &priv->map->RFPinsSelect);
	rtl818x_iowrite16(priv, &priv->map->RFPinsSelect, reg84 | 0x7);
	udelay(10);

	rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg80 | (1 << 2));
	udelay(2);
	rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg80);
	udelay(10);

	for (i = 15; i >= 0; i--) {
		u16 reg = reg80 | (bangdata & (1 << i)) >> i;

		if (i & 1)
			rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg);

		rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg | (1 << 1));
		rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg | (1 << 1));

		if (!(i & 1))
			rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg);
	}
{
	__le16 val;

	mutex_lock(&priv->io_mutex);
	usb_control_msg(priv->udev, usb_rcvctrlpipe(priv->udev, 0),
			RTL8187_REQ_GET_REG, RTL8187_REQT_READ,
			(unsigned long)addr, idx & 0x03,
			&priv->io_dmabuf->bits16, sizeof(val), HZ / 2);

	val = priv->io_dmabuf->bits16;
	mutex_unlock(&priv->io_mutex);

	return le16_to_cpu(val);
}

u32 rtl818x_ioread32_idx(struct rtl8187_priv *priv,
				__le32 *addr, u8 idx)
{
	__le32 val;

	mutex_lock(&priv->io_mutex);
	usb_control_msg(priv->udev, usb_rcvctrlpipe(priv->udev, 0),
			RTL8187_REQ_GET_REG, RTL8187_REQT_READ,
			(unsigned long)addr, idx & 0x03,
			&priv->io_dmabuf->bits32, sizeof(val), HZ / 2);

	val = priv->io_dmabuf->bits32;
	mutex_unlock(&priv->io_mutex);

	return le32_to_cpu(val);
}

void rtl818x_iowrite8_idx(struct rtl8187_priv *priv,
				u8 *addr, u8 val, u8 idx)
{
	mutex_lock(&priv->io_mutex);

	priv->io_dmabuf->bits8 = val;
	usb_control_msg(priv->udev, usb_sndctrlpipe(priv->udev, 0),
			RTL8187_REQ_SET_REG, RTL8187_REQT_WRITE,
			(unsigned long)addr, idx & 0x03,
			&priv->io_dmabuf->bits8, sizeof(val), HZ / 2);

	mutex_unlock(&priv->io_mutex);
}

void rtl818x_iowrite16_idx(struct rtl8187_priv *priv,
				__le16 *addr, u16 val, u8 idx)
{
	mutex_lock(&priv->io_mutex);

	priv->io_dmabuf->bits16 = cpu_to_le16(val);
	usb_control_msg(priv->udev, usb_sndctrlpipe(priv->udev, 0),
			RTL8187_REQ_SET_REG, RTL8187_REQT_WRITE,
			(unsigned long)addr, idx & 0x03,
			&priv->io_dmabuf->bits16, sizeof(val), HZ / 2);

	mutex_unlock(&priv->io_mutex);
}

void rtl818x_iowrite32_idx(struct rtl8187_priv *priv,
				__le32 *addr, u32 val, u8 idx)
{
	mutex_lock(&priv->io_mutex);

	priv->io_dmabuf->bits32 = cpu_to_le32(val);
	usb_control_msg(priv->udev, usb_sndctrlpipe(priv->udev, 0),
			RTL8187_REQ_SET_REG, RTL8187_REQT_WRITE,
			(unsigned long)addr, idx & 0x03,
			&priv->io_dmabuf->bits32, sizeof(val), HZ / 2);

	mutex_unlock(&priv->io_mutex);
}

static void rtl8225_write_bitbang(struct ieee80211_hw *dev, u8 addr, u16 data)
{
	struct rtl8187_priv *priv = dev->priv;
	u16 reg80, reg84, reg82;
	u32 bangdata;
	int i;

	bangdata = (data << 4) | (addr & 0xf);

	reg80 = rtl818x_ioread16(priv, &priv->map->RFPinsOutput) & 0xfff3;
	reg82 = rtl818x_ioread16(priv, &priv->map->RFPinsEnable);

	rtl818x_iowrite16(priv, &priv->map->RFPinsEnable, reg82 | 0x7);

	reg84 = rtl818x_ioread16(priv, &priv->map->RFPinsSelect);
	rtl818x_iowrite16(priv, &priv->map->RFPinsSelect, reg84 | 0x7);
	udelay(10);

	rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg80 | (1 << 2));
	udelay(2);
	rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg80);
	udelay(10);

	for (i = 15; i >= 0; i--) {
		u16 reg = reg80 | (bangdata & (1 << i)) >> i;

		if (i & 1)
			rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg);

		rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg | (1 << 1));
		rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg | (1 << 1));

		if (!(i & 1))
			rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg);
	}
{
	__le32 val;

	mutex_lock(&priv->io_mutex);
	usb_control_msg(priv->udev, usb_rcvctrlpipe(priv->udev, 0),
			RTL8187_REQ_GET_REG, RTL8187_REQT_READ,
			(unsigned long)addr, idx & 0x03,
			&priv->io_dmabuf->bits32, sizeof(val), HZ / 2);

	val = priv->io_dmabuf->bits32;
	mutex_unlock(&priv->io_mutex);

	return le32_to_cpu(val);
}

void rtl818x_iowrite8_idx(struct rtl8187_priv *priv,
				u8 *addr, u8 val, u8 idx)
{
	mutex_lock(&priv->io_mutex);

	priv->io_dmabuf->bits8 = val;
	usb_control_msg(priv->udev, usb_sndctrlpipe(priv->udev, 0),
			RTL8187_REQ_SET_REG, RTL8187_REQT_WRITE,
			(unsigned long)addr, idx & 0x03,
			&priv->io_dmabuf->bits8, sizeof(val), HZ / 2);

	mutex_unlock(&priv->io_mutex);
}

void rtl818x_iowrite16_idx(struct rtl8187_priv *priv,
				__le16 *addr, u16 val, u8 idx)
{
	mutex_lock(&priv->io_mutex);

	priv->io_dmabuf->bits16 = cpu_to_le16(val);
	usb_control_msg(priv->udev, usb_sndctrlpipe(priv->udev, 0),
			RTL8187_REQ_SET_REG, RTL8187_REQT_WRITE,
			(unsigned long)addr, idx & 0x03,
			&priv->io_dmabuf->bits16, sizeof(val), HZ / 2);

	mutex_unlock(&priv->io_mutex);
}

void rtl818x_iowrite32_idx(struct rtl8187_priv *priv,
				__le32 *addr, u32 val, u8 idx)
{
	mutex_lock(&priv->io_mutex);

	priv->io_dmabuf->bits32 = cpu_to_le32(val);
	usb_control_msg(priv->udev, usb_sndctrlpipe(priv->udev, 0),
			RTL8187_REQ_SET_REG, RTL8187_REQT_WRITE,
			(unsigned long)addr, idx & 0x03,
			&priv->io_dmabuf->bits32, sizeof(val), HZ / 2);

	mutex_unlock(&priv->io_mutex);
}

static void rtl8225_write_bitbang(struct ieee80211_hw *dev, u8 addr, u16 data)
{
	struct rtl8187_priv *priv = dev->priv;
	u16 reg80, reg84, reg82;
	u32 bangdata;
	int i;

	bangdata = (data << 4) | (addr & 0xf);

	reg80 = rtl818x_ioread16(priv, &priv->map->RFPinsOutput) & 0xfff3;
	reg82 = rtl818x_ioread16(priv, &priv->map->RFPinsEnable);

	rtl818x_iowrite16(priv, &priv->map->RFPinsEnable, reg82 | 0x7);

	reg84 = rtl818x_ioread16(priv, &priv->map->RFPinsSelect);
	rtl818x_iowrite16(priv, &priv->map->RFPinsSelect, reg84 | 0x7);
	udelay(10);

	rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg80 | (1 << 2));
	udelay(2);
	rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg80);
	udelay(10);

	for (i = 15; i >= 0; i--) {
		u16 reg = reg80 | (bangdata & (1 << i)) >> i;

		if (i & 1)
			rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg);

		rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg | (1 << 1));
		rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg | (1 << 1));

		if (!(i & 1))
			rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg);
	}
{
	mutex_lock(&priv->io_mutex);

	priv->io_dmabuf->bits8 = val;
	usb_control_msg(priv->udev, usb_sndctrlpipe(priv->udev, 0),
			RTL8187_REQ_SET_REG, RTL8187_REQT_WRITE,
			(unsigned long)addr, idx & 0x03,
			&priv->io_dmabuf->bits8, sizeof(val), HZ / 2);

	mutex_unlock(&priv->io_mutex);
}

void rtl818x_iowrite16_idx(struct rtl8187_priv *priv,
				__le16 *addr, u16 val, u8 idx)
{
	mutex_lock(&priv->io_mutex);

	priv->io_dmabuf->bits16 = cpu_to_le16(val);
	usb_control_msg(priv->udev, usb_sndctrlpipe(priv->udev, 0),
			RTL8187_REQ_SET_REG, RTL8187_REQT_WRITE,
			(unsigned long)addr, idx & 0x03,
			&priv->io_dmabuf->bits16, sizeof(val), HZ / 2);

	mutex_unlock(&priv->io_mutex);
}

void rtl818x_iowrite32_idx(struct rtl8187_priv *priv,
				__le32 *addr, u32 val, u8 idx)
{
	mutex_lock(&priv->io_mutex);

	priv->io_dmabuf->bits32 = cpu_to_le32(val);
	usb_control_msg(priv->udev, usb_sndctrlpipe(priv->udev, 0),
			RTL8187_REQ_SET_REG, RTL8187_REQT_WRITE,
			(unsigned long)addr, idx & 0x03,
			&priv->io_dmabuf->bits32, sizeof(val), HZ / 2);

	mutex_unlock(&priv->io_mutex);
}

static void rtl8225_write_bitbang(struct ieee80211_hw *dev, u8 addr, u16 data)
{
	struct rtl8187_priv *priv = dev->priv;
	u16 reg80, reg84, reg82;
	u32 bangdata;
	int i;

	bangdata = (data << 4) | (addr & 0xf);

	reg80 = rtl818x_ioread16(priv, &priv->map->RFPinsOutput) & 0xfff3;
	reg82 = rtl818x_ioread16(priv, &priv->map->RFPinsEnable);

	rtl818x_iowrite16(priv, &priv->map->RFPinsEnable, reg82 | 0x7);

	reg84 = rtl818x_ioread16(priv, &priv->map->RFPinsSelect);
	rtl818x_iowrite16(priv, &priv->map->RFPinsSelect, reg84 | 0x7);
	udelay(10);

	rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg80 | (1 << 2));
	udelay(2);
	rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg80);
	udelay(10);

	for (i = 15; i >= 0; i--) {
		u16 reg = reg80 | (bangdata & (1 << i)) >> i;

		if (i & 1)
			rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg);

		rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg | (1 << 1));
		rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg | (1 << 1));

		if (!(i & 1))
			rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg);
	}
{
	mutex_lock(&priv->io_mutex);

	priv->io_dmabuf->bits16 = cpu_to_le16(val);
	usb_control_msg(priv->udev, usb_sndctrlpipe(priv->udev, 0),
			RTL8187_REQ_SET_REG, RTL8187_REQT_WRITE,
			(unsigned long)addr, idx & 0x03,
			&priv->io_dmabuf->bits16, sizeof(val), HZ / 2);

	mutex_unlock(&priv->io_mutex);
}

void rtl818x_iowrite32_idx(struct rtl8187_priv *priv,
				__le32 *addr, u32 val, u8 idx)
{
	mutex_lock(&priv->io_mutex);

	priv->io_dmabuf->bits32 = cpu_to_le32(val);
	usb_control_msg(priv->udev, usb_sndctrlpipe(priv->udev, 0),
			RTL8187_REQ_SET_REG, RTL8187_REQT_WRITE,
			(unsigned long)addr, idx & 0x03,
			&priv->io_dmabuf->bits32, sizeof(val), HZ / 2);

	mutex_unlock(&priv->io_mutex);
}

static void rtl8225_write_bitbang(struct ieee80211_hw *dev, u8 addr, u16 data)
{
	struct rtl8187_priv *priv = dev->priv;
	u16 reg80, reg84, reg82;
	u32 bangdata;
	int i;

	bangdata = (data << 4) | (addr & 0xf);

	reg80 = rtl818x_ioread16(priv, &priv->map->RFPinsOutput) & 0xfff3;
	reg82 = rtl818x_ioread16(priv, &priv->map->RFPinsEnable);

	rtl818x_iowrite16(priv, &priv->map->RFPinsEnable, reg82 | 0x7);

	reg84 = rtl818x_ioread16(priv, &priv->map->RFPinsSelect);
	rtl818x_iowrite16(priv, &priv->map->RFPinsSelect, reg84 | 0x7);
	udelay(10);

	rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg80 | (1 << 2));
	udelay(2);
	rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg80);
	udelay(10);

	for (i = 15; i >= 0; i--) {
		u16 reg = reg80 | (bangdata & (1 << i)) >> i;

		if (i & 1)
			rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg);

		rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg | (1 << 1));
		rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg | (1 << 1));

		if (!(i & 1))
			rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg);
	}
{
	mutex_lock(&priv->io_mutex);

	priv->io_dmabuf->bits32 = cpu_to_le32(val);
	usb_control_msg(priv->udev, usb_sndctrlpipe(priv->udev, 0),
			RTL8187_REQ_SET_REG, RTL8187_REQT_WRITE,
			(unsigned long)addr, idx & 0x03,
			&priv->io_dmabuf->bits32, sizeof(val), HZ / 2);

	mutex_unlock(&priv->io_mutex);
}

static void rtl8225_write_bitbang(struct ieee80211_hw *dev, u8 addr, u16 data)
{
	struct rtl8187_priv *priv = dev->priv;
	u16 reg80, reg84, reg82;
	u32 bangdata;
	int i;

	bangdata = (data << 4) | (addr & 0xf);

	reg80 = rtl818x_ioread16(priv, &priv->map->RFPinsOutput) & 0xfff3;
	reg82 = rtl818x_ioread16(priv, &priv->map->RFPinsEnable);

	rtl818x_iowrite16(priv, &priv->map->RFPinsEnable, reg82 | 0x7);

	reg84 = rtl818x_ioread16(priv, &priv->map->RFPinsSelect);
	rtl818x_iowrite16(priv, &priv->map->RFPinsSelect, reg84 | 0x7);
	udelay(10);

	rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg80 | (1 << 2));
	udelay(2);
	rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg80);
	udelay(10);

	for (i = 15; i >= 0; i--) {
		u16 reg = reg80 | (bangdata & (1 << i)) >> i;

		if (i & 1)
			rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg);

		rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg | (1 << 1));
		rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg | (1 << 1));

		if (!(i & 1))
			rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg);
	}
{
	struct rtl8187_priv *priv = dev->priv;
	u16 reg80, reg82, reg84;

	reg80 = rtl818x_ioread16(priv, &priv->map->RFPinsOutput);
	reg82 = rtl818x_ioread16(priv, &priv->map->RFPinsEnable);
	reg84 = rtl818x_ioread16(priv, &priv->map->RFPinsSelect);

	reg80 &= ~(0x3 << 2);
	reg84 &= ~0xF;

	rtl818x_iowrite16(priv, &priv->map->RFPinsEnable, reg82 | 0x0007);
	rtl818x_iowrite16(priv, &priv->map->RFPinsSelect, reg84 | 0x0007);
	udelay(10);

	rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg80 | (1 << 2));
	udelay(2);

	rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg80);
	udelay(10);

	mutex_lock(&priv->io_mutex);

	priv->io_dmabuf->bits16 = data;
	usb_control_msg(priv->udev, usb_sndctrlpipe(priv->udev, 0),
			RTL8187_REQ_SET_REG, RTL8187_REQT_WRITE,
			addr, 0x8225, &priv->io_dmabuf->bits16, sizeof(data),
			HZ / 2);

	mutex_unlock(&priv->io_mutex);

	rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg80 | (1 << 2));
	udelay(10);

	rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg80 | (1 << 2));
	rtl818x_iowrite16(priv, &priv->map->RFPinsSelect, reg84);
}

static void rtl8225_write(struct ieee80211_hw *dev, u8 addr, u16 data)
{
	struct rtl8187_priv *priv = dev->priv;

	if (priv->asic_rev)
		rtl8225_write_8051(dev, addr, cpu_to_le16(data));
	else
		rtl8225_write_bitbang(dev, addr, data);
}

static u16 rtl8225_read(struct ieee80211_hw *dev, u8 addr)
{
	struct rtl8187_priv *priv = dev->priv;
	u16 reg80, reg82, reg84, out;
	int i;

	reg80 = rtl818x_ioread16(priv, &priv->map->RFPinsOutput);
	reg82 = rtl818x_ioread16(priv, &priv->map->RFPinsEnable);
	reg84 = rtl818x_ioread16(priv, &priv->map->RFPinsSelect);

	reg80 &= ~0xF;

	rtl818x_iowrite16(priv, &priv->map->RFPinsEnable, reg82 | 0x000F);
	rtl818x_iowrite16(priv, &priv->map->RFPinsSelect, reg84 | 0x000F);

	rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg80 | (1 << 2));
	udelay(4);
	rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg80);
	udelay(5);

	for (i = 4; i >= 0; i--) {
		u16 reg = reg80 | ((addr >> i) & 1);

		if (!(i & 1)) {
			rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg);
			udelay(1);
		}

		rtl818x_iowrite16(priv, &priv->map->RFPinsOutput,
				  reg | (1 << 1));
		udelay(2);
		rtl818x_iowrite16(priv, &priv->map->RFPinsOutput,
				  reg | (1 << 1));
		udelay(2);

		if (i & 1) {
			rtl818x_iowrite16(priv, &priv->map->RFPinsOutput, reg);
			udelay(1);
		}
	}