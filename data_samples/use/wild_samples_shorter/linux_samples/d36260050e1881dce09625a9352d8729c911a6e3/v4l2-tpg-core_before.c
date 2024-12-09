	case V4L2_PIX_FMT_NV42:
		buf[0][offset] = r_y_h;
		buf[1][2 * offset] = b_v;
		buf[1][(2 * offset + 1) %8] = g_u_s;
		break;

	case V4L2_PIX_FMT_YUYV:
		buf[0][offset] = r_y_h;