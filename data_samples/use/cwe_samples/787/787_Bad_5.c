
                  int i;unsigned int numWidgets;Widget **WidgetList;
                     numWidgets = GetUntrustedSizeValue();if ((numWidgets == 0) || (numWidgets > MAX_NUM_WIDGETS)) {ExitError("Incorrect number of widgets requested!");}WidgetList = (Widget **)malloc(numWidgets * sizeof(Widget *));printf("WidgetList ptr=%p\n", WidgetList);for(i=0; i<numWidgets; i++) {WidgetList[i] = InitializeWidget();}WidgetList[numWidgets] = NULL;showWidgets(WidgetList);
               
               