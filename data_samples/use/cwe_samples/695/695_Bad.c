
                  #include <jni.h>#include "Echo.h"//the java class above compiled with javah#include <stdio.h>
                     JNIEXPORT void JNICALLJava_Echo_runEcho(JNIEnv *env, jobject obj){char buf[64];gets(buf);printf(buf);}
               
               