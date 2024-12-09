
                 def validate_password(actual_pw, typed_pw):
		 
                   if len(actual_pw) <> len(typed_pw):
		   return 0
                   for i in len(actual_pw):
		   if actual_pw[i] <> typed_pw[i]:
		   return 0
                   
                   return 1
                 
                 
               
               