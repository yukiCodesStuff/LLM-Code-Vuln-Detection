
                  public class Client {private int UID;public int PID;private String userName;public Client(String userName){PID = getDefaultProfileID();UID = mapUserNametoUID( userName );this.userName = userName;}public void setPID(int ID) {UID = ID;}}
               
               