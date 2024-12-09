
                  $transfer_amount = GetTransferAmount();$balance = GetBalanceFromDatabase();
                     if ($transfer_amount < 0) {FatalError("Bad Transfer Amount");}$newbalance = $balance - $transfer_amount;if (($balance - $transfer_amount) < 0) {FatalError("Insufficient Funds");}SendNewBalanceToDatabase($newbalance);NotifyUser("Transfer of $transfer_amount succeeded.");NotifyUser("New balance: $newbalance");
               
               