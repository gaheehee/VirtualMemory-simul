## Project #3: Virtual Memory Simulator

### Goal
Implement a mini virtual memory system simulator.

+ **page allocation**  
  
요청으로 들어온 vpn을 하나의 페이지 안에 들어있는 PTE 수로 나눈 몫을 이용하여 몇 번째
outer에 들어갈 건지 결정하고, 나머지를 이용하여 그 outer의 page table에서 몇 번째 entry로 들
어갈 건지 결정하였다. 그렇게 결정된 page table에 메모리를 할당해준 후, 사용하지 않는 가장
작은 pfn을 mapcounts가 0인 것들 중 가장 작은 인덱스 값을 통해 찾아 해당 page table entry의
pfn 값으로 세팅해주고, 해당 mapcounts[pfn]을 +1 해주었다(해당 pfn을 가리키는 애가 하나 생
기므로). 그리고 valid를 1로, rw 요청으로 왔다면 writable도 1로 해주었다. 이 과정에서 두가지의
시행 착오를 겪었다.
첫번째는 rw일 경우 해당 pte의 writable을 1로 해줄 때, rw가 RW_WRITE일 경우 pte의 writable
을 1로 바꿔주도록 짰는데, 알고보니 요청받은 rw는 3으로 들어오고 RW_WRITE는 2였기 때문에
제대로 작동하지 않았다. 따라서 rw가 RW_WRITE일 경우를 rw가 3일 경우로 바꾸어 해결하였다.
두번째 시행착오는, current의 outer pagetable을 매번 할당해주도록 코드를 작성하여 이미 존재하
는 outer pagetable를 덮어서 또 새로 생성되는 문제를 겪었다. 이를 해당 outer pagetable이
NULL인지 체크하는 조건문을 통하여 이미 존재하면 메모리 할당을 해주지 않도록 하여 해결하였
다.

+ **how I implement deallocation**   

우선 vpn을 하나의 page에 들어있는 pte 개수로 나누어 몫은 outer의 몇 번째인지, 나머지는
page table의 몇 번째 entry인지 알아내었다. 그렇게 해당 pte로 가서 pfn를 찾아내어 그 pfn을
인덱스로 하는 mapcounts를 -1 해주고(해당 pfn을 가리키는 애가 하나 없어지므로), 해당 pte의
pfn, valid, writable을 모두 0으로 초기화해주었다.
그리고 나서 validcount라는 변수를 이용하여 현재 돌고있는 process의 모든 존재하는 각 page 
table의 pte들을 반복문을 통해 돌면서 하나의 page table 안에 valid한 pte가 존재하는지
vallidcount로 세리고 만약 존재하는 valid pte가 없다면 해당 page table를 free시켜주어 없애준다.
이때 해당 page table을 바로 free 시켜주었더니 해당 page table은 사라지지도 않았을뿐더러 이
상한 값이 pte의 pfn에 들어가 있었다. 이는 free하기 직전에 해당 page table을 NULL 시켜준 후
free 함으로써 해결하였다.

+ **how I implement fork**  

current의 pid와 요청받은 pid를 비교하여, switch하려는 process가 현재 돌고있는 process라면 현
재 돌고있는 process를 그대로 두고, 그게 아니라면 요청한 process가 processes (list_head)에 존
재하는지 list_for_each_entry를 이용하여 체크한다. 만약 processes에 존재한다면 current를 요청한
pid를 가지는 process로 바꿔주고 ptbr을 그 process의 pagetable을 가리키도록 해준다. 그러나
요청한 process가 processes에 존재하지 않는다면 current process의 page table을 fork한다.
Fork는 rq_process를 하나 생성하여 outer page에 메모리를 할당해준 후, rq_process의 pid를 요청
받은 pid로 세팅해주고 current에 존재하는 outer page entry와 그 안의 pte들을 하나하나 돌면서
valid인 entry에 대한 정보를 rq_process의 똑같은 위치의 pte 정보로 그대로 가져오고 해당 pfn
의 mapcounts를 +1 해준다. 만약 current의 pte가 writable이 1이라면, 둘이 같이 pfn을 공유하기
때문에 마음대로 write해서는 안되므로 current pte의 writable과 똑같은 위치의 rq_process의 pte 
writable을 모두 false로 해준다. 그리고 원래 write도 가능한데 fork를 하면서 os가 write를 막아놨
음을 표시하는 용도로 pte의 private변수를 1로 세팅해준다.
처음에는 current의 pte가 writable일 경우에만 current와 rq_process의 private를 1로 설정해주었
다. 이 때문에, 한번 fork되어서 writable이 false인 애들을 또 fork하여 생성된 pte도 원래는 write
가 가능하다는 것을 private로 표시를 해야 하는데, 이런 애들은 private가 1로 세팅되지 않아 나
중에 write를 할 때 copy-on-write가 되지 않는 문제가 발생하였다. 따라서 current에 존재하는
outer page entry와 그 안의 pte들을, 하나하나 돌면서 pte가 writable이 1인지만 체크하는 것이
아니라, private가 1인지도 체크하여 둘 중에 하나라도 만족하면 같은 위치의 rq_process의 pte의
private를 1로 바꿔주어 나중에 write할 때 copy-on-write가 될 수 있도록 해결해주었다.  

+ **how I implement copy-on-write**  

vpn을 하나의 page 안에 들어있는 entry수(NR_PTES_PER_PAGE)로 나누어 어떤 outer entry에 어
떤 pte 인지 알아내서 접근한다. write하려는 해당 pte의 private가 1이고 mapcount가 1 이상이면
fork로 인해 잠시 write를 막아놓은 상태를 뜻하므로 다른 free pfn을 찾아 할당하여 copy 하고 이
전의 pfn의 mapcounts는 하나 줄인다. 할당 받은 pfn은 같이 공유하는 다른 애가 없으므로
writable을 1로 바꾸고 private도 0으로 바꾸어 준다. 그리고 true를 반환하여 다시 write를 재개할
수 있도록 한다.
만약 write하려는 해당 pte의 private가 1이고 mapcount가 1이라면, 해당 pte의 pfn을 공유하는
다른 애들이 없으므로 다른 pfn을 찾지 않아도 된다. 따라서 원래 가리키고 있는 pfn에 write해도
된다는 것이다. 해당 pte의 private를 0으로 해주고, writable을 1로 해준 후 true를 반환하여 write
를 재개하도록 해주었다.
처음에 해당 pfn의 mapcounts가 1인 경우 그 pfn을 혼자만 사용하고있음을 고려하지 않고, 
private가 1인 것만 체크하여 private가 1인 애들은 모두 다른 free pfn을 할당하여 copy-on-write
해주어 시행착오를 겪었었다. 이에 private외에도 mapcounts가 1인지 1 이상인지 체크해주어 1이
이상일 때 다른 pfn을 찾아 copy-on-write를 해주도록 수정하였다.
