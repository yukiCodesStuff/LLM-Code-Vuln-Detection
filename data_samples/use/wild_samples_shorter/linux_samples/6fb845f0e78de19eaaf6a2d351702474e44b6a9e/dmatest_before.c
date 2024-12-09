			srcs[i] = um->addr[i] + src_off;
			ret = dma_mapping_error(dev->dev, um->addr[i]);
			if (ret) {
				dmaengine_unmap_put(um);
				result("src mapping error", total_tests,
				       src_off, dst_off, len, ret);
				failed_tests++;
				continue;
			}
			um->to_cnt++;
		}
		/* map with DMA_BIDIRECTIONAL to force writeback/invalidate */
					       DMA_BIDIRECTIONAL);
			ret = dma_mapping_error(dev->dev, dsts[i]);
			if (ret) {
				dmaengine_unmap_put(um);
				result("dst mapping error", total_tests,
				       src_off, dst_off, len, ret);
				failed_tests++;
				continue;
			}
			um->bidi_cnt++;
		}

		}

		if (!tx) {
			dmaengine_unmap_put(um);
			result("prep error", total_tests, src_off,
			       dst_off, len, ret);
			msleep(100);
			failed_tests++;
			continue;
		}

		done->done = false;
		tx->callback = dmatest_callback;
		cookie = tx->tx_submit(tx);

		if (dma_submit_error(cookie)) {
			dmaengine_unmap_put(um);
			result("submit error", total_tests, src_off,
			       dst_off, len, ret);
			msleep(100);
			failed_tests++;
			continue;
		}
		dma_async_issue_pending(chan);

		wait_event_freezable_timeout(thread->done_wait, done->done,

		status = dma_async_is_tx_complete(chan, cookie, NULL, NULL);

		dmaengine_unmap_put(um);

		if (!done->done) {
			result("test timed out", total_tests, src_off, dst_off,
			       len, 0);
			failed_tests++;
			continue;
		} else if (status != DMA_COMPLETE) {
			result(status == DMA_ERROR ?
			       "completion error status" :
			       "completion busy status", total_tests, src_off,
			       dst_off, len, ret);
			failed_tests++;
			continue;
		}

		if (params->noverify) {
			verbose_result("test passed", total_tests, src_off,
				       dst_off, len, 0);
			continue;
			verbose_result("test passed", total_tests, src_off,
				       dst_off, len, 0);
		}
	}
	ktime = ktime_sub(ktime_get(), ktime);
	ktime = ktime_sub(ktime, comparetime);
	ktime = ktime_sub(ktime, filltime);