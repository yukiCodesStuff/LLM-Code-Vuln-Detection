
                  @Statelesspublic class StockSymbolBean extends Thread implements StockSymbolRemote {
                        
                           ServerSocket serverSocket = null;Socket clientSocket = null;boolean listening = false;
                           public StockSymbolBean() {
                              try {serverSocket = new ServerSocket(Constants.SOCKET_PORT);} catch (IOException ex) {...}
                                 listening = true;while(listening) {start();}
                           }
                           public String getStockSymbol(String name) {...}
                           public BigDecimal getStockValue(String symbol) {...}
                           public void run() {try {clientSocket = serverSocket.accept();} catch (IOException e) {...}...}
                        
                     }
               
               