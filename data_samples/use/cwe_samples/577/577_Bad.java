
                  @Statelesspublic class StockSymbolBean implements StockSymbolRemote {
                        
                           ServerSocket serverSocket = null;Socket clientSocket = null;
                           public StockSymbolBean() {
                              try {serverSocket = new ServerSocket(Constants.SOCKET_PORT);} catch (IOException ex) {...}
                                 try {clientSocket = serverSocket.accept();} catch (IOException e) {...}
                           }
                           public String getStockSymbol(String name) {...}
                           public BigDecimal getStockValue(String symbol) {...}
                           private void processClientInputFromSocket() {...}
                     }
               
               