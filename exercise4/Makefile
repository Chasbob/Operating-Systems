KERNELDIR=/lib/modules/`uname -r`/build
#ARCH=i386
#KERNELDIR=/usr/src/kernels/`uname -r`-i686

MODULES = firewallExtension.ko findExecutable.ko kernelWrite.ko 
 
obj-m += firewallExtension.o findExecutable.o kernelWrite.o

all:
	make -C  $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean

install:	
	make -C $(KERNELDIR) M=$(PWD) modules_install

firewallSetup:
	gcc firewallSetup.c -o Setup/firewallSetup 

quickInstall:
	cp $(MODULES) /lib/modules/`uname -r`/extra
