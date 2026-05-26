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
			InstructionNode * firstNonPhi = FirstInstructionNode(node->Code);
			while (firstNonPhi->Value.Func == Operation::Phi)
				firstNonPhi = NextInstructionNode(firstNonPhi);
			auto inode = NextInstructionNode(firstNonPhi);
			while (inode)
			{
				auto nxt = NextInstructionNode(inode);
				if (inode->Value.Func == Operation::Phi)
				{
					InsertInstructionBefore(firstNonPhi, inode->Value);
					RemoveInstruction(inode);
				}
				inode = nxt;
			}
		}
	}
}
