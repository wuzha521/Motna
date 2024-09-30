#pragma once
#include "‘§±‡“Î.h"

namespace Tables
{
	typedef DWORD64 DataType;
	struct Table
	{
		DataType Data;
		Table* Node;
	};
	void AddTable(Table& Head, DataType Data);
	void SubTable(Table& Head, DataType Data);
	BOOL FindTable(Table& Head, DataType Data);

}