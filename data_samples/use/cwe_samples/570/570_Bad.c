
                  #define BIT_READ 0x0001 // 00000001#define BIT_WRITE 0x0010 // 00010000
                     unsigned int mask = BIT_READ & BIT_WRITE; /* intended to use "|" */
                     // using "&", mask = 00000000// using "|", mask = 00010001
                     // determine if user has read and write accessint hasReadWriteAccess(unsigned int userMask) {
                        // if the userMask has read and write bits set// then return 1 (true)if (userMask & mask) {return 1;}
                           // otherwise return 0 (false)return 0;
                     }
               
               