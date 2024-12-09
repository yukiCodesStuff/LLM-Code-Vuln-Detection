
                  ...Properties prop = new Properties();prop.load(new FileInputStream("config.properties"));String password = Base64.decode(prop.getProperty("password"));DriverManager.getConnection(url, usr, password);...
               
               