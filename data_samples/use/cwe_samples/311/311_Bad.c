
                  server.sin_family = AF_INET; hp = gethostbyname(argv[1]);if (hp==NULL) error("Unknown host");memcpy( (char *)&server.sin_addr,(char *)hp->h_addr,hp->h_length);if (argc < 3) port = 80;else port = (unsigned short)atoi(argv[3]);server.sin_port = htons(port);if (connect(sock, (struct sockaddr *)&server, sizeof server) < 0) error("Connecting");...while ((n=read(sock,buffer,BUFSIZE-1))!=-1) {
                        
                           write(dfd,password_buffer,n);...
                        
                     
                  
               
               