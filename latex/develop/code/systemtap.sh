yum install systemtap kernel-devel yum-utils
debuginfo-install kernel

# git clone git://sourceware.org/git/systemtap.git

stap -ve 'probe begin { log("hello world") exit () }'
stap -c df -e 'probe syscall.* { if (target()==pid()) log(name." ".argstr) }'
