/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
	STATWORD ps;
	disable(ps);
	bsd_t i = 0;
	for(;i < NSTORES;i++){
        clear_bsm(i);
    }
    kprintf("bsm.c - init_bsm\n");
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
	STATWORD ps;
	disable(ps);
	if(avail == NULL){	
		restore(ps);
		return SYSERR;
	}
		
	int i = 0;
	for(;i < NSTORES;i++){
		if(bsm_tab[i].bs_status == BSM_UNMAPPED){
			*avail = i;
			break;
		}
	}
//	kprintf("bsm.c - get_bsm\n");
	
	restore(ps);
	return OK;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{	
	STATWORD ps;
	disable(ps);

	bsm_tab[i].bs_status = BSM_UNMAPPED;
	bsm_tab[i].bs_pid = BADPID;
	bsm_tab[i].bs_vpno = 0;
	bsm_tab[i].bs_npages = 0;
	bsm_tab[i].bs_sem = 0;	

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
    kprintf("bsm.c - bsm_lookup\n");
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
    kprintf("bsm.c - bsm_map\n");
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
    kprintf("bsm.c - bsm_unmap\n");
}


void print_bsm_tab(){
    int i = 0;
    for(;i < NSTORES;i++){
        kprintf("%d:%d:%d:%d:%d:%d\n", i, bsm_tab[i].bs_status, bsm_tab[i].bs_pid, bsm_tab[i].bs_vpno, bsm_tab[i].bs_npages, bsm_tab[i].bs_sem);
    }
}

void clear_bsm(bsd_t bs_id){
    bsm_tab[bs_id].bs_status = BSM_UNMAPPED;
    bsm_tab[bs_id].bs_pid = BADPID;
    bsm_tab[bs_id].bs_vpno = 0;
    bsm_tab[bs_id].bs_npages = 0;
    bsm_tab[bs_id].bs_sem = 0;
}