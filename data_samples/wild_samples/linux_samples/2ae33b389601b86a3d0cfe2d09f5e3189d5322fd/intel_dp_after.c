		switch (intel_dig_port->port) {
		case PORT_A:
			ch_ctl = DPA_AUX_CH_CTL;
			break;
		case PORT_B:
			ch_ctl = PCH_DPB_AUX_CH_CTL;
			break;
		case PORT_C:
			ch_ctl = PCH_DPC_AUX_CH_CTL;
			break;
		case PORT_D:
			ch_ctl = PCH_DPD_AUX_CH_CTL;
			break;
		default:
			BUG();
		}
	}

#define C (((status = I915_READ_NOTRACE(ch_ctl)) & DP_AUX_CH_CTL_SEND_BUSY) == 0)
	if (has_aux_irq)
		done = wait_event_timeout(dev_priv->gmbus_wait_queue, C,
					  msecs_to_jiffies(10));
	else
		done = wait_for_atomic(C, 10) == 0;
	if (!done)
		DRM_ERROR("dp aux hw did not signal timeout (has irq: %i)!\n",
			  has_aux_irq);
#undef C

	return status;
}

static int
intel_dp_aux_ch(struct intel_dp *intel_dp,
		uint8_t *send, int send_bytes,
		uint8_t *recv, int recv_size)
{
	uint32_t output_reg = intel_dp->output_reg;
	struct intel_digital_port *intel_dig_port = dp_to_dig_port(intel_dp);
	struct drm_device *dev = intel_dig_port->base.base.dev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	uint32_t ch_ctl = output_reg + 0x10;
	uint32_t ch_data = ch_ctl + 4;
	int i, ret, recv_bytes;
	uint32_t status;
	uint32_t aux_clock_divider;
	int try, precharge;
	bool has_aux_irq = INTEL_INFO(dev)->gen >= 5 && !IS_VALLEYVIEW(dev);

	/* dp aux is extremely sensitive to irq latency, hence request the
	 * lowest possible wakeup latency and so prevent the cpu from going into
	 * deep sleep states.
	 */
	pm_qos_update_request(&dev_priv->pm_qos, 0);

	if (IS_HASWELL(dev)) {
		switch (intel_dig_port->port) {
		case PORT_A:
			ch_ctl = DPA_AUX_CH_CTL;
			ch_data = DPA_AUX_CH_DATA1;
			break;
		case PORT_B:
			ch_ctl = PCH_DPB_AUX_CH_CTL;
			ch_data = PCH_DPB_AUX_CH_DATA1;
			break;
		case PORT_C:
			ch_ctl = PCH_DPC_AUX_CH_CTL;
			ch_data = PCH_DPC_AUX_CH_DATA1;
			break;
		case PORT_D:
			ch_ctl = PCH_DPD_AUX_CH_CTL;
			ch_data = PCH_DPD_AUX_CH_DATA1;
			break;
		default:
			BUG();
		}
	}