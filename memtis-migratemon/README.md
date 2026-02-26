# How to use `migratemon` kernel module

## Compiling the module
Compiling the module is simple. Go to the module directory and run `make`.

## Dependencies
This module depends on 
- `CONFIG_SCHEDSTATS` and please make sure it is enabled in your kernel config. 
- After loading the module, it is necessary to enable it at runtime using `echo 1 | sudo tee /proc/sys/kernel/sched_schedstats`.

## Statistics provided
This module will provide information about `kmigraterd` threads at `/sys/kernel/mm/htmm/kmigraterdX`