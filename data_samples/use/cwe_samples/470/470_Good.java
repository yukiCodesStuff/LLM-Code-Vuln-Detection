
                  String ctl = request.getParameter("ctl");Worker ao = null;if (ctl.equals("Add")) {ao = new AddCommand();}else if (ctl.equals("Modify")) {ao = new ModifyCommand();}else {throw new UnknownActionError();}ao.doAction(request);
               
               