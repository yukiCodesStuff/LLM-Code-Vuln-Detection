
                  public class BankAccount {
                        
                           public final int MAXIMUM_WITHDRAWAL_LIMIT = 350;
                           
                           // variable for bank account balance
                           private double accountBalance;
                           
                           // constructor for BankAccount
                           public BankAccount() {accountBalance = 0;}
                           
                           // method to deposit amount into BankAccount
                           public void deposit(double depositAmount) {...}
                           
                           // method to withdraw amount from BankAccount
                           public void withdraw(double withdrawAmount) {
                              
                                 if (withdrawAmount < MAXIMUM_WITHDRAWAL_LIMIT) {
                                    
                                       double newBalance = accountBalance - withdrawAmount;accountBalance = newBalance;
                                 }else {System.err.println("Withdrawal amount exceeds the maximum limit allowed, please try again...");...}
                           }
                           
                           // other methods for accessing the BankAccount object
                           ...
                     }
               
               