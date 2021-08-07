/**********************************************************************
 * Copyright (c) 2020-2021
 *  Sang-Hoon Kim <sanghoonkim@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "types.h"
#include "list_head.h"
#include "vm.h"

// 할당된 페이지는 프로세스의 페이지 테이블을 조작하여 현재 프로세스에 매핑되어야 합니다. 시스템은 vm.h에 정의된 대로 2단계 계층 페이지 테이블을 유지함

/**
 * Ready queue of the system
 */
extern struct list_head processes;

/**
 * Currently running process
 */
extern struct process *current;

/**
 * Page Table Base Register that MMU will walk through for address translation
 */
extern struct pagetable *ptbr;


/**
 * The number of mappings for each page frame. Can be used to determine how
 * many processes are using the page frames.
 */
extern unsigned int mapcounts[];


/**
 * alloc_page(@vpn, @rw)
 *
 * DESCRIPTION
 *   Allocate a page frame that is not allocated to any process, and map it
 *   to @vpn. When the system has multiple free pages, this function should
 *   allocate the page frame with the **smallest pfn**.
 *   You may construct the page table of the @current process. When the page
 *   is allocated with RW_WRITE flag, the page may be later accessed for writes.
 *   However, the pages populated with RW_READ only should not be accessed with
 *   RW_WRITE accesses.
 *
 * RETURN
 *   Return allocated page frame number.
 *   Return -1 if all page frames are allocated.
 */

unsigned int alloc_page(unsigned int vpn, unsigned int rw)
{
	int outer_pte_num = vpn / NR_PTES_PER_PAGE;
	int pte_num = vpn % NR_PTES_PER_PAGE;
	//printf("outer_num: %d\n pte_num: %d\n",outer_pte_num, pte_num);

	// current process의 page table이 없다면, 생성( process를 list_head에 넣기?)
	if(current->pagetable.outer_ptes[outer_pte_num] == NULL){
		current->pagetable.outer_ptes[outer_pte_num]= malloc(sizeof(struct pte_directory));
	}
	
	struct pte_directory *pd = current->pagetable.outer_ptes[outer_pte_num];
	struct pte *pte = &pd->ptes[pte_num];

	// 사용가능한 가장 작은 값의 pfn에 alloc 
	//int pageFrameNum[NR_PAGEFRAMES] = {0};
	//를 이용했지만 프로세스마다 배열을 만들어야하므로
	// mapcounts를 이용하는 코드로 바꿈
	int pfn = NR_PAGEFRAMES + 2;

	for(int i = 0; i < NR_PAGEFRAMES; i++){
		if(mapcounts[i] == 0 && i < pfn){
			pfn = i;
		}
	}

	if(pfn != NR_PAGEFRAMES + 2){
		pte->pfn = pfn;
		mapcounts[pfn] += 1;
		//printf("mapcount%d: %d\n",pfn, mapcounts[pfn]);
		pte->valid = 1;

		if(rw == 3){
			pte->writable = true;
		}else if(rw = RW_READ){
			pte->writable = false;
		}
		return pte->pfn;
	}

	return -1;
}


/**
 * free_page(@vpn)
 *
 * DESCRIPTION
 *   Deallocate the page from the current processor. Make sure that the fields
 *   for the corresponding PTE (valid, writable, pfn) is set @false or 0.
 *   Also, consider carefully for the case when a page is shared by two processes,
 *   and one process is to free the page.
 */
void free_page(unsigned int vpn)
{

	int outer_pte_num = vpn / NR_PTES_PER_PAGE;
	int pte_num = vpn % NR_PTES_PER_PAGE;

	struct pte_directory *pd = current->pagetable.outer_ptes[outer_pte_num];
	struct pte *pte = &pd->ptes[pte_num];

	mapcounts[pte->pfn] -= 1;
	pte->pfn = 0;
	pte->valid = 0;
	pte->writable = false;

	// page에 valid 한 거 없으면 free
	int validcount = 0;

	for(int i = 0; i < NR_PTES_PER_PAGE; i++){

		if(current->pagetable.outer_ptes[i] != NULL){

			validcount = 0;
			for(int j = 0; j < NR_PTES_PER_PAGE; j++){
				if(current->pagetable.outer_ptes[i]->ptes[j].valid == 1){
					validcount++;
				}			
			}
			if(validcount == 0){
				current->pagetable.outer_ptes[i] = NULL;
				free(current->pagetable.outer_ptes[i]);
			}
		}
	}

}


/**
 * handle_page_fault()
 *
 * DESCRIPTION
 *   Handle the page fault for accessing @vpn for @rw. This function is called
 *   by the framework when the __translate() for @vpn fails. This implies;
 *   0. page directory is invalid
 *   1. pte is invalid
 *   2. pte is not writable but @rw is for write
 *   This function should identify the situation, and do the copy-on-write if
 *   necessary.
 *
 * RETURN
 *   @true on successful fault handling
 *   @false otherwise
 */
bool handle_page_fault(unsigned int vpn, unsigned int rw)
{
	int outer_pte_num = vpn / NR_PTES_PER_PAGE;
	int pte_num = vpn % NR_PTES_PER_PAGE;

	struct pte_directory *pd = current->pagetable.outer_ptes[outer_pte_num];
	struct pte *pte = &pd->ptes[pte_num];

	// fork()로 인해 잠시 write 막아놓은 상태인 경우
	// 다른 free pfn 할당 
	if(pte->private == 1 && mapcounts[pte->pfn] > 1){

		int newpfn = NR_PAGEFRAMES + 2;

		for(int i = 0; i < NR_PAGEFRAMES; i++){
			if(mapcounts[i] == 0 && i < newpfn){
				newpfn = i;
			}
		}

		if(newpfn != NR_PAGEFRAMES + 2){
			mapcounts[pte->pfn] -= 1;
			pte->pfn = newpfn;
			mapcounts[newpfn] += 1;
			pte->valid = 1;
			pte->private = 0;

			pte->writable = true;
			
			//printf("true");
			return true;

		}else{
			//printf("fault");
			return false;
		}
		
	} else if(pte-> private == 1 && mapcounts[pte->pfn] == 1) { 
		pte->private = 0;
		pte->writable = 1;
		pte->valid = 1;
		return true;
	}

	return false;
}


/**
 * switch_process()
 *
 * DESCRIPTION
 *   If there is a process with @pid in @processes, switch to the process.
 *   The @current process at the moment should be put into the @processes
 *   list, and @current should be replaced to the requested process.
 *   Make sure that the next process is unlinked from the @processes, and
 *   @ptbr is set properly.
 *
 *   If there is no process with @pid in the @processes list, fork a process
 *   from the @current. This implies the forked child process should have
 *   the identical page table entry 'values' to its parent's (i.e., @current)
 *   page table. 
 *   To implement the copy-on-write feature, you should manipulate the writable
 *   bit in PTE and mapcounts for shared pages. You may use pte->private for 
 *   storing some useful information :-)
 */

/*현재 @current 프로세스는 @process 목록에 넣어야 하며, @current는 요청된 프로세스로 대체
다음 프로세스가 @process에서 연결 해제되고 @ptbr이 올바르게 설정되었는지 확인*/

void switch_process(unsigned int pid)
{
	if(pid == current->pid){	// 현재 process pid인 경우 그대로

	}else{

		int exist = -1;
		struct process* temp;

		list_for_each_entry(temp, &processes, list) {
			if(temp->pid == pid){
				current = temp;
				ptbr = &current->pagetable;
				exist = 1;
				//printf("%d\n", temp->pid);
			}
			//printf("111: %d\n", temp->pid);
    	}

		if(exist == -1){	// processes에 switch 요청온 process가 없을 경우 fork

			// current는 processes에 넣음
			list_add_tail(&current->list, &processes);
			/*struct process* temp;
			list_for_each_entry(temp,&processes, list) {
				printf("222: %d\n", temp->pid);
    		}*/

			struct process* rq_process = malloc(sizeof(struct process));
			// 구조체 데이터 copy
			//memcpy(&rq_process, &current, sizeof(struct process));
			rq_process->pid = pid;

			struct pte *currpte;
			struct pte_directory *pd;
			struct pte *pte;
			int i = 0, immpfn = -1;

			while(i < NR_PTES_PER_PAGE ){

				if(current->pagetable.outer_ptes[i] != NULL){
					rq_process->pagetable.outer_ptes[i] = malloc(sizeof(struct pte_directory));
					pd = rq_process->pagetable.outer_ptes[i];

					for( int j = 0; j < NR_PTES_PER_PAGE ; j++){
						currpte = &current->pagetable.outer_ptes[i]->ptes[j];
						pte = &pd->ptes[j];

						if(currpte->valid == 1 ){
							if(currpte->writable == 1 || currpte->private == 1) {
								currpte->writable = 0;
								currpte->private = 1;
								pte->private = 1;
							}

							pte->valid = 1;
							pte->pfn = currpte->pfn;
							mapcounts[pte->pfn] += 1;
						}
						// 원래 write도 가능한데, fork()하면서 os가 막아 놓음을 표시
					}
				}
				i++;
			}
			
			current = rq_process;
			ptbr = &rq_process->pagetable;
		}
	}
}
