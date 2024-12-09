 **/
namespace node {

#define SECURITY_REVERSIONS(XX)                                            \
  XX(CVE_2024_27980, "CVE-2024-27980", "Unsafe Windows batch file execution")
//  XX(CVE_2016_PEND, "CVE-2016-PEND", "Vulnerability Title")

enum reversion {
#define V(code, ...) SECURITY_REVERT_##code,
  SECURITY_REVERSIONS(V)