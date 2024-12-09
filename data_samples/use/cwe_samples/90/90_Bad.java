
                  context = new InitialDirContext(env);String searchFilter = "StreetAddress=" + address;NamingEnumeration answer = context.search(searchBase, searchFilter, searchCtls);
               
               