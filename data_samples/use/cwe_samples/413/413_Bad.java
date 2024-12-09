
                  public class BankAccount {
                        
                           
                           // variable for bank account balance
                           private double accountBalance;
                           
                           // constructor for BankAccount
                           public BankAccount() {accountBalance = 0;}
                           
                           // method to deposit amount into BankAccount
                           public void deposit(double depositAmount) {
                              
                                 double newBalance = accountBalance + depositAmount;accountBalance = newBalance;
                           }
                           
                           // method to withdraw amount from BankAccount
                           public void withdraw(double withdrawAmount) {
                              
                                 double newBalance = accountBalance - withdrawAmount;accountBalance = newBalance;
                           }
                           
                           // other methods for accessing the BankAccount object
                           ...
                     }
               
               