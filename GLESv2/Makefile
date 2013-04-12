TOP_DIR = ..
OBJ_PREFIX = /GLESv2
TARGET = $(LIB_DIR)/libGLESv2.so
TARGETS = $(TARGET)

include $(TOP_DIR)/Make.rules

include $(TOP_DIR)/Make.gles_common

OBJS += $(GLES_COMMON_OBJS)

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
	@ln -s -f libGLESv2.so $(INSTALL_LIB_DIR)/libGLESv2.so.1
	@ln -s -f libGLESv2.so.1 $(INSTALL_LIB_DIR)/libGLESv2.so.1.0
else
	cp -d $(TARGET) $(INSTALL_LIB_DIR)
	ln -s -f libGLESv2.so $(INSTALL_LIB_DIR)/libGLESv2.so.1
	ln -s -f libGLESv2.so.1 $(INSTALL_LIB_DIR)/libGLESv2.so.1.0
endif

install-strip:: install
ifeq ($(V),0)
	@echo [STRIP] $(TARGET)
	@$(STRIP) $(INSTALL_LIB_DIR)/libGLESv2.so
else
	$(STRIP) $(INSTALL_LIB_DIR)/libGLESv2.so
endif
