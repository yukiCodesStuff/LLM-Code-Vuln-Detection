
                  #define SIZE 50int *tos, *p1, stack[SIZE];
                     void push(int i) {
                        p1++;if(p1==(tos+SIZE)) {
                              
                                 
                                 // Print stack overflow error message and exit
                                 
                              
                           }*p1 == i;
                     }
                     int pop(void) {
                        if(p1==tos) {
                              
                                 
                                 // Print stack underflow error message and exit
                                 
                              
                           }p1--;return *(p1+1);
                     }
                     int main(int argc, char *argv[]) {
                        
                           
                           // initialize tos and p1 to point to the top of stack
                           tos = stack;p1 = stack;
                           // code to add and remove items from stack
                           ...return 0;
                     }
               
               