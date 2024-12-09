
                  #define PRIV_ADMIN 0#define PRIV_REGULAR 1typedef struct{int privileges;int id;} user_t;user_t *Add_Regular_Users(int num_users){user_t* users = (user_t*)calloc(num_users, sizeof(user_t));int i = num_users;while( --i && (users[i].privileges = PRIV_REGULAR) ){users[i].id = i;}return users;}int main(){user_t* test;int i;test = Add_Regular_Users(25);for(i = 0; i < 25; i++) printf("user %d has privilege level %d\n", test[i].id, test[i].privileges);}
               
               