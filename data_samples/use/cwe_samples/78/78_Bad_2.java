
                  ...String btype = request.getParameter("backuptype");String cmd = new String("cmd.exe /K \"c:\\util\\rmanDB.bat "+btype+"&&c:\\utl\\cleanup.bat\"")
                     System.Runtime.getRuntime().exec(cmd);...
               
               