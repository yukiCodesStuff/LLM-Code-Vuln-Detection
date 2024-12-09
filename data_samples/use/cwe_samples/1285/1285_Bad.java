
                  
                     
                     // Method called from servlet to obtain product information
                     public String displayProductSummary(int index) {
                     
                        String productSummary = new String("");
                           try {String productSummary = getProductSummary(index);
                           
                           } catch (Exception ex) {...}
                           return productSummary;
                     }
                     public String getProductSummary(int index) {return products[index];}
               
               