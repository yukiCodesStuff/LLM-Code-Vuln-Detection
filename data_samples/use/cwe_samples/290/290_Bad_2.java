
                  String ip = request.getRemoteAddr();InetAddress addr = InetAddress.getByName(ip);if (addr.getCanonicalHostName().endsWith("trustme.com")) {trusted = true;}
               
               