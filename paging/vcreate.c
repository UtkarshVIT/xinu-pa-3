/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
//	kprintf("To be implemented!\n");
	if(hsize < 0 || hsize > 256){
		return SYSERR;
	}

	STATWORD ps;
	disable(ps);

	int new_bs_id;

	int ret_val = get_bsm(&new_bs_id);
	
	if(ret_val == SYSERR){
		restore(ps);
		return SYSERR;
	}

	int pid = create(procaddr, ssize, priority, name, nargs, args);

	if(pid == SYSERR){
		restore(ps);
		return SYSERR;
	}

	struct mblock *bs_base;
	
	bsm_map(pid, 4096, new_bs_id, hsize);

	struct pentry *cur_proc = &proctab[pid];
	cur_proc->store = new_bs_id;
	cur_proc->vhpno = 4096;
	cur_proc->vhpnpages = hsize;
	cur_proc->vmemlist->mnext = 4096 * NBPG; // first address after 16 MB virtual adress.
	cur_proc->vmemlist->mlen = hsize * 4096; // no of pages in heap * size of each page.

	bs_base = BACKING_STORE_BASE + (new_bs_id * BACKING_STORE_UNIT_SIZE);
	bs_base->mlen = hsize * NBPG;
	bs_base->mnext = NULL;


	restore(ps);
	return pid;
}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
