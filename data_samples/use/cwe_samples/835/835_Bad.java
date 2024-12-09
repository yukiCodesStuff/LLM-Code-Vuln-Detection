
                  public boolean isReorderNeeded(String bookISBN, int rateSold) {
                        
                           boolean isReorder = false;
                           int minimumCount = 10;int days = 0;
                           
                           // get inventory count for book
                           int inventoryCount = inventory.getIventoryCount(bookISBN);
                           
                           // find number of days until inventory count reaches minimum
                           while (inventoryCount > minimumCount) {
                              
                                 inventoryCount = inventoryCount - rateSold;days++;
                              
                           }
                           
                           // if number of days within reorder timeframe
                           
                           
                           // set reorder return boolean to true
                           if (days > 0 && days < 5) {isReorder = true;}
                           return isReorder;
                     }
               
               