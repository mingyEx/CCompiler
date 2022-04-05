# CCompiler

阅读 《[Engineering a Compiler](https://book.douban.com/subject/5288601/)》  的代码参考。

## 翻新目标
1.替换掉一切标准库中存在的内容

  把raw pointer都换成sp,up,你能做好这件事吗。

2.自己实现cfg里的图算法

3.添加oop支持

3.把后端换到llvm来增加经验值。

## 一些原则：
重写不是重构，应该以**实现相同的功能**而非**做到源代码的逐字符替换**为目标。

只要之前不存在，被我搞出来的，就可以摇摇尾巴说是“自己写出来的”。

## 没看懂的部分

### Assembly_x86
输出汇编语言的部分，顺便还干了“映射到内存然后链接”之类的事。

### CodeEmitter_x86 
### Instruction_x86
这个的Emit_STOSB输出在哪里也没准。
https://stackoverflow.com/a/2547502/13792395 提到：
> \x 允许您通过其十六进制代码指定字符。

更多相关在

https://www.felixcloutier.com/x86/add
https://www.cs.virginia.edu/~evans/cs216/guides/x86.html
https://stackoverflow.com/questions/51032301/how-can-i-write-raw-machine-code-for-x86-without-using-assembly
https://www.cs.uaf.edu/2016/fall/cs301/lecture/09_28_machinecode.html
https://www.google.com/search?q=Machine+Language+Code+x86+add&oq=Machine+Language+Code+x86+add&aqs=chrome..69i57.3276j0j7&sourceid=chrome&ie=UTF-8

搜就有的东西。

### CFG代码
把注释好好整理一遍。

line 40 第一遍读的时候没搞懂，挂起。

这个文件都够我水一篇博客了。

不对，是五篇。

真的，以写清楚它们做了啥来写五篇博客完全没问题。

### InterferenceAnalysis
这里的东西也需要再搞一遍，就当是复习算法题了。

...
到了优化那一部分已经完全想不起来了。
前端部分先写写试试吧，就当是练习现代cpp了。

### 动机:练习c++
艾琳山看完了源码剖析能写一个MySTL,那我翻新这个玩意简直太应该了！

用c++17来实现，就当是学画画的描图练习，类似mq的玩法来`练习c++和设计模式`
https://github.com/MichaelSuen-thePointer/mqLinq
既然没有人写好框架来让我做填空题，就只好自己抄了 >_<
