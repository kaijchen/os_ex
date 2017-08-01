#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/list.h>
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

void dfs_tasks(struct task_struct *this_task)
{
	struct task_struct *task;
	struct list_head *list;

	printk(KERN_INFO "%s %d\n",
			this_task->comm,
			task_pid_nr(this_task));

	list_for_each(list, &this_task->children) {
		task = list_entry(list, struct task_struct, sibling);

		dfs_tasks(task);
	}
}

/* This function is called when the module is removed */
void tasks_exit(void)
{
	printk(KERN_INFO "Removing Module\n");
	dfs_tasks(&init_task);
}

/* Marcos for regestering module entry and exit points */
module_init(tasks_init);
module_exit(tasks_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Tasks Module");
MODULE_AUTHOR("ckj");
