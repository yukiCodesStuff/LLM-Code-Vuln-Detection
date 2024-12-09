
                  ...
                     // update sales database for number of product sold with product ID
                     public void updateSalesForProduct(String productID, int amountSold) {
                        
                           
                           // get the total number of products in inventory database
                           int productCount = inventory.getProductCount(productID);
                           // convert integer values to short, the method for the
                           
                           
                           // sales object requires the parameters to be of type short
                           short count = (short) productCount;short sold = (short) amountSold;
                           // update sales database for product
                           sales.updateSalesCount(productID, count, sold);
                     }...
               
               