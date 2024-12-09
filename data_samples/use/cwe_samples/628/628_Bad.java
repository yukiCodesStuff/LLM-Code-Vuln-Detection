
                  private static final String[] ADMIN_ROLES = ...;public boolean void accessGranted(String resource, String user) {String[] userRoles = getUserRoles(user);return accessGranted(resource, ADMIN_ROLES);}
                     private boolean void accessGranted(String resource, String[] userRoles) {
                        
                           
                           // grant or deny access based on user roles
                           ...
                     }
               
            