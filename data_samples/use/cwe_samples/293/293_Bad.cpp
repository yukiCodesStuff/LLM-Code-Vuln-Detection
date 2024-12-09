
                  String trustedReferer = "http://www.example.com/"while(true){n = read(newsock, buffer, BUFSIZE);requestPacket = processPacket(buffer, n);if (requestPacket.referer == trustedReferer){openNewSecureSession(requestPacket);}}
               
               