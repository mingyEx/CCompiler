## CCompiler
   
This project comes from the code archive of [He Yong](https://www.csyong.net/index.html)  
Just as a reference for my reading of [Engineering a Compiler, 2nd Edition](https://book.douban.com/subject/20436488/).  
I want to increase my knowledge of compilers and exercise my ability to read code, engineering and refactoring by studying and refactoring it.

## todo

0.Completely understand the meaning of [this line](https://github.com/KargathEx/CCompiler/blob/master/IL/CFG.cpp#L48) of code
1.Replace as much functionality as possible with functions that exist in the standard library  
2.try to avoid the use of raw pointers  
3.Try adding support for classes and object-oriented features  
4.use LLVM to produce LLVM IR  
5.Write clear documentation and comments  
就算是中文，[扯淡的地方](https://www.zhihu.com/question/23697873/answer/25411314)也是很多的,保持动力把它撸完吧！
## The part I don't understand yet

Assembly_x86  
CodeEmitter_x86  
Instruction_x86  
InterferenceAnalysis  
https://stackoverflow.com/a/2547502/13792395 提到：

> \x 允许您通过其十六进制代码指定字符。

something more:

https://www.felixcloutier.com/x86/add  
https://www.cs.virginia.edu/~evans/cs216/guides/x86.html  
https://stackoverflow.com/questions/51032301/how-can-i-write-raw-machine-code-for-x86-without-using-assembly  
https://www.cs.uaf.edu/2016/fall/cs301/lecture/09_28_machinecode.html  
https://www.google.com/search?q=Machine+Language+Code+x86+add&oq=Machine+Language+Code+x86+add&aqs=chrome..69i57.3276j0j7&sourceid=chrome&ie=UTF-8  

### intention
Practicing my programming skills by reinventing the wheel,just like [MySTL](https://github.com/Alinshans/MyTinySTL) and [mqLinq](https://github.com/MichaelSuen-thePointer/mqLinq)  
rewrote it with new syntax and standard library facilities to Improve my programming skills

### update:  
我错了，想知道一个项目每个部分是负责什么的，应该去读设计文档，读wiki,或者直接去问作者，而不是一行行看代码。  
对我来说“文档”就是那本橡书。  

只有需要改动代码的时候才需要去看代码本身,靠读每一行代码来搞懂在干什么是本末倒置低效的做法。  
只有自己写类似的东西遇到某个功能 不会写/写不好 的时候才有必要去看看“别人是怎么做的”，又或者纯粹想要看高质量代码来提高自己的编写习惯，否则一行行的读别人的代码是没有意义的。  
虽然阅读过程中语法熟练度up,需要自己写指针的时候也因为看过这个项目而“有现成的经验可以直接拿来用”，但仅此而已。  

想清楚这点之后就打算自己重写这个项目，到AST里面繁多的指针到底该用哪种来表达所有权的时候彻底懵逼了，因为没有对这个项目整体的概念(也不想边画UML图边梳理这些)，且原作者自己封的指针与std里的接口有一些区别而不能简单替换。   

这个项目对我唯一的价值就是“培养了从没有文档的项目里一点点搞懂的能力，无论是查找相关伪代码，还是修改/输出各种中间内容验证假设”。这些能力可能对于干工程上的脏活累活挺有用的，但是正经学习姿势显然不是这样。  
学习软件开发的姿势多种多样，初代游戏程序员在卧室里改别人的代码，野路子自学党啃各种书参与各种项目，学院派一路理论代码风格感人，`爱好者`们广博的知识面，都有所成。  
我至今仍然不知道他们是怎么学会那么多东西的，也不想知道了。  
岩石的重量使人安心。  
另：《Code Reading: The Open Source Perspective》是给外行/初学者看的，没什么营养。 

