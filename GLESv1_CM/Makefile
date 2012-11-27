TOP_DIR = ..
OBJ_PREFIX = /GLESv1_CM
TARGET = $(LIB_DIR)/libGLESv1_CM.so
TARGETS = $(TARGET)

include $(TOP_DIR)/Make.rules

include $(TOP_DIR)/Make.gles_common

OBJS += $(GLES_COMMON_OBJS)
DEPS += $(GLES_COMMON_DEPS)

$(TARGET): $(OBJ_DIR) $(DEPS) $(OBJS) $(LIB_DIR)
ifeq ($(V),0)
	@echo [LD] $@
	@$(CC) $(LDFLAGS) -pthread -shared -o $@ $(OBJS) -lEGL -lpthread
else
	$(CC) $(LDFLAGS) -pthread -shared -o $@ $(OBJS) -lEGL -lpthread
endif

install:: $(INSTALL_LIB_DIR) $(TARGET)
ifeq ($(V),0)
	@echo [INSTALL] $(TARGET)
	@cp -d $(TARGET) $(INSTALL_LIB_DIR)
	@ln -s -f libGLES_CM.so $(INSTALL_LIB_DIR)/libGLES_CM.so.1
	@ln -s -f libGLES_CM.so.1 $(INSTALL_LIB_DIR)/libGLES_CM.so.1.4.5
else
	cp -d $(TARGET) $(INSTALL_LIB_DIR)
	ln -s -f libGLES_CM.so $(INSTALL_LIB_DIR)/libGLES_CM.so.1
	ln -s -f libGLES_CM.so.1 $(INSTALL_LIB_DIR)/libGLES_CM.so.1.4.5
endif

install-strip:: install
ifeq ($(V),0)
	@echo [STRIP] $(TARGET)
	@$(STRIP) $(INSTALL_LIB_DIR)/libGLES_CM.so
else
	$(STRIP) $(INSTALL_LIB_DIR)/libGLES_CM.so
endif
