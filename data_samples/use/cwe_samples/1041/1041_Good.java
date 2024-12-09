
                  public class Main {
                     private double ComplexMath(double r, double s) {
                        //complex math equations
                        double pi = Math.PI;
                        double surface_area = pi * r * s + pi * Math.pow(r, 2);
                        return surface_area;
                     }
                     public static void main(String[] args) {
                        double s = 10.0;
                        double r = 1.0;
                        double surface_area;
                        if(r > 0.0) {
                           surface_area = ComplexMath(r, s);
                        }
                        if(r > 1.0) {
                           surface_area = ComplexMath(r, s);
                        }
                     }
                  }
               
            