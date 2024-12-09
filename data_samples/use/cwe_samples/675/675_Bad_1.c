
               void bind_socket(void) {
               
                 int server_sockfd;int server_len;struct sockaddr_in server_address;
                 
                 /*unlink the socket if already bound to avoid an error when bind() is called*/
                 
                 unlink("server_socket");server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
                 server_address.sin_family = AF_INET;server_address.sin_port = 21;server_address.sin_addr.s_addr = htonl(INADDR_ANY);server_len = sizeof(struct sockaddr_in);
               bind(server_sockfd, (struct sockaddr *) &s1, server_len);
               }
             
             