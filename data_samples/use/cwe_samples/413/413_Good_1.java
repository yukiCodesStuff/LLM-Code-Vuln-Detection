
                  public class BankAccount {
                        ...
                           // lock object for thread access to methods
                           private ReentrantLock balanceChangeLock;
                           
                           // condition object to temporarily release lock to other threads
                           private Condition sufficientFundsCondition;
                           
                           // method to deposit amount into BankAccount
                           public void deposit(double amount) {
                              
                                 
                                 // set lock to block access to BankAccount from other threads
                                 balanceChangeLock.lock();try {
                                    double newBalance = balance + amount;balance = newBalance;
                                       
                                       // inform other threads that funds are available
                                       sufficientFundsCondition.signalAll();
                                    
                                 } catch (Exception e) {...}finally {// unlock lock objectbalanceChangeLock.unlock();}
                           }
                           
                           // method to withdraw amount from bank account
                           public void withdraw(double amount) {
                              
                                 
                                 // set lock to block access to BankAccount from other threads
                                 balanceChangeLock.lock();try {
                                    while (balance < amount) {
                                          
                                             
                                             // temporarily unblock access
                                             
                                             
                                             // until sufficient funds are available
                                             sufficientFundsCondition.await();
                                       }double newBalance = balance - amount;balance = newBalance;
                                    
                                 } catch (Exception e) {...}finally {// unlock lock objectbalanceChangeLock.unlock();}
                           }...
                     }
               
            