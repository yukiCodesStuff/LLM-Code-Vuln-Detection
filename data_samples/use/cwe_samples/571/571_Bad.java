
                  
                     public void updateInventory(String productNumber) {
                        boolean isProductAvailable = false;boolean isDelayed = false;
                           if (productInStore(productNumber)) {isProductAvailable = true;updateInStoreDatabase(productNumber);}else if (productInWarehouse(productNumber)) {isProductAvailable = true;updateInWarehouseDatabase(productNumber);}else {isProductAvailable = true;}
                           if ( isProductAvailable ) {updateProductDatabase(productNumber);}else if ( isDelayed ) {
                              
                                 
                                 /* Warn customer about delay before order processing */
                                 ...
                           }
                     }
               
               