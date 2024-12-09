
                  def dispatchCommand(command, user, args):
                        if command == 'Login':loginUser(args)return
                           
                           
                           # user has requested a file
                           if command == 'Retrieve_file':
			   
			   if authenticated(user) and ownsFile(user,args):
			   
			   sendFile(args)return
			   
                           if command == 'List_files':listFiles(args)return
                           
                           ...
                           
                        
                     
                  
               
               