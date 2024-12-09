
                  String ctl = request.getParameter("ctl");Class cmdClass = Class.forName(ctl + "Command");Worker ao = (Worker) cmdClass.newInstance();ao.checkAccessControl(request);ao.doAction(request);
               
               