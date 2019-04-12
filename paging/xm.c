/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
  STATWORD ps;
  disable(ps);

  if((virtpage < NBPG)){
    restore(ps);
    return SYSERR;
  } 
  if((npages < 1) || (npages > 256)){
    restore(ps);
    return SYSERR;
  } 
  if((source < 0) || (source > 7)){
    restore(ps);
    return SYSERR;
  }
  bsm_map(currpid, virtpage, source, npages);

  restore(ps);
  return OK;
}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{
  STATWORD ps;
  disable(ps);

  if(virtpage < NBPG){
    restore(ps);
    return SYSERR;
  }

  bsm_unmap(currpid, virtpage, 0);
  write_cr3(proctab[currpid].pdbr * NBPG);
  restore(ps);
  return OK;
}
