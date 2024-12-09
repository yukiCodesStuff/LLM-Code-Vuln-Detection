			prom_printf("W=%d H=%d LB=%d addr=0x%x\n",
				    width, height, pitch, addr);
			btext_setup_display(width, height, 8, pitch, addr);
		}
#endif /* CONFIG_PPC_EARLY_DEBUG_BOOTX */
	}
}