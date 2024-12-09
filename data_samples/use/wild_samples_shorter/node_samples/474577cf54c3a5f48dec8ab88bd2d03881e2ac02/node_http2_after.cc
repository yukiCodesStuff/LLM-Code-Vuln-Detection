#include "node_http2.h"
#include "node_http2_state.h"
#include "node_perf.h"
#include "node_revert.h"
#include "util-inl.h"

#include <algorithm>

    if (UNLIKELY(!session->CanAddStream() ||
                 Http2Stream::New(session, id, frame->headers.cat) ==
                     nullptr)) {
      if (session->rejected_stream_count_++ > 100 &&
          !IsReverted(SECURITY_REVERT_CVE_2019_9514)) {
        return NGHTTP2_ERR_CALLBACK_FAILURE;
      }
      // Too many concurrent streams being opened
      nghttp2_submit_rst_stream(**session, NGHTTP2_FLAG_NONE, id,
                                NGHTTP2_ENHANCE_YOUR_CALM);
      return NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE;
    }

    session->rejected_stream_count_ = 0;
  } else if (!stream->IsDestroyed()) {
    stream->StartHeaders(frame->headers.cat);
  }
  return 0;