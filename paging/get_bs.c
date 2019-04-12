#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {

	STATWORD ps;
	disable(ps);
  /* requests a new mapping of npages with ID map_id */
  //  kprintf("get_bs.c - get_bs - To be implemented!\n");
	if(npages == 0){
		// Number of pages should be greater than zero.
		restore(ps);
		return SYSERR;
	}

	if(npages > 256){
		// Number of pages shouldn't be more than 256.
		restore(ps);
		return SYSERR;
	}
	
	if(bsm_tab[bs_id].bs_status == BSM_MAPPED){
		if(bsm_tab[bs_id].bs_pid == currpid){
			// Already assigned to the same process. Hence, returning the old "npages" value.
			restore(ps);
			return bsm_tab[bs_id].bs_npages;
		}
			
		else {
			// Assigned to some other process.
			if(bsm_tab[bs_id].bs_private_heap == 1){
				restore(ps);
				return SYSERR;
			}
			else{
				restore(ps);
				return bsm_tab[bs_id].bs_npages;
			}
		}
	}
	else if(bsm_tab[bs_id].bs_status == BSM_UNMAPPED){
		bsm_tab[bs_id].bs_status = BSM_MAPPED;
		bsm_tab[bs_id].bs_pid = currpid;
		bsm_tab[bs_id].bs_vpno = 0;
		bsm_tab[bs_id].bs_npages = npages;
		bsm_tab[bs_id].bs_sem = 0;	
		bsm_tab[bs_id].bs_private_heap = 0;
	}
		restore(ps);
    return npages;
}


