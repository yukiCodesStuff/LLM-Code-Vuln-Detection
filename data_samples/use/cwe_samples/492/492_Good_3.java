
                  public class UrlToolApplet extends Applet implements ActionListener {
                        
                           
                           // private member variables for applet components
                           private Label enterUrlLabel;private TextField enterUrlTextField;private Button submitButton;
                           
                           // init method that adds components to applet
                           public void init() {setLayout(new FlowLayout());enterUrlLabel = new Label("Enter URL: ");enterUrlTextField = new TextField("", 20);submitButton = new Button("Submit");add(enterUrlLabel);add(enterUrlTextField);add(submitButton);submitButton.addActionListener(this);}
                           
                           // implementation of actionPerformed method of ActionListener interface
                           public void actionPerformed(ActionEvent evt) {
                              if (evt.getSource() == submitButton) {String urlString = enterUrlTextField.getText();URL url = null;try {url = new URL(urlString);} catch (MalformedURLException e) {System.err.println("Malformed URL: " + urlString);}if (url != null) {getAppletContext().showDocument(url);}}
                           }
                     }
               
            