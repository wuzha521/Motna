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
    Table* Ptr = Head.Node; // 当前节点初始化为头节点的下一个节点
    Table* Ptr2 = &Head;    // 前一个节点初始化为头节点

    while (Ptr) {
        if (Ptr->Data == Data) {
            // 找到要删除的节点
            if (Ptr2 == &Head) {
                // 如果是删除头节点的直接后继
                Head.Node = Ptr->Node; // 头节点的Node指向新的第一个节点
            }
            else {
                // 否则，将前一个节点的Node指向当前节点的下一个节点
                Ptr2->Node = Ptr->Node;
            }
            delete Ptr;                  // 删除当前节点，释放内存
            return;                       // 删除完成后退出函数
        }
        Ptr2 = Ptr; // 移动前一个节点指针
        Ptr = Ptr->Node; // 移动当前节点指针
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


