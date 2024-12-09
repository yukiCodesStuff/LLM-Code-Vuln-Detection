
                  public class BankAccount {
                        
                           
                           // private member variables of BankAccount class
                           private String accountOwnerName;private String accountOwnerSSN;private int accountNumber;private double balance;
                           
                           // constructor for BankAccount class
                           public BankAccount(String accountOwnerName, String accountOwnerSSN,int accountNumber, double initialBalance, int initialRate){this.accountOwnerName = accountOwnerName;this.accountOwnerSSN = accountOwnerSSN;this.accountNumber = accountNumber;this.balance = initialBalance;this.start(initialRate);}
                           
                           // start method will add interest to balance every 30 days
                           
                           
                           // creates timer object and interest adding action listener object
                           public void start(double rate){ActionListener adder = new InterestAdder(rate);Timer t = new Timer(1000 * 3600 * 24 * 30, adder);t.start();}
                           
                           // InterestAdder is an inner class of BankAccount class
                           
                           
                           // that implements the ActionListener interface
                           private class InterestAdder implements ActionListener{
                              private double rate;
                                 public InterestAdder(double aRate){this.rate = aRate;}
                                 public void actionPerformed(ActionEvent event){
                                    
                                       // update interest
                                       double interest = BankAccount.this.balance * rate / 100;BankAccount.this.balance += interest;
                                 }
                              
                           }
                     }
               
               