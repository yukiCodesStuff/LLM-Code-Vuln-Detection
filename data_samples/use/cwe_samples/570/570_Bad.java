
                  
                     public void updateUserAccountOrder(String productNumber, String accountNumber) {
                        boolean isValidProduct = false;boolean isValidAccount = false;
                           if (validProductNumber(productNumber)) {isValidProduct = true;updateInventory(productNumber);}else {return;}
                           if (validAccountNumber(accountNumber)) {isValidProduct = true;updateAccount(accountNumber, productNumber);}
                           if (isValidProduct && isValidAccount) {updateAccountOrder(accountNumber, productNumber);}
                     }
               
               