namespace node {

#define SECURITY_REVERSIONS(XX)                                            \
//  XX(CVE_2016_PEND, "CVE-2016-PEND", "Vulnerability Title")

enum reversion {
#define V(code, ...) SECURITY_REVERT_##code,