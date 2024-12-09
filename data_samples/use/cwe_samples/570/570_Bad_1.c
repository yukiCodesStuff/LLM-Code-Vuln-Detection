
                  int updateInventory(char* productNumber, int numberOfItems) {
                     
                        int initCount = getProductCount(productNumber);
                           int updatedCount = initCount + numberOfItems;
                           int updated = updateProductCount(updatedCount);
                           // if statement for debugging purposes onlyif (1 == 0) {
                              
                                 char productName[128];productName = getProductName(productNumber);
                                 printf("product %s initially has %d items in inventory \n", productName, initCount);printf("adding %d items to inventory for %s \n", numberOfItems, productName);
                                 if (updated == 0) {printf("Inventory updated for product %s to %d items \n", productName, updatedCount);}
                                 else {printf("Inventory not updated for product: %s \n", productName);}
                              
                           }
                           return updated;
                     }
                  
               
               