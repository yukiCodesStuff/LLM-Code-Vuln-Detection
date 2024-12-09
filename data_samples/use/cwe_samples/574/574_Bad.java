
                  @Entitypublic class Customer implements Serializable {
                     
                        private String id;private String firstName;private String lastName;private Address address;
                           public Customer() {...}
                           public Customer(String id, String firstName, String lastName) {...}
                           @Idpublic String getCustomerId() {...}
                           public synchronized void setCustomerId(String id) {...}
                           public String getFirstName() {...}
                           public synchronized void setFirstName(String firstName) {...}
                           public String getLastName() {...}
                           public synchronized void setLastName(String lastName) {...}
                           @OneToOne()public Address getAddress() {...}
                           public synchronized void setAddress(Address address) {...}
                     }
               
               