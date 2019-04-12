/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>


extern int page_replace_policy;

/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{

  STATWORD ps;
  disable(ps);

  pd_t *pd_entry;
  pt_t *pt_entry;
  int new_pt, new_frame, bs, pg_offset_bs;
  unsigned long *temp_bs_content, *temp_fr_content;

  unsigned long v_addr;
  v_addr = read_cr2();

  virt_addr_t *virt_addr;
  virt_addr = (virt_addr_t *)&v_addr;
  
  unsigned int pg_offset = virt_addr->pg_offset, 
                pt_offset = virt_addr->pt_offset,
                pd_offset = virt_addr->pd_offset;
  
  unsigned long pdbr;
  pdbr = proctab[currpid].pdbr;

  pd_entry = pdbr + pd_offset * sizeof(pd_t);

  if(pd_entry->pd_pres == 0){
    new_pt = create_pt();
    
    if(new_pt == SYSERR){
      kill(currpid);
      restore(ps);
      return SYSERR;
    }

    pd_entry->pd_pres = 1;
    pd_entry->pd_write = 1;
		pd_entry->pd_user = 0;
		pd_entry->pd_pwt = 0;
		pd_entry->pd_pcd = 0;
		pd_entry->pd_acc = 0;
		pd_entry->pd_mbz = 0;
		pd_entry->pd_fmb = 0;
		pd_entry->pd_global = 0;
		pd_entry->pd_avail = 0;
		pd_entry->pd_base = new_pt+FRAME0;


		frm_tab[new_pt].fr_status = FRM_MAPPED;
		frm_tab[new_pt].fr_type = FR_TBL;
    frm_tab[new_pt].fr_pid = currpid;
    
  }

  pt_entry = (pt_t *)(pd_entry->pd_base * NBPG + pg_offset * sizeof(pt_t));

  if(pt_entry->pt_pres != 1)
	{
//		bsm_lookup(currpid,v_addr,&bs,&pg_offset_bs);

  //  kprintf("pfint - before calling get_frm\n");
    get_frm(&new_frame);
  //  kprintf("pfint - after calling get_frm : %d\n", new_frame);

		pt_entry->pt_pres = 1;
		pt_entry->pt_write = 1;
		pt_entry->pt_base = (FRAME0+new_frame);

		frm_tab[pd_entry->pd_base-FRAME0].fr_refcnt++;

		frm_tab[new_frame].fr_status = FRM_MAPPED;
		frm_tab[new_frame].fr_type = FR_PAGE;
		frm_tab[new_frame].fr_pid = currpid;
		frm_tab[new_frame].fr_vpno = v_addr/NBPG;

		bsm_lookup(currpid,v_addr,&bs,&pg_offset_bs);
		read_bs((char*)((FRAME0+new_frame)*NBPG),bs,pg_offset_bs);

		if(page_replace_policy == AGING)
			insert_frm_fifo(new_frame);
		else if(page_replace_policy == SC)
			update_frms_LRU();

    kprintf("After updating\n");
  }
  
  write_cr3(pdbr);
  kprintf("After writing\n");
  restore(ps);
  return OK;
}


int create_pt(){
  int frame_number;
  unsigned int frame_addr;
  pt_t *page_table;
  
  int ret_val = get_frm(&frame_number);
  if(ret_val == SYSERR){
    return SYSERR;
  }

  frame_addr = (FRAME0 + frame_number) * NBPG;

  page_table = (pt_t *)frame_addr;

  int i = 0;
  for(;i < 1024;i++){
    page_table[i].pt_acc = 0;
		page_table[i].pt_avail = 0;
		page_table[i].pt_base = 0;
		page_table[i].pt_dirty = 0;
		page_table[i].pt_global = 0;
		page_table[i].pt_mbz = 0;
		page_table[i].pt_pcd = 0;
		page_table[i].pt_pres = 0;
		page_table[i].pt_pwt = 0;
		page_table[i].pt_user = 0;
		page_table[i].pt_write = 0;
  }

  return frame_number;
}