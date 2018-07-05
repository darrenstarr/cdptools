# Diary of developing this module

## Intro and rant

Well, this was a bit of a marathon. I started on Sunday morning and 3 days and 6,000 lines of code later, I was pretty happy with the results. I'm pretty sure I could have done a better job on /proc/net/cdp/summary and /proc/net/cdp/details but to be honest, I don't really care about those. Welcome to the open source world.

Let's talk about me a little. I'm a programmer, been doing it a while and I have an absolute sick anti-compiled language obsession. I honestly get sick just thinking of all the massive security problems related to this sick need everyone has of writing horrible code, compiling it and cursing the world with it. Let's consider the massive number of security holes which exist out there because instead of updating a Just-In-Time compiler/runtime environment on computers through a patch, instead, we compile our security problems and bugs into our software and we end up with CPU firmware patches that slow computers down by up to 30%... because we turn the CPU into a JIT when we should have used a JIT to begin with.

I HATE kernel modules. I'm sitting here compiling a Linux kernel to debug from as I type this and I'll be waiting a really long time because someone in their brilliance decided I need to compile 10 trillion files for 10 trillion features for a kernel that will run in a VM that doesn't need more than the basics and a small handful of drivers. Anyone who submits a pull request to add anything not absolutely required for booting the system into the kernel should be maimed. It's absolutely inexcusable. For example, a USB network adapter that can't be used for network booting... it doesn't belong there. My code... doesn't belong there. Make a package, a snap, whatever and let it be downloaded as a package. Why should I have to sit and wait for it to download?

What about code for providing an FCoE target? It actually doesn't work. There are multiple generations of FCoE target code in the kernel which all don't work. I know. I tried. All of them. And in addition to that, the firmware for the cards which they should in theory have worked with is not kept up to date. So, there's simply no point to me having to waste time even testing to see whether I should compile it.

Then there's C. My goodness. Now that Rust is a kernel grade programming language and Redox is proof of this, C doesn't have any remaining reason to exist. It's a truly disgusting language. I mean seriously. Let's explain.

  1) It uses a preprocessor .... for code!

  2) It requires forward declarations because C has no concept of programs... or modules... it's a file by file system that generates shit code that can't be inlined across files.

  3) We need header files ... for everything. And since C has no idea what a project is, I'll spend days compiling that are wasted time and effort to generate substandard code.

  4) C doesn't know what a string is. I'm serious. It's 2018 and C still doesn't have an actual string. I mean, why would you need a string right? Not everyone uses them? And let's not forget that because C doesn't have strings and is generally pointer hell, we're entirely dependent on static code analysis and tools like Valgrind to compensate for the occasional failure to null terminate or a plethora of other problems associated with it.

  5) C doesn't have generics. Oh wait... of course it does, you can write entire programs... in #define statements.

You know what, I can go on an on with rants about how this world is unfair and writing code like an animal in a late 1960's hell called C and *NIX but it's not about that. We use Linux because it's a force of nature. It keeps moving forward no matter what. And while Linux generally is a nightmare of utterly wasted hours, it probably is still the best tool for the job.

## My environment

I have a Windows machine. I love my Windows machine. It's Windows 10 with WSL running Ubuntu. The problem with WSL is that it's a replacement for Linux, not an actual Linux. To understand this, consider that WSL presents the Linux kernel API to everything on top of it. So while most everything you'll ever need from Ubuntu (or other distros) run 100% unchanged on top of a Microsoft built "Linux-Like" kernel subsystem, it's not actually Linux. Which means that it's useful for absolutely everything except writing kernel modules.

I also run Hyper-V. I like it. It's nice for Docker and Sarah Cooley (@VirtualScooley) is possibly the most impressive and passionate program manager I've ever seen. I think she has an actual addiction to making Hyper-V kick ass.

So, I needed a development environment for writing a Linux kernel module on Windows for a Raspberry Pi. That should be easy enough right? Well, it wasn't bad.

### Step 1 - User Space

CDP is a Type-Length-Value protocol without nesting. It's a 4-byte header for version, TTL and checksum followed by a series of TLVs. Those TLVs are somewhat consistently formated, but they're not well documented. Thus far, I've been using Wireshark for most of my documentation. So, it was important to make a solid C library for reading CDP packets and storing the data. So, I did. 