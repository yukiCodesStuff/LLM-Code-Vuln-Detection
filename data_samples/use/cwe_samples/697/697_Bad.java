
                  public class Truck {
                        private String make;private String model;private int year;
                           public boolean equals(Object o) {
                              if (o == null) return false;if (o == this) return true;if (!(o instanceof Truck)) return false;
                                 Truck t = (Truck) o;
                                 return (this.make.equals(t.getMake()) && this.model.equals(t.getModel()));
                           }
                     }
               
               