    TLSEXT_IDX_cryptopro_bug,
    TLSEXT_IDX_early_data,
    TLSEXT_IDX_certificate_authorities,
    TLSEXT_IDX_quic_transport_params_draft,
    TLSEXT_IDX_quic_transport_params,
    TLSEXT_IDX_padding,
    TLSEXT_IDX_psk,
    /* Dummy index - must always be the last entry */
    TLSEXT_IDX_num_builtins
#define CLIENT_HANDSHAKE_LABEL "CLIENT_HANDSHAKE_TRAFFIC_SECRET"
#define SERVER_HANDSHAKE_LABEL "SERVER_HANDSHAKE_TRAFFIC_SECRET"
#define CLIENT_APPLICATION_LABEL "CLIENT_TRAFFIC_SECRET_0"
#define SERVER_APPLICATION_LABEL "SERVER_TRAFFIC_SECRET_0"
#define EARLY_EXPORTER_SECRET_LABEL "EARLY_EXPORTER_SECRET"
#define EXPORTER_SECRET_LABEL "EXPORTER_SECRET"

#  ifndef OPENSSL_NO_KTLS