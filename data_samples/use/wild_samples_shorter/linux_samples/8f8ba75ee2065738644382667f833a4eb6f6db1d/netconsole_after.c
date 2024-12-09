				 * rtnl_lock already held
				 */
				if (nt->np.dev) {
					__netpoll_cleanup(&nt->np);
					dev_put(nt->np.dev);
					nt->np.dev = NULL;
				}
				nt->enabled = 0;
				stopped = true;
				break;