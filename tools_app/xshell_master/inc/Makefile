SUBDIRS=$(shell ls -l | grep ^d |awk '{print $$9}')

SOURCE=${wildcard *.c}
OBJS=${patsubst %.c, %.o, $(SOURCE)}

all:$(SUBDIRS) $(OBJS)
$(SUBDIRS):ECHO
#===========compile===========
$(OBJS):%.o:%.c
	$(CC) $(CFLAGS) -o $(OBJ_DIR)/$@ -c $^
ECHO:
	@echo $(SUBDIRS)
