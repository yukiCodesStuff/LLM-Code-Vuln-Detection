
                  int foo = 0;int storenum(int num) {static int counter = 0;counter++;if (num > foo) foo = num;return foo;}
               
               