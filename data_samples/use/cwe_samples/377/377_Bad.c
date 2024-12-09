
                  if (tmpnam_r(filename)) {
                        
                           FILE* tmp = fopen(filename,"wb+");while((recv(sock,recvbuf,DATA_SIZE, 0) > 0)&(amt!=0)) amt = fwrite(recvbuf,1,DATA_SIZE,tmp);
                     }...
               
               