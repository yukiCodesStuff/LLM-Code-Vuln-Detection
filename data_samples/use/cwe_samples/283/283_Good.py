
                  def killProcess(processID):
                        user = getCurrentUser()
                           
                           #Check process owner against requesting user
                           if getProcessOwner(processID) == user:os.kill(processID, signal.SIGKILL)return
                           else:print("You cannot kill a process you don't own")return
                        
                     
                  
               
            