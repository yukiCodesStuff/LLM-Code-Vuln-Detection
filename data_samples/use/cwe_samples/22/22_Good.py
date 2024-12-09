
                  
                     import os
                     import sys
                     def main():
                        
                        filename = sys.argv[1]
                        path = os.path.normpath(f"{os.getcwd()}{os.sep}{filename}")
                        try:
                           
                           with open(path, 'r') as f:
                              
                              file_data = f.read()
                              
                           
                        except FileNotFoundError as e:
                           
                           print("Error - file not found")
                           
                        
                     main()
                     
               
               