#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>

MODULE_LICENSE("GPL");

int handler_cookie;
int net_dev_irq_no = 19;
const char* net_dev_name = "ens33";

irqreturn_t handler(int interrupt_no, void* cookie){
    printk("NIC device sent interrupt\n");
    return IRQ_NONE;
}

static int __init interrupt_init(void){
    int result = request_irq(net_dev_irq_no, handler, IRQF_SHARED, net_dev_name, &handler_cookie);

    if(result){
        printk("Unable to register interrupt handler for NIC device\n");
    }

    return result;
}

static void __exit interrupt_exit(void){
    free_irq(net_dev_irq_no, &handler_cookie);
}

module_init(interrupt_init);
module_exit(interrupt_exit);