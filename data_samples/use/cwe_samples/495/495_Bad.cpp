
                  class Color{private:int[2] colorArray;int colorValue;public:Color () : colorArray { 1, 2 }, colorValue (3) { };int[2] & fa () { return colorArray; } // return reference to private arrayint & fv () { return colorValue; } // return reference to private integer};int main (){Color c;c.fa () [1] = 42; // modifies private array elementc.fv () = 42; // modifies private intreturn 0;}
               
            