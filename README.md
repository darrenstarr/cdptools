# Linux CDP Client Module

## About

This is a project which aims to implement the Cisco Discovery Protocol for use on Linux. It's not unique as there have been many other implementations before it. But it is implemented as a proper "first-class" SNAP citizen and does not depend on performing all packet filtering in user mode.

The current state of the project is that it appears to work quite well on Ethernet interfaces and has been tested against several devices. It is currently running in the lab on Raspberry Pis and on virtual machines.

## Information

This module produces new files in /proc/net

### /proc/net/cdp/summary

This is similar to the "show cdp neighbor" command on the Cisco command prompt

### /proc/net/cdp/detail

This is similar to the "show cdp entry *" command on the Cisco command prompt

### /proc/net/cdp/json

This is a dump of all the known information from the CDP neighbor database in JSON format. I wrote the JSON generator myself as C is an archaic language with no real object model to write a proper serializer for.
However, I believe the JSON output to be usable and will make patches if I find scenarios where it is not.

## Design

The design of this module is that code which is Linux kernel specific is in the directory /module and the vast majority of the code to make CDP work is abstracted into /libcdp.

The design is very focused on C++ style coding. Everything is a class. Every class has constuctors and destructors. Almost all property setters are abstracted into functions. In some cases, the getters are also functions.
The code is documented, though sometimes slightly inconsistently. There are some cases where more memory allocation then is considered "proper C coding" occurs. This will have obvious performance impacts. But since the code
is typically run no more than 12 times a minute, it shouldn't be a major consideration. There is some copy code in place, but rarely for large buffers. Packet construction occurs in-place on sk_buffs.

### Development platform

This project was developed mostly in Visual Studio 2017 using the Linux C++ target aimed at Windows Subsystem for Linux. The module code was developed in Visual Studio Code on Windows with Linux running in VMware virtual machine.
I'm planning on switching to QEMU eventually in order to be able to run Hyper-V at the same time as a Linux debug kernel which doesn't seem to want to boot for me on Hyper-V.

#### Platforms - Linux Kernel

The files under /module were developed almost entirely from Visual Studio Code on Windows and the script in the root of the project remote_build.sh when called with the IP address of the remote host will upload and compile the module
there for testing. I recommend using SSH keys instead of username and password to do this as it can become very painful over time

```
# Upload the project to a remote host and compile it
./remote_build.sh user@192.168.1.1
```

#### Platforms - Linux user mode

The libcdp directory compiles as user mode code on Linux and has been developed using Visual Studio 2017 with the C++ for Linux cross platform profile. It works really well once you configure the connection to the remote host.
I used my local host running Ubuntu on Windows Subsystem for Linux.

The libcdp.tests directory uses Google Tests unit testing framework. It compiles and is tested on Linux on WSL and on real Ubuntu 18.04 and Raspbian (current as of this writing).

```
cd libcdp.tests
mkdir build
cd build
cmake ..
make
valgrind --leak-check=yes ./libcdptests
``` 

#### Platforms - Windows

The libcdp directory compiles as a project within the solution in Visual Studio 2017 with Visual C++ and native Windows libraries. This was done for editing, refactoring, profiling and testing. As 95% or more of the code resides
in the libcdp directory, it is all able to be run and tested on the Windows platform.

The libcdp.tests directory uses Google Tests unit testing framework and runs as a project with live unit testing in Visual Studio 2017.

### Minimal dependence on macros

In libcdp I have not used any Linux macro disasters like list or rbtree because I didn't want to spend my whole life looking for BSD versions that I can include in my code when I do switch to user mode. Though to be honest, as soon
as I move to user mode, I'm dumping all the C code I can in favor of C# (which I've also already mostly written).

The libcdp component is a classic case of NMH (not made here) as I simply can't trust licensing or headaches associated with using other people's C libraries. All the platform code is in /libcdp/platform and is very very slim. I've
managed to include only the absolute basics of what I need. I did depend on things like the standard C libraries and the only confusing code in the cross-platform code (which uses macros ARG!!!) is the socket address structures which
for some reason the kernel developers and C library developers couldn't seem to standardize naming on... even though the structures are basically identical.

In any case, libcdp is pretty self-contained and for the most part should be pretty solid. I'm not going to introduce more macros. If I get the chance, I'll eliminate as many as I can from what is already in the project. This is 2018
and while I'm not a huge fan of wasted and unnecessary overhead, I do believe that programming in the preprocessor is a quick path to being bombarded with stupid bugs.

## 802.2 LLC SNAP

CDP is very confusing design. In order to make it almost 100% uniform across all layer-2 networks, as will Spanning Tree Protocol, it is implemented using SNAP to provide support for a new layer-3 protocol (CDP) in a uniform fashion
across multiple interface types. The problem with this however is that SNAP depends on LLC which is an 802.2 protocol instead of an 802.3 protocol. The result being that the module PSNAP must be used on Linux to support SNAP
multiplexing. As SNAP is only partially developed on Linux and uses a non-UNIXy means of sending and receiving packets, this would mean that CDP would need its own address family on top of SNAP to work in user space. As such, I've
coded everything in C and made a kernel module.

The good news is that the CDP code is completely abstracted, so at some point, it should be possible to move the libcdp code into user space and instead implement AF_CDP instead. The downside is, I'm not really sure where to even
begin on making such an extension to the kernel. Once I've hardened the library and I've gotten past some milestones, I'll investigate this as an option but if I can make this module stable, I most likely won't bother.

## Possible caveats

### Memory leaks

I am still learning modern kernel coding and I have programmed in C since the previous millennium. As such, I am more or less lost when it comes to memory leak detection in the kernel. I'm working on this though. There is almost no
memory allocation in the /module directory.

*** Update ***

I've ported the lib Windows, instrumented Google tests on Windows and Linux user mode. I've run against Valgrind and so far, things are looking very positive.

### Valgrind/Electric Fence/etc...

I have managed (as seen in the platforms section above) to start unit testing with Valgrind on Linux and WSL and have verified that a chunk of the code in the library is fully able to run without memory leaks, bounds issues, overflows,
underruns, etc... at least for the functions I've tested so far. I'll be performing most of my remaining work from within the unit testing framework and will continue to thoroughly test for memory leaks.

I do not have a solution for verifying memory usage in the Linux kernel since I've had absolutely no success compiling a kernel that will actually boot on my machine let alone with one instrumented for memory leak detection. If
necessary, I'll pay someone to build me a virtual machine that is able to do this as I've already wasted far too many hours with this nonsense.

### User mode vs. Kernel mode

This code is all kernel mode at this time. This is sadly because I don't know enough about kernel development to create a new address family for sockets. I'm not even sure if this can be done using a simple kernel module that isn't
submitted for a pull request.

### Scrubbing

I hate the idea of scrubbing as it's generally reserved for trying to cause issues with code through a blind and brute force method. As this is a kernel module, it is worth while to look into this.

### MAC addresses flooding/spoofing

It is theoretically possible that if a device running this code is connected to a non-CDP aware broadcast domain and someone were to intentionally spoof a lot of packets into the broadcast domain, it could become very memory
intensive for the kernel. A good idea would be to limit the number of known neighbors either globally and/or per interface.

### Configuration

There are no real configuration settings at this time. I don't really understand the kernel module mechanisms for setting configuration. From what I have been told, there is some sort of configuration API that has been introduced
to the kernel tree. As such, I'll consider whether to just make parameters or whether to use the configuration interface when the decision becomes interesting to me. At this time, I simply enable CDP on all Ethernet (or wireless)
interfaces on the device as this fits my needs.

### IPv6 in the address list

Currently, the code does not enumerate and include IPv6 addresses in the transmitted packets. This should be corrected soon.

### Licensing

I'm going to speak with legal following the summer vacation. Up until now, we have talked about MIT or BSD licensing all our code, and in the case of libcdp, this shouldn't be a problem. However, in the case of
the kernel module, this will likely have to be GPL or LGPL. We have no idea how to work with the GPL. Altogether, there is very little code which is Linux kernel specific in this project and as such, I don't
want to be burdened with the legal overhead of the GPL. I can easily see this costing me a hundred or more working hours to sort out. So, we'll see what we can do. Obviously anything which must be licensed under
the GPL will be, but we'll minimize that in favor of an open license like MIT on all the non-module code.
