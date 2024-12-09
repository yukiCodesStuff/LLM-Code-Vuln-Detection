
                  public class BankManager {
                     
                        
                           
                           // user allowed to perform bank manager tasks
                           private User user = null;private boolean isUserAuthentic = false;
                           
                           // constructor for BankManager class
                           public BankManager(String username) {user = getUserFromUserDatabase(username);}
                           
                           // retrieve user from database of users
                           public User getUserFromUserDatabase(String username) {...}
                           
                           // authenticate user
                           public boolean authenticateUser(String username, String password) {
                              if (user == null) {System.out.println("Cannot find user " + username);}else {if (password.equals(user.getPassword())) {isUserAuthentic = true;}}return isUserAuthentic;
                           }
                           
                              
                                 
                                 // methods for performing bank manager tasks
                                 ...
                           
                           
                        
                     }
               
            