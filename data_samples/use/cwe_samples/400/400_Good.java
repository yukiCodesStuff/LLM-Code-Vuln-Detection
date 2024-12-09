
                  public static final int SERVER_PORT = 4444;public static final int MAX_CONNECTIONS = 10;...
                     public void acceptConnections() {
                     
                        try {ServerSocket serverSocket = new ServerSocket(SERVER_PORT);int counter = 0;boolean hasConnections = true;while (hasConnections) {hasConnections = checkForMoreConnections();Socket client = serverSocket.accept();Thread t = new Thread(new ClientSocketThread(client));t.setName(client.getInetAddress().getHostName() + ":" + counter++);ExecutorService pool = Executors.newFixedThreadPool(MAX_CONNECTIONS);pool.execute(t);}serverSocket.close();
                           
                           } catch (IOException ex) {...}
                     }
               
            