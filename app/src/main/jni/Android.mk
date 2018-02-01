LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := bodyControlCom
LOCAL_SRC_FILES := uart.c  flex.c modbus.c bodyControlCom.c bodyControlJniInterface.c
#����log������Ӧ��log��
LOCAL_LDLIBS += -llog 

include $(BUILD_SHARED_LIBRARY)