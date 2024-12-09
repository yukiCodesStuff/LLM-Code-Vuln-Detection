
                  protected void doPost (HttpServletRequest req, HttpServletResponse res) throws IOException {String ip = req.getRemoteAddr();InetAddress addr = InetAddress.getByName(ip);...out.println("hello " + addr.getHostName());}
               
               