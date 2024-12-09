namespace node {

#define SECURITY_REVERSIONS(XX)                                            \
  XX(CVE_2019_9514, "CVE-2019-9514", "HTTP/2 Reset Flood")                 \
  XX(CVE_2019_9516, "CVE-2019-9516", "HTTP/2 0-Length Headers Leak")       \
  XX(CVE_2019_9518, "CVE-2019-9518", "HTTP/2 Empty DATA Frame Flooding")   \
//  XX(CVE_2016_PEND, "CVE-2016-PEND", "Vulnerability Title")