# CCompiler

阅读 《[Engineering a Compiler](https://book.douban.com/subject/5288601/)》  的代码参考，目标是学会编译器的各个部分的实现代码，以及ssa的φ函数的实现。

已经知道个大概了，无非是那几个图算法，遍历，找边界，插入φ函数，然后改名字为ssa形式，而已。

但是每个自己都写不出来，把这些算法都实现出来并且把中间的转换过程和缺失的东西都补上。

https://www.zhihu.com/question/24992774
https://github.com/mingy532/CCompiler/blob/master/IL/Optimizer_UselessInstructionRemoval.cpp#L180
很多地方只是”看懂了“而已。 为甚么如此而非ruby. 背后是什么算法，怎么分析复杂度，都没数..

https://stackoverflow.com/questions/33379145/equivalent-of-python-map-function-using-lambda


前人的玩具代码。
todo:
SimpleC 部分搞定。
按顺序理清IR部分的注释，和每一行代码的意思，到cfg.cpp了。

按照某人做的，将每个部分都替换成标准库||准标准库。 
尝试加入class,lambda,gc等。

弃坑,还是去看看主世界的东西什么样,遇到好玩的加进来.
