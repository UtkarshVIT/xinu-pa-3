/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

extern int page_replace_policy;

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
//  kprintf("To be implemented!\n");
 // kprintf("init_frm\n");
  STATWORD ps;
  disable(ps);
  
  int i = 0;
  for(;i < NFRAMES; i++){
    clear_frm(i);
  }
  
  restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
 // kprintf("get_frm\n");
  STATWORD ps;
  disable(ps);

  int i = 0;
  for(;i < NFRAMES;i++){
    if(frm_tab[i].fr_status == FRM_UNMAPPED){
      *avail = i;
//	kprintf("here :%d\n", i);
      restore(ps);
      return OK;
    }
  }

  int frame_number;
  if(page_replace_policy == SC){
//	kprintf("here\n");
    frame_number = remove_frm_fifo();
    free_frm(frame_number);
    *avail = frame_number;
    restore(ps);
    return OK;
  }else if(page_replace_policy == AGING){
    frame_number = get_frm_LRU();
    free_frm(frame_number);
    *avail = frame_number;
    restore(ps);
    return OK;
  }
  restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{
  STATWORD ps;
  disable(ps);

  unsigned long v_addr, pdbr;
  unsigned int pt_offset, pd_offset;
  
  pt_t *pt_entry;
  pd_t *pd_entry;
  kprintf("test %d\n", i);  
  int bs_store, page_num, frame_pid;

  if(frm_tab[i].fr_type == FR_PAGE){
    v_addr = frm_tab[i].fr_vpno;
    frame_pid = frm_tab[i].fr_pid;
    pdbr = proctab[frame_pid].pdbr;
    pd_offset = v_addr/1024;
    pt_offset = v_addr & 1023;
    pd_entry = pdbr + (pd_offset * sizeof(pd_t));
    pt_entry = (pd_entry->pd_base * NBPG) + (pt_offset * sizeof(pt_t));
    bs_store = proctab[frm_tab[i].fr_pid].store;
    page_num = frm_tab[i].fr_vpno - proctab[frame_pid].vhpno;

    write_bs((i+FRAME0)*NBPG, bs_store, page_num);
    pt_entry->pt_pres = 0;
    if(frm_tab[pd_entry->pd_base - FRAME0].fr_refcnt-- == 0){
      if(page_replace_policy == AGING){
        frm_tab[pd_entry->pd_base-FRAME0].fr_loadtime = 0;
      }
      frm_tab[pd_entry->pd_base - FRAME0].fr_pid = BADPID;
      frm_tab[pd_entry->pd_base - FRAME0].fr_status = FRM_UNMAPPED;
      frm_tab[pd_entry->pd_base - FRAME0].fr_type = FR_PAGE;
      frm_tab[pd_entry->pd_base - FRAME0].fr_vpno = 0;
      pd_entry->pd_pres = 0;
    }
  }

  restore(ps);
  return OK;
}

void evict_frame(int pid){
  int i = 0;
  for(;i < NFRAMES;i++){
    if(frm_tab[i].fr_pid == pid){
      if(frm_tab[i].fr_type == FR_PAGE){
        update_frm_fifo(i);
      }
      clear_frm(i);
      frm_tab[i].next_frame = -1;

    }
  }
}


int get_frm_LRU(){
  unsigned long load_time = 99999999;
  int i;
  int frame_number = -1;
  fr_map_t *frm;

  for(i=0;i<NFRAMES;i++){
    frm = &frm_tab[i];
    if(frm_tab[i].fr_type == FR_PAGE){
      if(frm->fr_loadtime < load_time){
        load_time = frm->fr_loadtime;
        frame_number = i;
      }
      else if(load_time == frm->fr_loadtime){
        if(frm_tab[frame_number].fr_vpno < frm_tab[i].fr_vpno){
          frame_number = i;
        }
      }
    }
  }
  return frame_number;
}

void update_frms_LRU()
{
	STATWORD ps;
	int i,j,pid;
	struct pentry *pptr;
	unsigned long pdbr;
	pd_t *pd_p;
	pt_t *pt_p;
	int frm_num;
	
	disable(ps);
	timeCount++;

	for(pid = 1; pid<NPROC; pid++)
	{
		pptr = &proctab[pid];
		if(pptr->pstate != PRFREE)
		{
			pdbr = pptr->pdbr;
			pd_p = pdbr;
			for (i=0;i<4;i++)
				{
				 continue;
				 pd_p++;
				}
			for(i=4;i<1024;i++)
			{
				if(pd_p->pd_pres == 1)
				{
					pt_p = (pd_p->pd_base)*NBPG;
					kprintf("Page table is at 0x%08x address\n",pt_p);
					for(j=0;j<1024;j++)
					{
            //kprintf("test %d %d\n", i, j);
						if(pt_p->pt_pres == 1 && pt_p->pt_acc==1)
						{
							frm_num = pt_p->pt_base-FRAME0;
							frm_tab[frm_num].fr_loadtime = timeCount;
							pt_p->pt_acc=0;
						}
						pt_p++;						
					}
				}

				pd_p++;
			}
		}
	}
  //kprintf("updated LRU\n");
	restore(ps);
	return OK;
}

void init_frame_fifo(void)
{
	int i = 0;
	for(i = 0; i < NFRAMES; i++)
		{
		frame_fifo[i].frm_id = i;
		frame_fifo[i].next_frame = -1;
		}
}

void insert_frm_fifo(int frame_num)
{
	STATWORD ps;
	disable(ps);
	int nxt_frm = -1;
	int curr_frm = -1;
	
	if(fifo_head == -1)
		{
		fifo_head = frame_num;
//		kprintf("Updated fifo_head for the first time\n");
		restore(ps);
		return OK;
		}
	else
		{
		nxt_frm = frame_fifo[fifo_head].next_frame;
		curr_frm = fifo_head;
		}
	
	while(nxt_frm != -1)
		{
//		kprintf("In while loop for to update fifo\n");
		curr_frm = nxt_frm;
		nxt_frm = frame_fifo[nxt_frm].next_frame;
		}
	
	frame_fifo[curr_frm].next_frame = frame_num;
	frame_fifo[frame_num].next_frame = -1;
	
//	kprintf(" After updating fifo\n");
	restore(ps);
	return OK;
	
}

int remove_frm_fifo()
{
	STATWORD ps;
	disable(ps);

	int frame_number;
	frame_number = fifo_head;
	
	fifo_head = frame_fifo[fifo_head].next_frame;

	frame_fifo[frame_number].next_frame = -1;

	restore(ps);
	return(frame_number);
}

void update_frm_fifo(int i)
{
	STATWORD ps;
	disable(ps);
	int nxt_frm = -1;
	int curr_frm = -1;
	if(fifo_head == i)
		{
		fifo_head = frame_fifo[fifo_head].next_frame;
		frame_fifo[i].next_frame = -1;
		restore(ps);
		return;
		}
	else
		{
		nxt_frm = frame_fifo[fifo_head].next_frame;
		curr_frm = fifo_head;
		}
	while(nxt_frm != i)
		{
		curr_frm = nxt_frm;
		nxt_frm = frm_tab[nxt_frm].next_frame;
		}
	frame_fifo[curr_frm].next_frame = frm_tab[nxt_frm].next_frame;
	frame_fifo[nxt_frm].next_frame = -1;
	
	restore(ps);
}

void clear_frm(int i){
  frm_tab[i].fr_status = FRM_UNMAPPED;			
  frm_tab[i].fr_pid = BADPID;			
  frm_tab[i].fr_vpno = 0;			
  frm_tab[i].fr_refcnt = 0;		
  frm_tab[i].fr_type = FR_PAGE;		
  frm_tab[i].fr_dirty = 0;
  frm_tab[i].cookie = 0;
  frm_tab[i].fr_loadtime = 0;
}

void print_frm_tab(){
  int i = 0;
  for(;i < NFRAMES;i++){
      kprintf("%d:%d:%d:%d:%d:%d:%d:%d:%d\n", i, frm_tab[i].fr_status, frm_tab[i].fr_pid, frm_tab[i].fr_vpno, frm_tab[i].fr_refcnt, frm_tab[i].fr_type, frm_tab[i].fr_dirty, frm_tab[i].cookie, frm_tab[i].fr_loadtime);
  }
}
