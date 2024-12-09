
                #define NAME_TYPE 1#define ID_TYPE 2
                struct MessageBuffer{int msgType;union {char *name;int nameID;};};
                
                int main (int argc, char **argv) {
                struct MessageBuffer buf;char *defaultMessage = "Hello World";
                buf.msgType = NAME_TYPE;buf.name = defaultMessage;printf("Pointer of buf.name is %p\n", buf.name);
                /* This particular value for nameID is used to make the code architecture-independent. If coming from untrusted input, it could be any value. */
                
                buf.nameID = (int)(defaultMessage + 1);printf("Pointer of buf.name is now %p\n", buf.name);if (buf.msgType == NAME_TYPE) {printf("Message: %s\n", buf.name);}else {printf("Message: Use ID %d\n", buf.nameID);}
                }
              
              