#ifndef COMPILER_TRANSFORM_INVARIANT_h
#define COMPILER_TRANSFORM_INVARIANT_h

#include "CFG.h"

namespace Compiler
{
	namespace Intermediate
	{
		class TransformInvariant
		{
		public:
			static void PlacePhiAtTop(ControlFlowNode * node);
		};
	}
}

#endif