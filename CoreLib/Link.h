#ifndef CORE_LIB_LINK_H
#define CORE_LIB_LINK_H

#include "Common.h"
#include "Exception.h"

namespace CoreLib
{
	namespace Basic
	{
		template<typename T>
		class LinkedNode
		{
			template<typename T1>
			friend class LinkedList;		//向 class LinkedList 授予对 LinkedNode 的私有及受保护成员的访问权。
		private:
			LinkedNode<T> *pPrev, *pNext;	//指向前一个和后一个节点，双链表
			LinkedList<T> * FLink;	// 为了在操作之后修改所属于的链表的属性
		public:
			T Value;
			//为什么是以FLink为构造参数的? 总之我们知道函数操作跟名字一样。  因为每个节点都是要在FLink上进行操作的，鸭！
			LinkedNode (LinkedList<T> * lnk):FLink(lnk)
			{
				pPrev = pNext = 0;
			};
			LinkedNode<T> * GetPrevious()
			{
				return pPrev;
			};
			LinkedNode<T> * GetNext()
			{
				return pNext;
			};
			LinkedNode<T> * InsertAfter(const T & nData)	
			{
				LinkedNode<T>* n = new LinkedNode<T>(FLink);
				n->Value = nData;
				n->pPrev = this;
				n->pNext = this->pNext;
				LinkedNode<T> *npp = n->pNext;
				if (npp)//验证存在才设定。
				{
					npp->pPrev = n;	
				}
				pNext = n;	//而this的next就该是n了。
				if (!n->pNext)
					FLink->FTail = n;//设置整个链表的相关内容
				FLink->FCount ++;
				return n;
			};
			LinkedNode<T> * InsertBefore(const T & nData)
			{
				LinkedNode<T> * n = new LinkedNode<T>(FLink);
				n->Value = nData;
				n->pPrev = pPrev;
				n->pNext = this;
				pPrev = n;
				LinkedNode<T> *npp = n->pPrev;
				if (npp)
					npp->pNext = n;
				if (!n->pPrev)
					FLink->FHead = n;
				FLink->FCount ++;
				return n;
			};
			void Delete()
			{
				//删除当前节点
				if (pPrev)
					pPrev->pNext = pNext;
				if (pNext)
					pNext->pPrev = pPrev;

				FLink->FCount --;		//链表内元素数量的统计.
				if (FLink->FHead == this)
				{
					FLink->FHead = pNext;
				}
				if (FLink->FTail == this)
				{
					FLink->FTail = pPrev;
				}
				delete this;
			}
		};
		template<typename T>
		class LinkedList			//==std::list

		{
			template<typename T1>
			friend class LinkedNode;
		private:
			LinkedNode<T> * FHead, *FTail;					//自己的头和尾
			int FCount;										//数量统计
		public:

			class Iterator
			{
			public:
				LinkedNode<T> * Current, *Next;		//Current 代表的是迭代器指向的当前节点
				void SetCurrent(LinkedNode<T> * cur)
				{
					Current = cur;
					if (Current)
						Next = Current->GetNext();
					else
						Next = 0;
				}
				Iterator(LinkedNode<T> * cur)
				{
					SetCurrent(cur);
				}
				T & operator *() const
				{
					return Current->Value;
				}
				Iterator& operator ++()
				{
					SetCurrent(Next);
					return *this;
				}
				Iterator operator ++(int)
				{
					Iterator rs = *this;
					SetCurrent(Next);
					return rs;
				}
				bool operator != (const Iterator & iter) const
				{
					return Current != iter.Current;
				}
				bool operator == (const Iterator & iter) const
				{
					return Current == iter.Current;
				}
			};
		
			Iterator begin() const
			{
				return Iterator(FHead);
			}
			Iterator end() const
			{
				return Iterator(0);
			}
		public:
			LinkedList() : FHead(0), FTail(0), FCount(0)
			{
			}
			~LinkedList()
			{
				Clear();
			}
			LinkedList(const LinkedList<T> & link) : FHead(0), FTail(0), FCount(0)
			{
				this->operator=(link);
			}
			LinkedList(LinkedList<T> && link) : FHead(0), FTail(0), FCount(0)
			{
				this->operator=(_Move(link));
			}
			LinkedList<T> & operator = (LinkedList<T> && link)
			{
				if (FHead != 0)
					Clear();
				FHead = link.FHead;
				FTail = link.FTail;
				FCount = link.FCount;
				link.FHead = 0;
				link.FTail = 0;
				link.FCount = 0;
				return *this;
			}
			LinkedList<T> & operator = (const LinkedList<T> & link)
			{
				if (FHead != 0)
					Clear();
				auto p = link.FHead;
				while (p)
				{
					AddLast(p->Value);
					p = p->GetNext();
				}
				return *this;
			}
			template<typename IteratorFunc>
			void ForEach(const IteratorFunc & f)
			{
				auto p = FHead;
				while (p)
				{
					f(p->Value);
					p = p->GetNext();
				}
			}
			LinkedNode<T> * GetNode(int x)
			{
				LinkedNode<T> *pCur = FHead;
				for (int i=0;i<x;i++)
				{
					if (pCur)
						pCur = pCur->pNext;
					else
						throw "Index out of range";
				}
				return pCur;
			};
			LinkedNode<T> * Find(const T& fData)
			{
				for (LinkedNode<T> * pCur = FHead; pCur; pCur = pCur->pNext)
				{
					if (pCur->Value == fData)
						return pCur;
				}
				return 0;
			};
			LinkedNode<T> * FirstNode()
			{
				return FHead;
			};
			T & First()
			{
				if (!FHead)
					throw IndexOutofRangeException("LinkedList: index out of range.");
				return FHead->Value;
			}
			T & Last()
			{
				if (!FTail)
					throw IndexOutofRangeException("LinkedList: index out of range.");
				return FTail->Value;
			}
			LinkedNode<T> * LastNode()
			{
				return FTail;
			};
			LinkedNode<T> * AddLast(const T & nData)
			{
				LinkedNode<T> * n = new LinkedNode<T>(this);
				n->Value = nData;
				n->pPrev = FTail;
				if (FTail)
					FTail->pNext = n;
				n->pNext = 0;
				FTail = n;
				if (!FHead)
					FHead = n;
				FCount ++;
				return n;
			};
			// Insert a blank node
			LinkedNode<T> * AddLast()
			{
				LinkedNode<T> * n = new LinkedNode<T>(this);
				n->pPrev = FTail;
				if (FTail)
					FTail->pNext = n;
				n->pNext = 0;
				FTail = n;
				if (!FHead)
					FHead = n;
				FCount ++;
				return n;
			};
			LinkedNode<T> * AddFirst(const T& nData)
			{
				LinkedNode<T> *n = new LinkedNode<T>(this);
				n->Value = nData;
				n->pPrev = 0;
				n->pNext = FHead;
				if (FHead)
					FHead->pPrev = n;
				FHead = n;
				if (!FTail)
					FTail = n;
				FCount ++;
				return n;
			};
			void Delete(LinkedNode<T>*n, int Count = 1)
			{
				LinkedNode<T> *n1,*n2 = 0, *tn;
				n1 = n->pPrev;
				tn = n;
				int numDeleted = 0;
				for (int i=0; i<Count; i++)
				{
					n2 = tn->pNext;
					delete tn;
					tn = n2;
					numDeleted++;
					if (tn == 0)
						break;
				}
				if (n1)
					n1->pNext = n2;
				else
					FHead = n2;
				if (n2)
					n2->pPrev = n1;
				else
					FTail = n1;
				FCount -= numDeleted;
			}
			void Clear()
			{
				for (LinkedNode<T> *n = FHead; n; )
				{
					LinkedNode<T> * tmp = n->pNext;
					delete n;
					n = tmp;
				}
				FHead = 0;
				FTail = 0;
				FCount = 0;
			}
			List<T> ToList() const
			{
				List<T> rs;
				rs.Reserve(FCount);	// Reserve 是储存不是反转
				for (auto & item : *this)
				{
					rs.Add(item);
				}
				return rs;
			}	
			int Count()
			{
				return FCount;
			}
		};
	}
}
#endif