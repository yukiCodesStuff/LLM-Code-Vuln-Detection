/*
 * Copyright 2015-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
                SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
                return SUB_STATE_ERROR;
            }
            if (confunc != NULL) {
                int tmpret;

                tmpret = confunc(s, &pkt);
                if (tmpret <= 0) {
                    WPACKET_cleanup(&pkt);
                    check_fatal(s);
                    return SUB_STATE_ERROR;
                } else if (tmpret == 2) {
                    /*
                     * The construction function decided not to construct the
                     * message after all and continue. Skip sending.
                     */
                    WPACKET_cleanup(&pkt);
                    st->write_state = WRITE_STATE_POST_WORK;
                    st->write_state_work = WORK_MORE_A;
                    break;
                } /* else success */
            }
            if (!ssl_close_construct_packet(s, &pkt, mt)
                    || !WPACKET_finish(&pkt)) {
                WPACKET_cleanup(&pkt);