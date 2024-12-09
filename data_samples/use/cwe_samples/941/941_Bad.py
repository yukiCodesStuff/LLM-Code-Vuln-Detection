
                  sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)sock.bind( (UDP_IP,UDP_PORT) )while true:
                        data = sock.recvfrom(1024)if not data:break
                           (requestIP, nameToResolve) = parseUDPpacket(data)record = resolveName(nameToResolve)sendResponse(requestIP,record)
                     
                  
               
               