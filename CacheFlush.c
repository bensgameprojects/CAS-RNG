#include <jni.h>
#include <x86intrin.h>
#include "CacheFlush.h"

JNIEXPORT jint JNICALL Java_CacheFlush_clfsh
(JNIEnv *env, jobject obj, jobject ref){
  //do cache flush method here:
  //first get the arguments
  //get long addr arg
  //cast as char*
  //char* address  = (char*)addr;
  //call the cacheflush
  _mm_mfence();
  _mm_clflush(ref);
}
