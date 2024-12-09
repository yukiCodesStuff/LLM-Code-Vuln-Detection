
                  public void acceptConnections() {
                     
                        try {ServerSocket serverSocket = new ServerSocket(SERVER_PORT);int counter = 0;boolean hasConnections = true;while (hasConnections) {Socket client = serverSocket.accept();Thread t = new Thread(new ClientSocketThread(client));t.setName(client.getInetAddress().getHostName() + ":" + counter++);t.start();}serverSocket.close();
                           
                           } catch (IOException ex) {...}
                     }
               
               