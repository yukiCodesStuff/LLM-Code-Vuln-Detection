/*
 * Copyright 2015-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
                SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
                return SUB_STATE_ERROR;
            }
            if (confunc != NULL && !confunc(s, &pkt)) {
                WPACKET_cleanup(&pkt);
                check_fatal(s);
                return SUB_STATE_ERROR;
            }
            if (!ssl_close_construct_packet(s, &pkt, mt)
                    || !WPACKET_finish(&pkt)) {
                WPACKET_cleanup(&pkt);