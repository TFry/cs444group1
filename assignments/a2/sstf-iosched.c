/*
 * elevator sstf
 */
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

//Set disk
int disk_head = -1;

struct sstf_data {
	struct list_head queue;
};

static void sstf_merged_requests(struct request_queue *q, struct request *rq,
				 struct request *next)
{
	list_del_init(&next->queuelist);
}

static int sstf_dispatch(struct request_queue *q, int force)
{
	struct sstf_data *helper = q->elevator->elevator_data;
	if(!list_empty(&helper->queue)){
		struct request *r;
		char up_down;
		
		r = list_entry(helper->queue.next, struct request, queuelist);
		list_del_init(&r->queuelist);
		elevator_dispatch_sort(q, r);
		
		if(r_data_dir(r) == READ){
			up_down = 'R';
		}
		else{
			up_down = 'W';
		}
		printk("[CLOOK] direction %c %llu\n",up_down, blk_rq_pos(r));
		
		return 1;
	}
	return 0;
	
}

static void sstf_add_request(struct request_queue *q, struct request *rq)
{
	struct noop_data *helper = q->elevator->elevator_data;
	struct list_head *cur = NULL;
	struct list_head *cur2 = NULL;
	struct list_head *cur1 = NULL;

	bool is_larger_than_head;
	
	//Checking if request is smaller or larger than head
	list_for_each(cur, &helper->queue){
		struct request *c = list_entry(cur, struct request, queuelist);
		if(rq_end(rq) > rq_end(c)){
			is_larger_than_head=1;
			break;
		}
		else{
			is_larger_than_head=0;
			break;
		}
	}
	
	//Must sort for request being larger first
	//Iterate until next request is larger
	//Thus we are sorted
	if(is_larger_than_head)
		{	
			list_for_each(cur2, &helper->queue)	
			{	
				struct request *h = list_entry(cur2, struct request, queuelist);	
				if(rq_end(h) > rq_end(rq))
					break;
			}
		}
 		
		//Sort by smaller request than head
		//Iterates from largest to smallest
		//Followed by iterating until next request is larger
		else
		{	bool prev_checker= 0;
			bool circled_already=0;
			list_for_each(cur2, &helper->queue)
			{
				struct request *g = list_entry(cur2, struct request, queuelist);
				if(rq_end_sector(g) > rq_end_sector(rq) && circled_already)
					break;
				if(prev_checker > rq_end_sector(g))
					circled_already=1;	
					
				prev_checker= rq_end_sector(g);	
			}	
		}
		
	
	list_add_tail(&rq->queuelist, cur2);

	//Set the read or write
	//head up or head down
	char up_down;
	if(rq_data_dir(rq) == READ)
		up_down = 'R';
	else
		up_down = 'W';

	
	//Print added request
	printk("[CLOOK] add %c %llu\n", up_down, blk_rq_pos(rq));

	
	//Printing out the entire queue just for debugging
	printk("current queue: ");	
       	list_for_each(cur1, &helper->queue)
	{
		struct request *f = list_entry(cur1, struct request, queuelist);	
	
		printk(" %llu", blk_rq_pos(f));
	}
	//new line for cleaness
	printk("\n");
}

static struct request *
sstf_former_request(struct request_queue *q, struct request *rq)
{

}

static struct request *
sstf_latter_request(struct request_queue *q, struct request *rq)
{

}

static int sstf_init_queue(struct request_queue *q, struct elevator_type *e)
{

}

static void sstf_exit_queue(struct elevator_queue *e)
{

}

static struct elevator_type elevator_sstf = {
	.ops.sq = {
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


MODULE_AUTHOR("Team 11-01");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SSTF IO scheduler");
