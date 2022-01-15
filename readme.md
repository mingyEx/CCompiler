# CCompiler

阅读 《[Engineering a Compiler](https://book.douban.com/subject/5288601/)》  的代码参考。

就当是学画画的描图练习，重新用c++17来实现。
搞完之后还要发给老师看看，告诉他这次是我有了充分的知识和经验之后重新写的，

经验是指读过这种东西:
https://github.com/Ubpa/UVisitor

或者看过cppcon上关于tdd的视频之后学会了catch2的用法，还有log库之类的作为手脚架。


知识可以是，把此东西移植到了llvm为后端上，或者学了实现object之后加了前端代码之类的。
#### 反正是玩具，怎么玩不是玩，在学校时候还能不知天高地厚看什么PLT,现在一无所有了反倒xjb想？
### 玉女心经，技压全真；重阳一生，不弱于人。

读过一些其他的代码之后，对于如何写有了更多的认识，于是重写了这个东西。

一些原则：
重写不是重构，应该以“实现相同的功能”而非“做到源代码的逐字符替换”为目标。
只要之前不存在，被我搞出来的，就可以摇摇尾巴说是“自己写出来的”。

这个玩意我完全可以给windows和linux分别写不同的系统调用，这样就是跨平台软件了，棒不棒。
还可以加一点，以rust重写之，这样不就有学习机会了！

至于gc什么的，晚点再说。

## 没看懂的部分

### Assembly_x86
输出汇编语言的部分，这下不是借用任何工具了！
顺便还干了“映射到内存然后链接”之类的事。
重写的时候再说吧。

可以参考clang源码，待会build一下试试，这次只build clang 应该不会需要四个小时了吧...



### CodeEmitter_x86 
Instruction_x86
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
要把注释好好整理一遍。
line 40 第一遍读的时候没搞懂，挂起。
这个文件都够我水一篇博客了。

## end
写完这个之后就去读clang源码作为娱乐项目。
嗯！

## 自我激励
艾琳山看完了源码剖析能写一个MySTL,那我翻新这个玩意简直太应该了！