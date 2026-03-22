SUBDIRS=$(shell ls -l | grep ^d |awk  '{print $$9}')
SRC_DIR:=$(shell find $(PRO_DIR) -maxdepth 3 -type d)
src=$(foreach dir, $(SRC_DIR), $(wildcard $(dir)/*.c))
obj=$(patsubst %.c, %.o, $(notdir $(src)))

all:$(SUBDIRS) $(obj)

$(SUBDIRS):ECHO
	make -C $@
#==============compile============= 
$(obj):%.o:%.c#变量替换.C生成对应的.O文件
	$(CC) $(CFLAGS) -o $(OBJ_DIR)/$@ -c $^
ECHO:
	@echo $(SUBDIRS)
