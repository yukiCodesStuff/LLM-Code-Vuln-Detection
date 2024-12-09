static int __init smbios_present(const char __iomem *p)
{
	u8 buf[32];
	int offset = 0;

	memcpy_fromio(buf, p, 32);
	if ((buf[5] < 32) && dmi_checksum(buf, buf[5])) {
		dmi_ver = (buf[6] << 8) + buf[7];
			dmi_ver = 0x0206;
			break;
		}
		offset = 16;
	}
	return dmi_present(buf + offset);
}

void __init dmi_scan_machine(void)
{