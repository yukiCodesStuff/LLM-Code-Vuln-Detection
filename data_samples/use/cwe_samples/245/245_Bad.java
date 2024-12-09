
                  public class DatabaseConnection {
                        private static final String CONNECT_STRING = "jdbc:mysql://localhost:3306/mysqldb";private Connection conn = null;
                           public DatabaseConnection() {}
                           public void openDatabaseConnection() {try {conn = DriverManager.getConnection(CONNECT_STRING);} catch (SQLException ex) {...}}
                           // Member functions for retrieving database connection and accessing database...
                     }
               
               