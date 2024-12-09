#include "node_http2.h"
#include "node_http2_state.h"
#include "node_perf.h"
#include "util-inl.h"

#include <algorithm>

    if (UNLIKELY(!session->CanAddStream() ||
                 Http2Stream::New(session, id, frame->headers.cat) ==
                     nullptr)) {
      // Too many concurrent streams being opened
      nghttp2_submit_rst_stream(**session, NGHTTP2_FLAG_NONE, id,
                                NGHTTP2_ENHANCE_YOUR_CALM);
      return NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE;
    }
  } else if (!stream->IsDestroyed()) {
    stream->StartHeaders(frame->headers.cat);
  }
  return 0;