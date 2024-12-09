
                  class A{void foo(bool);};void A::foo(bool heap) {int localArray[2] = {11,22};int *p = localArray;if (heap){p = new int[2];}delete[] p;}
               
            