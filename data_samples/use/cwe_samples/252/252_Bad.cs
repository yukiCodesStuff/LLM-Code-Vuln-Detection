
                  char[] byteArray = new char[1024];for (IEnumerator i=users.GetEnumerator(); i.MoveNext() ;i.Current()) {String userName = (String) i.Current();String pFileName = PFILE_ROOT + "/" + userName;StreamReader sr = new StreamReader(pFileName);sr.Read(byteArray,0,1024);//the file is always 1k bytessr.Close();processPFile(userName, byteArray);}
               
               