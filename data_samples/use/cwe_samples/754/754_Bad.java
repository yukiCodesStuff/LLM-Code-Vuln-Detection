
                  FileInputStream fis;byte[] byteArray = new byte[1024];for (Iterator i=users.iterator(); i.hasNext();) {
                        String userName = (String) i.next();String pFileName = PFILE_ROOT + "/" + userName;FileInputStream fis = new FileInputStream(pFileName);fis.read(byteArray); // the file is always 1k bytesfis.close();processPFile(userName, byteArray);
                        
                     
                  
               
               