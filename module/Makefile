obj-m += cdp.o
cdp-objs := \
	cdp_module.o \
	cdp_proc.o \
	cdp_proc_detail.o \
	cdp_proc_summary.o \
	cdp_receive.o \
	cdp_neighbor.o \
	../libcdp/buffer_stream.o \
	../libcdp/cdp_packet.o \
	../libcdp/cdp_packet_parser.o \
	../libcdp/cisco_hello_protocol.o \
	../libcdp/ip_address_array.o \
	../libcdp/ip_prefix_array.o \
	../libcdp/ip_prefix.o \
	../libcdp/power_over_ethernet_availability.o \
	../libcdp/stream_reader.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean

