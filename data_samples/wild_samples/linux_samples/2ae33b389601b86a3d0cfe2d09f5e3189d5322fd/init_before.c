	while (count--) {
		u8 iaddr = nv_ro08(bios, init->offset + 0);
		u8 idata = nv_ro08(bios, init->offset + 1);

		trace("\t[0x%02x] = 0x%02x\n", iaddr, idata);
		init->offset += 2;

		init_wr32(init, dreg, idata);
		init_mask(init, creg, ~mask, data | idata);
	}
}

/**
 * INIT_IO_RESTRICT_PLL2 - opcode 0x4a
 *
 */
static void
init_io_restrict_pll2(struct nvbios_init *init)
{
	struct nouveau_bios *bios = init->bios;
	u16 port = nv_ro16(bios, init->offset + 1);
	u8 index = nv_ro08(bios, init->offset + 3);
	u8  mask = nv_ro08(bios, init->offset + 4);
	u8 shift = nv_ro08(bios, init->offset + 5);
	u8 count = nv_ro08(bios, init->offset + 6);
	u32  reg = nv_ro32(bios, init->offset + 7);
	u8  conf, i;

	trace("IO_RESTRICT_PLL2\t"
	      "R[0x%06x] =PLL= ((0x%04x[0x%02x] & 0x%02x) >> 0x%02x) [{\n",
	      reg, port, index, mask, shift);
	init->offset += 11;

	conf = (init_rdvgai(init, port, index) & mask) >> shift;
	for (i = 0; i < count; i++) {
		u32 freq = nv_ro32(bios, init->offset);
		if (i == conf) {
			trace("\t%dkHz *\n", freq);
			init_prog_pll(init, reg, freq);
		} else {
			trace("\t%dkHz\n", freq);
		}
		init->offset += 4;
	}