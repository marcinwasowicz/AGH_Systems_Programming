#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

MODULE_LICENSE("GPL");

int handler_cookie;
int net_dev_irq_no = 19;
const char* net_dev_name = "ens33";

struct tasklet_struct my_tasklet;
unsigned long tasklet_data = 10;

struct timer_list my_timer;
unsigned long expires =  300;
u32 timer_flags = 40;

void timer_handler(struct timer_list* timer){
    printk("Timer correctly executed\n");
}

void tasklet_handler(unsigned long data){
    printk("Tasklet executed with data: %ld\n", data);
    int ret_val = mod_timer(&my_timer, jiffies + msecs_to_jiffies(expires));
    if(ret_val){
        printk("Timer is currently active, cannot schedule\n");
    }
}

irqreturn_t handler(int interrupt_no, void* cookie){
    printk("NIC device sent interrupt\n");
    tasklet_schedule(&my_tasklet);
    return IRQ_NONE;
}

static int __init interrupt_init(void){
    timer_setup(&my_timer, timer_handler, timer_flags);
    tasklet_init(&my_tasklet, tasklet_handler, tasklet_data);
    
    int result = request_irq(net_dev_irq_no, handler, IRQF_SHARED, net_dev_name, &handler_cookie);

    if(result){
        printk("Unable to register interrupt handler for NIC device\n");
    }
    return result;
}

static void __exit interrupt_exit(void){
    free_irq(net_dev_irq_no, &handler_cookie);
    tasklet_kill(&my_tasklet);
    del_timer(&my_timer);
}

module_init(interrupt_init);
module_exit(interrupt_exit);