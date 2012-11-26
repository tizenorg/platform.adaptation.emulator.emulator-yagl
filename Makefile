TOP_DIR = .

include $(TOP_DIR)/Make.rules

SUBDIRS = EGL GLESv1_CM GLESv2

$(EVERYTHING)::
	@for n in $(SUBDIRS); do $(MAKE) -C $$n $@ || exit 1; done
