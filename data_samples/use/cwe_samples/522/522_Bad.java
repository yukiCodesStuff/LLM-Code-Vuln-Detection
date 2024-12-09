
                  ...Properties prop = new Properties();prop.load(new FileInputStream("config.properties"));String password = prop.getProperty("password");DriverManager.getConnection(url, usr, password);...
               
               