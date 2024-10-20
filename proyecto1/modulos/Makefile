obj-m += sysinfo.o    

all:  
	make -C /lib/modules/$(shell uname -r)/build M="/home/dennis/Sistemaoperativos/e-SO1_2S2024_201700747/proyecto1/modulos" modules  

clean:  
	make -C /lib/modules/$(shell uname -r)/build M="/home/dennis/Sistemaoperativos/e-SO1_2S2024_201700747/proyecto1/modulos" clean
