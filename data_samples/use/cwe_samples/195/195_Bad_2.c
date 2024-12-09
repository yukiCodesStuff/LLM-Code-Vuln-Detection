
                  DataPacket *packet;int numHeaders;PacketHeader *headers;
                     sock=AcceptSocketConnection();ReadPacket(packet, sock);numHeaders =packet->headers;
                     if (numHeaders > 100) {ExitError("too many headers!");}headers = malloc(numHeaders * sizeof(PacketHeader);ParsePacketHeaders(packet, headers);
               
               