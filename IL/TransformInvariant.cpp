#include "TransformInvariant.h"

namespace Compiler
{
	namespace Intermediate
	{
		//这个函数是干嘛的?
		void TransformInvariant::PlacePhiAtTop(ControlFlowNode * node)
		{
			if (node->Code.Count() == 0)
				return;
			InstructionNode * firstNonPhi = node->Code.FirstNode();
			while (firstNonPhi->Value.Func == Operation::Phi)
				firstNonPhi = firstNonPhi->GetNext();
			auto inode = firstNonPhi->GetNext();
			while (inode)
			{
				auto nxt = inode->GetNext();
				if (inode->Value.Func == Operation::Phi)
				{
					firstNonPhi->InsertBefore(inode->Value);
					inode->Delete();
				}
				inode = nxt;
			}
		}
	}
}
