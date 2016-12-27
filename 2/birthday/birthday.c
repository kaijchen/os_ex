#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/slab.h>

struct birthday {
	int day;
	int month;
	int year;
	struct list_head list;
};

static LIST_HEAD(birthday_list);

int add_person(int y, int m, int d)
{
	struct birthday *person;

	person = kmalloc(sizeof(*person), GFP_KERNEL);

	if (person == NULL)
		return -1;

	person->day = d;
	person->month = m;
	person->year = y;
	INIT_LIST_HEAD(&person->list);

	/* add this person to the end of birthday_list */
	list_add_tail(&person->list, &birthday_list);

	return 0;
}


/* This function is called when the module is loaded */
int birthday_init(void)
{
	struct birthday *ptr;

	printk(KERN_INFO "Loading Module\n");

	/* add 5 persons */
	add_person(1969, 12, 28);
	add_person(1957, 12, 4);
	add_person(1953, 3, 16);
	add_person(1943, 2, 4);
	add_person(1941, 9, 9);

	list_for_each_entry(ptr, &birthday_list, list) {
		/* on each iteration ptr points */
		/* to the next birthday struct */
		printk(KERN_INFO "added person %d-%d-%d\n",
				ptr->year,
				ptr->month,
				ptr->day);
	}
	return 0;
}

/* This function is called when the module is removed */
void birthday_exit(void)
{
	struct birthday *ptr, *next;

	printk(KERN_INFO "Removing Module\n");

	list_for_each_entry_safe(ptr, next, &birthday_list, list) {
		printk(KERN_INFO "deleting person %d-%d-%d\n",
				ptr->year,
				ptr->month,
				ptr->day);
		list_del(&ptr->list);
		kfree(ptr);
	}
}

/* Marcos for regestering module entry and exit points */
module_init(birthday_init);
module_exit(birthday_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Birthday Module");
MODULE_AUTHOR("ckj");
