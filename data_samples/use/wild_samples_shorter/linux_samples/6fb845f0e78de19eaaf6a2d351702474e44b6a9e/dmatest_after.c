			srcs[i] = um->addr[i] + src_off;
			ret = dma_mapping_error(dev->dev, um->addr[i]);
			if (ret) {
				result("src mapping error", total_tests,
				       src_off, dst_off, len, ret);
				goto error_unmap_continue;
			}
			um->to_cnt++;
		}
		/* map with DMA_BIDIRECTIONAL to force writeback/invalidate */
					       DMA_BIDIRECTIONAL);
			ret = dma_mapping_error(dev->dev, dsts[i]);
			if (ret) {
				result("dst mapping error", total_tests,
				       src_off, dst_off, len, ret);
				goto error_unmap_continue;
			}
			um->bidi_cnt++;
		}

		}

		if (!tx) {
			result("prep error", total_tests, src_off,
			       dst_off, len, ret);
			msleep(100);
			goto error_unmap_continue;
		}

		done->done = false;
		tx->callback = dmatest_callback;
		cookie = tx->tx_submit(tx);

		if (dma_submit_error(cookie)) {
			result("submit error", total_tests, src_off,
			       dst_off, len, ret);
			msleep(100);
			goto error_unmap_continue;
		}
		dma_async_issue_pending(chan);

		wait_event_freezable_timeout(thread->done_wait, done->done,

		status = dma_async_is_tx_complete(chan, cookie, NULL, NULL);

		if (!done->done) {
			result("test timed out", total_tests, src_off, dst_off,
			       len, 0);
			goto error_unmap_continue;
		} else if (status != DMA_COMPLETE) {
			result(status == DMA_ERROR ?
			       "completion error status" :
			       "completion busy status", total_tests, src_off,
			       dst_off, len, ret);
			goto error_unmap_continue;
		}

		dmaengine_unmap_put(um);

		if (params->noverify) {
			verbose_result("test passed", total_tests, src_off,
				       dst_off, len, 0);
			continue;
			verbose_result("test passed", total_tests, src_off,
				       dst_off, len, 0);
		}

		continue;

error_unmap_continue:
		dmaengine_unmap_put(um);
		failed_tests++;
	}
	ktime = ktime_sub(ktime_get(), ktime);
	ktime = ktime_sub(ktime, comparetime);
	ktime = ktime_sub(ktime, filltime);