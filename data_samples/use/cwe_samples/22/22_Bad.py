
                  
                  import os
                  import sys
                  def main():
                     
                     filename = sys.argv[1]
                     path = os.path.join(os.getcwd(), filename)
                     try:
                        
                        with open(path, 'r') as f:
                           
                           file_data = f.read()
                           
                        
                     except FileNotFoundError as e:
                        
                        print("Error - file not found")
                        
                     
                  main()
                  
               
               