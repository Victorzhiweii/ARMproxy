ArmProxy: main.o crc.o cJSON.o profile.o sock.o uart.o command.o
	arm-linux-gnueabihf-gcc -o ArmProxy main.o crc.o cJSON.o profile.o sock.o uart.o command.o -lpthread -lm
uart.o: uart.c
	arm-linux-gnueabihf-gcc -c uart.c -Wformat -Wmissing-format-attribute -Wmissing-include-dirs -Wunused-value -Wunused-variable -Wunused-but-set-parameter
main.o: main.c
	arm-linux-gnueabihf-gcc -c main.c -Wformat -Wmissing-format-attribute -Wmissing-include-dirs -Wunused-value -Wunused-variable -Wunused-but-set-parameter
sock.o: sock.c
	arm-linux-gnueabihf-gcc -c sock.c -Wformat -Wmissing-format-attribute -Wmissing-include-dirs -Wunused-value -Wunused-variable -Wunused-but-set-parameter
crc.o: crc.c
	arm-linux-gnueabihf-gcc -c crc.c -Wformat -Wmissing-format-attribute -Wmissing-include-dirs -Wunused-value -Wunused-variable -Wunused-but-set-parameter
cJSON.o: cJSON.c
	arm-linux-gnueabihf-gcc -c cJSON.c -Wformat -Wmissing-format-attribute -Wmissing-include-dirs -Wunused-value -Wunused-variable -Wunused-but-set-parameter
profile.o: profile.c
	arm-linux-gnueabihf-gcc -c profile.c 
command.o: command.c
	arm-linux-gnueabihf-gcc -c command.c 
clean:
	rm ArmProxy
distclean: clean
	find -name '*.o' | xargs rm -f
