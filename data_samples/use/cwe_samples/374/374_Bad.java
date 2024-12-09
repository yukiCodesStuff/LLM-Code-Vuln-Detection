
                  public class BookStore {
                        private BookStoreInventory inventory;private SalesDBManager sales;...
                           // constructor for BookStore
                           public BookStore() {this.inventory = new BookStoreInventory();this.sales = new SalesDBManager();...}public void updateSalesAndInventoryForBookSold(String bookISBN) {
                              
                                 
                                 // Get book object from inventory using ISBN
                                 Book book = inventory.getBookWithISBN(bookISBN);
                                 // update sales information for book sold
                                 sales.updateSalesInformation(book);
                                 // update inventory
                                 inventory.updateInventory(book);
                           }
                           // other BookStore methods
                           ...
                     }public class Book {private String title;private String author;private String isbn;
                        // Book object constructors and get/set methods
                        ...}
               
               