## Project #3: Virtual Memory Simulator

### Goal
Implement a mini virtual memory system simulator.

- page allocation ��û���� ���� vpn�� �ϳ��� ������ �ȿ� ����ִ� PTE ���� ���� ���� �̿��Ͽ� �� ��° outer�� �� ���� �����ϰ�, �������� �̿��Ͽ� �� outer�� page table���� �� ��° entry�� �� � ���� �����Ͽ���. �׷��� ������ page table�� �޸𸮸� �Ҵ����� ��, ������� �ʴ� ���� ���� pfn�� mapcounts�� 0�� �͵� �� ���� ���� �ε��� ���� ���� ã�� �ش� page table entry�� pfn ������ �������ְ�, �ش� mapcounts[pfn]�� +1 ���־���(�ش� pfn�� ����Ű�� �ְ� �ϳ� �� ��Ƿ�). �׸��� valid�� 1��, rw ��û���� �Դٸ� writable�� 1�� ���־���. �� �������� �ΰ����� ���� ������ �޾���. ù��°�� rw�� ��� �ش� pte�� writable�� 1�� ���� ��, rw�� RW_WRITE�� ��� pte�� writable �� 1�� �ٲ��ֵ��� ®�µ�, �˰����� ��û���� rw�� 3���� ������ RW_WRITE�� 2���� ������ ����� �۵����� �ʾҴ�. ���� rw�� RW_WRITE�� ��츦 rw�� 3�� ���� �ٲپ� �ذ��Ͽ���. �ι�° ����������, current�� outer pagetable�� �Ź� �Ҵ����ֵ��� �ڵ带 �ۼ��Ͽ� �̹� ������ �� outer pagetable�� ��� �� ���� �����Ǵ� ������ �޾���. �̸� �ش� outer pagetable�� NULL���� üũ�ϴ� ���ǹ��� ���Ͽ� �̹� �����ϸ� �޸� �Ҵ��� ������ �ʵ��� �Ͽ� �ذ��Ͽ� ��. - how I implement deallocation �켱 vpn�� �ϳ��� page�� ����ִ� pte ������ ������ ���� outer�� �� ��°����, �������� page table�� �� ��° entry���� �˾Ƴ�����. �׷��� �ش� pte�� ���� pfn�� ã�Ƴ��� �� pfn�� �ε����� �ϴ� mapcounts�� -1 ���ְ�(�ش� pfn�� ����Ű�� �ְ� �ϳ� �������Ƿ�), �ش� pte�� pfn, valid, writable�� ��� 0���� �ʱ�ȭ���־���. �׸��� ���� validcount��� ������ �̿��Ͽ� ���� �����ִ� process�� ��� �����ϴ� �� page table�� pte���� �ݺ����� ���� ���鼭 �ϳ��� page table �ȿ� valid�� pte�� �����ϴ��� vallidcount�� ������ ���� �����ϴ� valid pte�� ���ٸ� �ش� page table�� free�����־� �����ش�. �̶� �ش� page table�� �ٷ� free �����־����� �ش� page table�� ��������� �ʾ����Ӵ��� �� ���� ���� pte�� pfn�� �� �־���. �̴� free�ϱ� ������ �ش� page table�� NULL ������ �� free �����ν� �ذ��Ͽ���. - how I implement fork current�� pid�� ��û���� pid�� ���Ͽ�, switch�Ϸ��� process�� ���� �����ִ� process��� �� �� �����ִ� process�� �״�� �ΰ�, �װ� �ƴ϶�� ��û�� process�� processes (list_head)�� �� ���ϴ��� list_for_each_entry�� �̿��Ͽ� üũ�Ѵ�. ���� processes�� �����Ѵٸ� current�� ��û�� pid�� ������ process�� �ٲ��ְ� ptbr�� �� process�� pagetable�� ����Ű���� ���ش�. �׷��� ��û�� process�� processes�� �������� �ʴ´ٸ� current process�� page table�� fork�Ѵ�. Fork�� rq_process�� �ϳ� �����Ͽ� outer page�� �޸𸮸� �Ҵ����� ��, rq_process�� pid�� ��û ���� pid�� �������ְ� current�� �����ϴ� outer page entry�� �� ���� pte���� �ϳ��ϳ� ���鼭 valid�� entry�� ���� ������ rq_process�� �Ȱ��� ��ġ�� pte ������ �״�� �������� �ش� pfn �� mapcounts�� +1 ���ش�. ���� current�� pte�� writable�� 1�̶��, ���� ���� pfn�� �����ϱ� ������ ������� write�ؼ��� �ȵǹǷ� current pte�� writable�� �Ȱ��� ��ġ�� rq_process�� pte writable�� ��� false�� ���ش�. �׸��� ���� write�� �����ѵ� fork�� �ϸ鼭 os�� write�� ���Ƴ� ���� ǥ���ϴ� �뵵�� pte�� private������ 1�� �������ش�. ó������ current�� pte�� writable�� ��쿡�� current�� rq_process�� private�� 1�� �������־� ��. �� ������, �ѹ� fork�Ǿ writable�� false�� �ֵ��� �� fork�Ͽ� ������ pte�� ������ write �� �����ϴٴ� ���� private�� ǥ�ø� �ؾ� �ϴµ�, �̷� �ֵ��� private�� 1�� ���õ��� �ʾ� �� �߿� write�� �� �� copy-on-write�� ���� �ʴ� ������ �߻��Ͽ���. ���� current�� �����ϴ� outer page entry�� �� ���� pte����, �ϳ��ϳ� ���鼭 pte�� writable�� 1������ üũ�ϴ� ���� �ƴ϶�, private�� 1������ üũ�Ͽ� �� �߿� �ϳ��� �����ϸ� ���� ��ġ�� rq_process�� pte�� private�� 1�� �ٲ��־� ���߿� write�� �� copy-on-write�� �� �� �ֵ��� �ذ����־���. - how I implement copy-on-write vpn�� �ϳ��� page �ȿ� ����ִ� entry��(NR_PTES_PER_PAGE)�� ������ � outer entry�� �� �� pte ���� �˾Ƴ��� �����Ѵ�. write�Ϸ��� �ش� pte�� private�� 1�̰� mapcount�� 1 �̻��̸� fork�� ���� ��� write�� ���Ƴ��� ���¸� ���ϹǷ� �ٸ� free pfn�� ã�� �Ҵ��Ͽ� copy �ϰ� �� ���� pfn�� mapcounts�� �ϳ� ���δ�. �Ҵ� ���� pfn�� ���� �����ϴ� �ٸ� �ְ� �����Ƿ� writable�� 1�� �ٲٰ� private�� 0���� �ٲپ� �ش�. �׸��� true�� ��ȯ�Ͽ� �ٽ� write�� �簳�� �� �ֵ��� �Ѵ�. ���� write�Ϸ��� �ش� pte�� private�� 1�̰� mapcount�� 1�̶��, �ش� pte�� pfn�� �����ϴ� �ٸ� �ֵ��� �����Ƿ� �ٸ� pfn�� ã�� �ʾƵ� �ȴ�. ���� ���� ����Ű�� �ִ� pfn�� write�ص� �ȴٴ� ���̴�. �ش� pte�� private�� 0���� ���ְ�, writable�� 1�� ���� �� true�� ��ȯ�Ͽ� write �� �簳�ϵ��� ���־���. ó���� �ش� pfn�� mapcounts�� 1�� ��� �� pfn�� ȥ�ڸ� ����ϰ������� �������� �ʰ�, private�� 1�� �͸� üũ�Ͽ� private�� 1�� �ֵ��� ��� �ٸ� free pfn�� �Ҵ��Ͽ� copy-on-write ���־� ���������� �޾�����. �̿� private�ܿ��� mapcounts�� 1���� 1 �̻����� üũ���־� 1�� �̻��� �� �ٸ� pfn�� ã�� copy-on-write�� ���ֵ��� �����Ͽ���.