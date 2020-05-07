#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "cpu/reg.h"
#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP*  new_wp(){
    if(free_!=NULL)
    {
        WP *p=free_;
	free_=free_->next;
	return p;
    }
    else
    {	printf("1\n");
	assert(0);}
}

void free_wp(WP *wp){
    WP *p=free_;
    while(p->next!=NULL)
        p=p->next;
    p->next=wp;
    wp->next=NULL;
}

int set_watchpoint(char *e){
    WP *p;
    if(head==NULL)
    {	init_wp_pool();
	p=new_wp();
	head=p;
    }
    else
        p=new_wp();
    printf("Set watchpoint #%d\n",p->NO);
    strcpy(p->expr,e);
    printf("expr=%s\n",p->expr);
    bool success=true;
    p->old_val=expr(p->expr,&success);
    if(success)
	printf("Old value = 0x%x\n",p->old_val);
    else
    {	printf("Fail!\n");
        return 0;
    }
    return p->NO;
}

bool delete_watchpoint(int NO){
    if(head==NULL)
    {
	printf("No Watchpoint!\n");
	return false;
    }
    WP *p=head;
    if(head->NO==NO)
    {
        head=head->next;
	free_wp(p);
	printf("Watchpoint#%d Deleted!\n",NO);
	return true;
    }
    while(!p&&p->NO!=NO)
    {
        p=p->next;
    }
    if(p->NO==NO)
    {	
	free_wp(p);
        p=p->next;
    }
    else
    {
	printf("NO Not Found!\n");
	return false;
    }
    printf("Watchpoint#%d Deleted!\n",NO);
    return true;
}

void list_watchpoint(){
    WP *p=head;
    if(head==NULL)
	printf("No Watchpoint!\n");
    else
    {
	printf("NO\tExpr\t	Old Value\n");
	while(p&&p!=free_)
	{
	    printf("%2d\t%-25s\t0x%x\n",p->NO,p->expr,p->old_val);
	    p=p->next;
	}
    }
}

WP* scan_watchpoint(){
    WP *p=head;
    if(!p&&p!=free_)
    {
        printf("No Watchpoint!\n");
	return NULL;
    }
    else
    {
        bool success=true;
	while(p)
	{
	    p->new_val=expr(p->expr,&success);
	    if(success)
	    {
	        if(p->new_val!=p->old_val)
		{
		    printf("Hit Watchpoint %d at address  0x%8x\n",p->NO,cpu.eip);
		    printf("expr\t= %s\n",p->expr);
		    printf("old value = 0x%x\n",p->old_val);
		    printf("New Value = 0x%x\n",p->new_val);
		    p->old_val=p->new_val;
		    printf("Program Paused\n");
		    return p;
		}
	    }
	}
    }
    return NULL;
}
