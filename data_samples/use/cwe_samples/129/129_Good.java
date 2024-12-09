
                  
                     
                     // Method called from servlet to obtain product information
                     public String displayProductSummary(int index) {
                     
                        String productSummary = new String("");
                           try {String productSummary = getProductSummary(index);
                           
                           } catch (Exception ex) {...}
                           return productSummary;
                     }
                     public String getProductSummary(int index) {
                        String productSummary = "";
                           if ((index >= 0) && (index < MAX_PRODUCTS)) {productSummary = products[index];}else {System.err.println("index is out of bounds");throw new IndexOutOfBoundsException();}
                           return productSummary;
                     }
               
               