
                  while(true) {DatagramPacket rp = new DatagramPacket(rData,rData.length);outSock.receive(rp);InetAddress IPAddress = rp.getAddress();int port = rp.getPort();out = secret.getBytes();DatagramPacket sp =new DatagramPacket(out, out.length, IPAddress, port);outSock.send(sp);}
               
               