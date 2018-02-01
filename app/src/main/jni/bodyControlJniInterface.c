#include <jni.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include "com_aviconics_devicelib_BodyControlCom.h"
#include "bodyControlCom.h"
#include  "flex.h"

#define ANDROID_JNI
#ifdef ANDROID_JNI 
#include "./utils/android_log_print.h"
#else
        #define LOGD(fmt, args...) do {\
                printf (fmt, ##args);\
        } while(0)
#endif

JavaVM* javaVm = NULL;
JNIEnv* rcvJniEnv = NULL;
jclass bodyComClass;
jmethodID ack_data_summit =NULL;

static int msg_ack_to_java (unsigned int cmd, unsigned char *ack_code, int len)
{
        int ret = 0;

        if (1)
        {
                LOGD ("subBoard_ack_to_java recv ######command [%d] ############# len[%d]", cmd,len);
                int i;
                for (i = 0; i < len; i++)
                        LOGD("0x%x ", ack_code [i]);
                LOGD("\n\n");
        }

        if (ack_data_summit == NULL)
        {
        	JavaVMAttachArgs args = { JNI_VERSION_1_6, __FUNCTION__, NULL };
        	JNIEnv* jniEvn = NULL;
        	int res = (*javaVm)->AttachCurrentThread(javaVm, &jniEvn, &args);
        	rcvJniEnv = jniEvn;

        	ack_data_summit = (*jniEvn)->GetStaticMethodID(jniEvn, bodyComClass,
        				"bodyCom_callback_ack_process","(I[BI)I");
        }

        jbyte *jAck = (jbyte *)calloc(len , sizeof(jbyte));
        memcpy(jAck, ack_code, len);
        jbyteArray jArrayAck = (*rcvJniEnv)->NewByteArray(rcvJniEnv, len);
	
        (*rcvJniEnv)->SetByteArrayRegion(rcvJniEnv, jArrayAck, 0, len, jAck);
        //submit to java 
        ret = (jint)(*rcvJniEnv)->CallStaticIntMethod(rcvJniEnv, bodyComClass,
        		ack_data_summit, cmd, jArrayAck, len);
        //	(*jniEnv)->ReleaseStringUTFChars(jniEnv, jstr, name);
        (*rcvJniEnv)->DeleteLocalRef(rcvJniEnv, jArrayAck);
       
        free(jAck);
        //(*jniEnv)->ReleaseByteArrayElements(jniEnv, jArrayAck, jAck, 0);
	
        return ret;
}

JNIEXPORT jint JNICALL Java_com_aviconics_devicelib_BodyControlCom_native_1bodyCom_1msg_1send
  (JNIEnv *env, jobject obj, jint command, jbyteArray cmd_code, jint len)
{
    int ret = 3;
    unsigned char buffer [64];
    int i;

    (*env)->GetByteArrayRegion(env, cmd_code, 0, len, buffer);
	ret = mindpush_bodyCom_command_send(command, buffer, len);

   LOGD("java send board command [%d] len [%d]\n", command, len);
   for (i = 0; i < len; i++)
   	LOGD("0x%x ", buffer [i]);
   LOGD("\n\n");
    return ret; 
}

JNIEXPORT jint JNICALL Java_com_aviconics_devicelib_BodyControlCom_native_1bodyCom_1init
  (JNIEnv *env, jobject obj, jint port, jint baud)
{
    
    (*env)->GetJavaVM(env, &javaVm);

    jclass tmp = (*env)->FindClass(env, "com/aviconics/devicelib/BodyControlCom");
    bodyComClass = (jclass)(*env)->NewGlobalRef(env, tmp);
    if (bodyComClass == NULL)
    {
    	LOGD("get bodyComClass failed\n");

        return -1;
    }

    return minpush_bodyCom_init (port, baud, msg_ack_to_java);
}

JNIEXPORT jint JNICALL Java_com_aviconics_devicelib_BodyControlCom_native_1bodyCom_1exit
  (JNIEnv *env, jobject obj)
{

    return mindpush_bodyCom_exit ();
}
JNIEXPORT jint JNICALL Java_com_aviconics_devicelib_BodyControlCom_native_1flexible_1init
        (JNIEnv *env, jobject obj)
{
    return body_flexible_init();
}
JNIEXPORT jint JNICALL Java_com_aviconics_devicelib_BodyControlCom_native_1flexible
        (JNIEnv *env, jobject obj, jint flexible)
{
     return body_flexible(flexible);
}
JNIEXPORT jint JNICALL Java_com_aviconics_devicelib_BodyControlCom_native_1flexible_1state
        (JNIEnv *env, jobject obj)
{
    return flexible_state();
}
