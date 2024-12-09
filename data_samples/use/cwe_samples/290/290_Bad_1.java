
                  while(true) {
                        DatagramPacket rp=new DatagramPacket(rData,rData.length);outSock.receive(rp);String in = new String(p.getData(),0, rp.getLength());InetAddress clientIPAddress = rp.getAddress();int port = rp.getPort();
                           if (isTrustedAddress(clientIPAddress) & secretKey.equals(in)) {out = secret.getBytes();DatagramPacket sp =new DatagramPacket(out,out.length, IPAddress, port); outSock.send(sp);}
                     }
               
               