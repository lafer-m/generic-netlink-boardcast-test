
obj-m += link_agent.o

link_agent-objs := \
	link_agent_test.o \
	netlink.o

# add our custom header file
# EXTRA_CFLAGS=-I$(PWD)/../include/s

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean