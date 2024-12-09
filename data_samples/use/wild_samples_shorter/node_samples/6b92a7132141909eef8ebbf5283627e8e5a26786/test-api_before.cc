}


#ifndef WIN32
class ThreadInterruptTest {
 public:
  ThreadInterruptTest() : sem_(NULL), sem_value_(0) { }