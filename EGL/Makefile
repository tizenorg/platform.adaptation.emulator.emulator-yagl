TOP_DIR = ..
OBJ_PREFIX = /EGL
TARGET = $(LIB_DIR)/libEGL.so
TARGETS = $(TARGET)

include $(TOP_DIR)/Make.rules

$(TARGET): $(OBJ_DIR) $(DEPS) $(OBJS) $(LIB_DIR)
ifeq ($(V),0)
	@echo [LD] $@
	@$(CC) $(LDFLAGS) -pthread -shared -o $@ $(OBJS) -lX11 -lpthread -lXext -lXfixes -ldl -ldrm -ldrm_vigs
else
	$(CC) $(LDFLAGS) -pthread -shared -o $@ $(OBJS) -lX11 -lpthread -lXext -lXfixes -ldl -ldrm -ldrm_vigs
endif

install:: $(INSTALL_LIB_DIR) $(TARGET)
ifeq ($(V),0)
	@echo [INSTALL] $(TARGET)
	@cp -d $(TARGET) $(INSTALL_LIB_DIR)
	@ln -s -f libEGL.so $(INSTALL_LIB_DIR)/libEGL.so.1
	@ln -s -f libEGL.so.1 $(INSTALL_LIB_DIR)/libEGL.so.1.0
else
	cp -d $(TARGET) $(INSTALL_LIB_DIR)
	ln -s -f libEGL.so $(INSTALL_LIB_DIR)/libEGL.so.1
	ln -s -f libEGL.so.1 $(INSTALL_LIB_DIR)/libEGL.so.1.0
endif

install-strip:: install
ifeq ($(V),0)
	@echo [STRIP] $(TARGET)
	@$(STRIP) $(INSTALL_LIB_DIR)/libEGL.so
else
	$(STRIP) $(INSTALL_LIB_DIR)/libEGL.so
endif
