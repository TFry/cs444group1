/*
 * elevator sstf
 */
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

struct sstf_data {
	struct list_head queue;
	char direction;
	sector_t sector;
};

static void sstf_merged_requests(struct request_queue *q, struct request *rq,
				 struct request *next)
{
	list_del_init(&next->queuelist);
}

static int sstf_dispatch(struct request_queue *q, int force)
{
	struct sstf_data *nd = q->elevator->elevator_data;
	
	// printk("Inside dispatch function.\n");
	if (!list_empty(&nd->queue)) {
		struct request *rq, *prev, *next;

		// printk("List not empty.\n");
		
		// get previous and next nodes
		prev = list_entry(nd->queue.prev, struct request, queuelist);
		next = list_entry(nd->queue.next, struct request, queuelist);

		// just use next if there's only one request
		rq = next;

		if (prev != next)
                {
			// printk("There are more than one request.\n");

			if (nd->direction == 'B')
			{
				// printk("Going backward in queue.\n");

				// if prev position is less than nd, dispatch prev
				if (nd->sector >= blk_rq_pos(prev))
				{
					rq = prev;
				}

				// otherwise, dispatch next
				else
				{
					rq = next;
					nd->direction = 'F';
				}
			}
			
			else
			{
				// printk("Going forward in queue.\n");

				// if next position is less than nd, dispatch next
				if (nd->sector >= blk_rq_pos(next))
				{
					rq = next;
				}

				else
				{
					rq = prev;
					nd->direction = 'B';
				}
			}
		}

		list_del_init(&rq->queuelist);
		elv_dispatch_add_tail(q, rq);
		nd->sector = blk_rq_sectors(rq) + blk_rq_pos(rq);
		// printk("Dispatched.\n");
		return 1;
	}
	return 0;
}

static void sstf_add_request(struct request_queue *q, struct request *rq)
{
	struct sstf_data *nd = q->elevator->elevator_data;
	struct request *prev, *next;

	// printk("Inside add_request function.\n");

	if (!list_empty(&nd->queue))
	{
		// printk("List not empty, insertion sort.\n");

		// get previous and next nodes
		prev = list_entry(nd->queue.prev, struct request, queuelist);
		next = list_entry(nd->queue.next, struct request, queuelist);

		// iterating until correct position is found
		while (blk_rq_pos(next) <= blk_rq_pos(rq))
		{
			prev = list_entry(prev->queuelist.prev, struct request, queuelist);
			next = list_entry(next->queuelist.next, struct request, queuelist);	
		}
		
		// add request to list
		list_add(&rq->queuelist, &prev->queuelist);
	}

	else
	{
		// printk("List is empty, just add.\n");

		list_add(&rq->queuelist, &nd->queue);
	}

	// printk("Request added.\n");
}

static struct request *
sstf_former_request(struct request_queue *q, struct request *rq)
{
	struct sstf_data *nd = q->elevator->elevator_data;

	if (rq->queuelist.prev == &nd->queue)
		return NULL;
	return list_entry(rq->queuelist.prev, struct request, queuelist);
}

static struct request *
sstf_latter_request(struct request_queue *q, struct request *rq)
{
	struct sstf_data *nd = q->elevator->elevator_data;

	if (rq->queuelist.next == &nd->queue)
		return NULL;
	return list_entry(rq->queuelist.next, struct request, queuelist);
}

static int sstf_init_queue(struct request_queue *q, struct elevator_type *e)
{
	struct sstf_data *nd;
	struct elevator_queue *eq;

	eq = elevator_alloc(q, e);
	if (!eq)
		return -ENOMEM;

	nd = kmalloc_node(sizeof(*nd), GFP_KERNEL, q->node);
	if (!nd) {
		kobject_put(&eq->kobj);
		return -ENOMEM;
	}

	// default position of 0, forward
	nd->sector = 0;
	nd->direction = 'F';
	eq->elevator_data = nd;

	INIT_LIST_HEAD(&nd->queue);

	spin_lock_irq(q->queue_lock);
	q->elevator = eq;
	spin_unlock_irq(q->queue_lock);
	return 0;
}

static void sstf_exit_queue(struct elevator_queue *e)
{
	struct sstf_data *nd = e->elevator_data;

	BUG_ON(!list_empty(&nd->queue));
	kfree(nd);
}

static struct elevator_type elevator_sstf = {
	.ops = {
		.elevator_merge_req_fn		= sstf_merged_requests,
		.elevator_dispatch_fn		= sstf_dispatch,
		.elevator_add_req_fn		= sstf_add_request,
		.elevator_former_req_fn		= sstf_former_request,
		.elevator_latter_req_fn		= sstf_latter_request,
		.elevator_init_fn		= sstf_init_queue,
		.elevator_exit_fn		= sstf_exit_queue,
	},
	.elevator_name = "sstf",
	.elevator_owner = THIS_MODULE,
};

static int __init sstf_init(void)
{
	return elv_register(&elevator_sstf);
}

static void __exit sstf_exit(void)
{
	elv_unregister(&elevator_sstf);
}

module_init(sstf_init);
module_exit(sstf_exit);


MODULE_AUTHOR("Group 11-01");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SSTF IO scheduler");
