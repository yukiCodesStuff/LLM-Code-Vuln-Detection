		if (odd) {
			buf[1][0] = (buf[1][0] + b_v) / 2;
			buf[1][1] = (buf[1][1] + g_u_s) / 2;
			break;
		}
		buf[1][0] = b_v;
		buf[1][1] = g_u_s;
		break;

	case V4L2_PIX_FMT_YUV444M:
		buf[0][offset] = r_y_h;
		buf[1][offset] = g_u_s;
		buf[2][offset] = b_v;
		break;

	case V4L2_PIX_FMT_YVU444M:
		buf[0][offset] = r_y_h;
		buf[1][offset] = b_v;
		buf[2][offset] = g_u_s;
		break;

	case V4L2_PIX_FMT_NV24:
		buf[0][offset] = r_y_h;
		buf[1][2 * offset] = g_u_s;
		buf[1][(2 * offset + 1) % 8] = b_v;
		break;

	case V4L2_PIX_FMT_NV42:
		buf[0][offset] = r_y_h;
		buf[1][2 * offset] = b_v;
		buf[1][(2 * offset + 1) % 8] = g_u_s;
		break;

	case V4L2_PIX_FMT_YUYV:
		buf[0][offset] = r_y_h;
		if (odd) {
			buf[0][1] = (buf[0][1] + g_u_s) / 2;
			buf[0][3] = (buf[0][3] + b_v) / 2;
			break;
		}