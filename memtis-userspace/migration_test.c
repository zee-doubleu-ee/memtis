// ===== task =====
// Node 1에 1GB 메모리를 동적으로 할당받고 Node 0로 move_pages 시스템콜을 이용해서 page migration을 수행하는 유저프로그램을 구현 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <numa.h> // numa API를 사용하기 위함
#include <numaif.h>

#define ONE_GIGABYTE (1024UL * 1024 * 1024)
#define PAGE_SIZE 4096

int main(){
  if(numa_available()==-1){
    // 먼저 NUMA를 지원하는지 확인, 미지원시 error 출력
    fprintf(stderr, "[TLB Task] Error : system does not support NUMA");
    return 1;
  }

  // ******** Node 1에 1GB를 동적할당 ********
  printf("[TLB Task] Allocating 1GB to Node 1.. \n");

  char *new_data = (char *)numa_alloc_onnode(ONE_GIGABYTE*10, 1);
  // void *numa_alloc_onnode(size_t size, int node)
  // -> size(Byte)만큼을 node번 Node에 할당 -> 시작 주소 포인터 void*를 할당

  if(new_data == NULL){ // 만약 할당받은 new_data가 NULL, 즉 무효하면 오류 출력
    perror("allocating 1GB to Node 1 failed : ");
    return 1;
  }

  printf("[TLB Task] Physical allocation starts.. \n");
  memset(new_data, 1, ONE_GIGABYTE*10);
  printf("[TLB Task] Physical allocation ends.. \n");

  // ******** Node 1 -> Node 0으로 Migration ********
  unsigned long number_of_pages = 10* ONE_GIGABYTE / PAGE_SIZE;

  // 페이지 별 migration 성공 여부를 담을 배열
  int *status = malloc(sizeof(int) * number_of_pages);
  // 페이지 시작 주소를 담을 배열
  void **pages_start_point = malloc(sizeof(void *) * number_of_pages);
  // 페이지 별 Migration되어야 할 Target Node를 담을 배열
  int *target_node = malloc(sizeof(int) * number_of_pages);

  // 만약 위 3개의 배열 중에 잘못된 게 하나라도 있으면 바로 모두 free해주고, Node 1에 할당한 1GB 메모리도 free해줌
  if((status==NULL)||(target_node==NULL)||(pages_start_point==NULL)){
    perror("Failed to allocate arrays for move_pages syscall");
    if(status!=NULL) free(status);
    if(target_node!=NULL) free(target_node);
    if(pages_start_point!=NULL) free(pages_start_point);
    numa_free(new_data, ONE_GIGABYTE);
    return 1;
  }

  printf("[TLB Task] Preparing migration to Node 0\n");
  printf("[TLB Task] # of Page : %lu \n",number_of_pages);

  // 위에서 선언한 3개의 배열에 정보 입력
  for(unsigned long i=0; i<number_of_pages; i++){
    pages_start_point[i] = new_data + (i*PAGE_SIZE);
    target_node[i] = 0;
    status[i] = -999; // 일단 아무 음수값으로 저장
  }
  printf("[TLB Task] Preparing migration ends..\n");

  // ******** Migration 실행 ********
  // long move_pages(int pid, unsigned long count, void **pages, const int *nodes, int *status, int flags);
  // pid : 페이지를 이동시킬 프로세스의 ID (0이면 현재 프로세스)
  // count : migration시키고자 하는 페이지의 갯수
  // pages : 이동할 페이지들의 시작 주소를 담고 있는 포인터 배열
  // nodes : 각 페이지가 이동할 target node를 담고 있는 배열
  // status : 각 페이지가 성공적으로 이동했는지 상태를 담고있는 배열
  // flags : migration 과정에서 적용시킬 flag
  // -> MPOL_MF_MOVE : 실제로 이동시키라는 뜻의 flag

  printf("[TLB Task] Calling move_pages() system calls \n");
  long res = move_pages(0, number_of_pages, pages_start_point, target_node, status, MPOL_MF_MOVE);

  if(res<0){
    // 오류 발생시 perror 출력
    perror("move_pages() syscall errors!");
  }
  else{
    printf("[TLB Task] move_pages() successfully ends \n");
    printf("[TLB Task] check kernel log by : $dmesg \n");

    long failed_page_count = 0;
    for(unsigned long i=0;i<number_of_pages;i++){
      if(status[i]<0) failed_page_count++;
    }
    printf("[TLB Task] # of migration failed pages : %ld \n", failed_page_count);
  }
  printf("\n==================[end]================\n");
  free(pages_start_point);
  free(target_node);
  free(status);
  numa_free(new_data, ONE_GIGABYTE);

  return 0;
}