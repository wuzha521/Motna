#include "Table.h"
void Tables::AddTable(Table& Head, DataType Data) {
    if (FindTable(Head, Data))
        return;
    Table* NewTable = new Table;
    NewTable->Data = Data;
    NewTable->Node = NULL;
    Table* Ptr = &Head;
    while (Ptr->Node)
        Ptr = Ptr->Node;
    Ptr->Node = NewTable;


}

void Tables::SubTable(Table& Head, DataType Data) {
    Table* Ptr = Head.Node; // ��ǰ�ڵ��ʼ��Ϊͷ�ڵ����һ���ڵ�
    Table* Ptr2 = &Head;    // ǰһ���ڵ��ʼ��Ϊͷ�ڵ�

    while (Ptr) {
        if (Ptr->Data == Data) {
            // �ҵ�Ҫɾ���Ľڵ�
            if (Ptr2 == &Head) {
                // �����ɾ��ͷ�ڵ��ֱ�Ӻ��
                Head.Node = Ptr->Node; // ͷ�ڵ��Nodeָ���µĵ�һ���ڵ�
            }
            else {
                // ���򣬽�ǰһ���ڵ��Nodeָ��ǰ�ڵ����һ���ڵ�
                Ptr2->Node = Ptr->Node;
            }
            delete Ptr;                  // ɾ����ǰ�ڵ㣬�ͷ��ڴ�
            return;                       // ɾ����ɺ��˳�����
        }
        Ptr2 = Ptr; // �ƶ�ǰһ���ڵ�ָ��
        Ptr = Ptr->Node; // �ƶ���ǰ�ڵ�ָ��
    }
}

BOOL Tables::FindTable(Table& Head, DataType Data)
{
	Table* Ptr = &Head;
	while (Ptr)
		if (Ptr->Data == Data)
			return true;
		else Ptr = Ptr->Node;
	return false;
}


