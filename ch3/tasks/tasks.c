#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>

/* This function is called when the module is loaded */
int tasks_init(void)
{
	struct task_struct *task;

	printk(KERN_INFO "Loading Module\n");

	for_each_process(task) {
		printk(KERN_INFO "%s %d\n",
				task->comm,
				task_pid_nr(task));
	}

	return 0;
}

/* This function is called when the module is removed */
void tasks_exit(void)
{
	printk(KERN_INFO "Removing Module\n");
}

/* Marcos for regestering module entry and exit points */
module_init(tasks_init);
module_exit(tasks_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Tasks Module");
MODULE_AUTHOR("ckj");
