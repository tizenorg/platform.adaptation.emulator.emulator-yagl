TOP_DIR = ..
OBJ_PREFIX = /EGL
TARGET = $(LIB_DIR)/libEGL.so
TARGETS = $(TARGET)

include $(TOP_DIR)/Make.rules

$(TARGET): $(OBJ_DIR) $(DEPS) $(OBJS) $(LIB_DIR)
ifeq ($(V),0)
	@echo [LD] $@
	@$(CC) $(LDFLAGS) -pthread -shared -o $@ $(OBJS) -lX11 -lpthread -lXext -ldl
else
	$(CC) $(LDFLAGS) -pthread -shared -o $@ $(OBJS) -lX11 -lpthread -lXext -ldl
endif

install:: $(INSTALL_LIB_DIR) $(TARGET)
ifeq ($(V),0)
	@echo [INSTALL] $(TARGET)
	@cp -d $(TARGET) $(INSTALL_LIB_DIR)
	@ln -s -f libEGL.so $(INSTALL_LIB_DIR)/libEGL.so.1
	@ln -s -f libEGL.so.1 $(INSTALL_LIB_DIR)/libEGL.so.1.4
else
	cp -d $(TARGET) $(INSTALL_LIB_DIR)
	ln -s -f libEGL.so $(INSTALL_LIB_DIR)/libEGL.so.1
	ln -s -f libEGL.so.1 $(INSTALL_LIB_DIR)/libEGL.so.1.4
endif

install-strip:: install
ifeq ($(V),0)
	@echo [STRIP] $(TARGET)
	@$(STRIP) $(INSTALL_LIB_DIR)/libEGL.so
else
	$(STRIP) $(INSTALL_LIB_DIR)/libEGL.so
endif
