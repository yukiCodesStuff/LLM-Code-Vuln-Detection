
                
                  def main():
                  
                    sum = 0
                    numbers = input("Enter a space-separated list of numbers: ").split(" ")
                    try:
                    
                      for num in numbers:
                      
                        sum = sum + int(num)
                      
                      print(f"Sum of {numbers} = {sum}")
                    
                    except ValueError:
                    
                      print("Error: invalid input")
                    
                  
                  main()
                
              
              