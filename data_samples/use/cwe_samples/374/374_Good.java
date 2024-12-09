
                  ...public void updateSalesAndInventoryForBookSold(String bookISBN) {
                        
                           
                           // Get book object from inventory using ISBN
                           Book book = inventory.getBookWithISBN(bookISBN);
                           // Create copy of book object to make sure contents are not changed
                           Book bookSold = (Book) book.clone();
                           // update sales information for book sold
                           sales.updateSalesInformation(bookSold);
                           // update inventory
                           inventory.updateInventory(book);
                     }...
               
            