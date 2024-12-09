
               XPath xpath = XPathFactory.newInstance().newXPath();XPathExpression xlogin = xpath.compile("//users/user[login/text()='" + login.getUserName() + "' and password/text() = '" + login.getPassword() + "']/home_dir/text()");Document d = DocumentBuilderFactory.newInstance().newDocumentBuilder().parse(new File("db.xml"));String homedir = xlogin.evaluate(d);
             
             