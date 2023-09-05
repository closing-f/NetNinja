<!--
 * @Author: closing
 * @Date: 2023-06-06 21:06:22
 * @LastEditors: closing
 * @LastEditTime: 2023-08-30 11:31:07
 * @Description: 请填写简介
-->
<font size=5>
## 导读

条款01:视C++为一个语言联邦

1. copy构造函数被用来“以同型对象**初始化**自我对象”,copy assignmnent操作符被用来“从另一个同型对象中拷贝其值到自我对象“。如果一个新对象被定义, 一定会有个构造函数被调用，不可能调用赋值操作。**如果没有新对象被定义, 就不会有构造函数被调用**，那么当然就是赋值操作被调用

条款02:尽量以const,enum,inline替换#define

1. 对于单纯常量，最好以const对象或enumg替换Adefines；对于形似函数的宏(macrog),最好改用inline函数待换defines。

条款03:尽可能使用const

1. 如果关键字const出现在星号左边，表示被指物是常量：如果出现在星号右边，表示指针自身是常量
2. 两个成员函数如果只是常量性(constness)不同，可以被重载。
3. 在C++中，只有被声明为const的成员函数才能被一个const类对象调用。
4. 对于const成员函数，编译器强制实施bitwise constness，只需寻找成员变量的赋值动作即可。一个更改了“指针所指物”的成员函数虽然不能算是const,但如果只有指针(而非其所指物)隶属于对象，那么称此函数为bitvise const不会引发编译器异议
5. 当const和non-const成员函数有着实质等价的实现时，令non-const版本调用const版本可避免代码重复。

条款04:确定对象被使用前已先被初始化

1. C++规定，对象的成员变量的初始化动作发生在进入构造函数本体之前。用所谓的member initialization list(成员初值列)替换赋值动作效率会更高

   ```c++
   ABEntry::ABEntry (const std::stringt name, const std::strings address,const std::list<PhoneNumber>6 phones)
   :theName(name),
   theAddress (address),//现在，这些都是初始化(initializations) 
   thePhones (phones),
   numTimesConsulted(0)
   {} //现在，构造函数本体不必有任何动作
   ```

   基于赋值的那个版本(本例第一版本)首先调用defaut构造函数为theMame, theAddress和thePhones设初值，然后立刻再对它们赋予新值；成员初值列(member initialization list)的做法避免了这一问题，因为初值列中针对各个成员变量而设的实参，被拿去作为各成员变量之构造函数的实参
2. C++对“定义于不同的编译单元内的noo-local static对象”的初始化相对次序并无明确定义。为了解决顺序问题，将每个non-local static对象搬到自己的专属函数内(**该对象在此函数内被声明为static**)。这些函数返回一个reference指向它所含的对象。然后用户调用这些函数，而不直接指涉这些对象（C++保证，函数内的local static对象会在“该函数被调用期间”“首次遇上该对象之定义式”时被初始化）

## 构造/析构/赋值运算

条款05:了解C++默默编写并调用哪些函数

1. 如果你打算在一个“内含reference成员”的class内支持赋值操作(assignment),你必须自己定义copy assignment操作符。面对“内含const成员”(如本例之objectValue)的classes, 编译器的反应也一样。

条款06:若不根使用编译器自动生成的函数，就该明确拒绝

1. 为不支持类拷贝，可以在一个专门为了阻止copying动作而设计的base class内，将copy构造函数和capy assignment操作符声明为private，这样可以将报错（友元调用该private函数)从链接期提前至编译期。

条款07:为多态基类声明virtual析构函数

1. 为你希望它成为抽象的那个class声明一个pure virtual析构函数。但你必须为这个pure virtual析构函数提供一份定义：析构函数的运作方式是，最深层派生(mast derived)的那个class其析构函数最先被调用，然后是其每一个base class的析构函数被调用。编译器会在derived classes的析构函数中创建一个对基类析构函数的调用动作。

条款08：别让异常逃离析构函数

1. 析构函数绝对不要吐出异常。如果一个被析构函数调用的函数可能抛出异常，析构函数应该捕捉任何异常，然后吞下它们(不传播)或结束程序。
2. 如果客户需要对某个操作函数运行期间抛出的异常做出反应，那么class应该提供一个普通函数(而非在析构函数中)执行该操作。

条款09:绝不在构造和析构过程中调用virtual函数

1. base class构造期间virtual函数绝不会下降到derived classes阶层
2. 在derived class对象的base class构造期间，对象的类型是base class而不是derived class
3. 一旦derived class析构函数开始执行，对象内的derived class成员变量便呈现未定义值，所以C++视它们仿佛不再存在。进入base class构函数后对象就成为一个base class对象。

条款10:令operator=返回一个reference to*this

条款11:在operator=中处理“自我赋值”

1. 确保当对象自我赋值时operator=有良好行为。其中技术包括比较“来源对象”和“目标对象”的地址、精心周到的语句顺序、以及copy-and-swap。

条款12:复制对象时勿忘其每一个成分
1. 当你编写一个copying函数，请确保(1)复制所有local成员变量，(2)调用所有base classes内的适当的copying函数
2. 令某个copying函数调用另一个copying函数却无法让你达到你想要的目标。
3. 令copy assigmment操作符调用copy构造函数是不合理的，因为这就像试图构造一个已经存在的对象
4. 令copy构造函数调用copy assignment操作符同样无意义。构造函数用来初始化新对象，而assignment操作符只施行于已初始化对象身上
5. 如果你发现你的copy构造函数和copy assignmemt操作符有相近的代码，消除重复代码的做法是，建立一个新的成员函数给两者调用

## 资源管理
条款13：以对象管理资源
条款14：在资源管理类中小心coping行为
条款15：在资源管理类中提供对原始资源的访问
1. APls往往要求访问原始资源(raw tesources),所以每一个RAII class应该提供一个“取得其所管理之资源”的办法。
2. 对原始资源的访问可能经由显式转换或隐式转换。一般而言显式转换比较安全，但隐式转换对客户比较方便。

条款16: 成对使用new和delete时要来取相同形式
条款17:以独立语句将newed对象置入智能指针

## 设计与声明
条款18: 让接口容易被正确使用，不易被误用
条款19：设计class犹如设计type
每次设计class时最好在脑中过一遍以下问题：
1. 对象该如何创建销毁：包括构造函数、析构函数以及new和delete操作符的重构需求。
2. 对象的构造函数与赋值行为应有何区别：构造函数和赋值操作符的区别，重点在资源管理上。
3. 对象被拷贝时应考虑的行为：拷贝构造函数。
4. 对象的合法值是什么？最好在语法层面、至少在编译前应对用户做出监督。
5. 新的类型是否应该复合某个继承体系，这就包含虚函数的覆盖问题。
6. 新类型和已有类型之间的隐式转换问题，这意味着类型转换函数和非explicit函数之间的取舍。
7. 新类型是否需要重载操作符。
8. 什么样的接口应当暴露在外，而什么样的技术应当封装在内（public和private）
9. 新类型的效率、资源获取归还、线程安全性和异常安全性如何保证。
10. 这个类是否具备template的潜质，如果有的话，就应改为模板类。

条款20：宁以pass-by-reference-to-const替换pass-by-value
函数接口应该以const引用的形式传参，而不应该是按值传参，否则可能会有以下问题：

1. 按值传参涉及大量参数的复制，这些副本大多是没有必要的。
2. 如果拷贝构造函数设计的是深拷贝而非浅拷贝，那么拷贝的成本将远远大于拷贝某几个指针。
3. 对于多态而言，将父类设计成按值传参，如果传入的是子类对象，仅会对子类对象的父类部分进行拷贝，即部分拷贝，而所有属于子类的特性将被丢弃，造成不可预知的错误，同时虚函数也不会被调用。
4. 小的类型并不意味着按值传参的成本就会小。首先，类型的大小与编译器的类型和版本有很大关系，某些类型在特定编译器上编译结果会比其他编译器大得多。小的类型也无法保证在日后代码复用和重构之后，其类型始终很小。

条款21：必须返回对象时，别妄想返回其reference
1. 但是对于C++11以上的编译器，我们可以采用给类型编写“转移构造函数”以及使用std::move()函数更加优雅地消除由于拷贝造成的时间和空间的浪费
条款22：将成员变量声明为private
条款23：宁以non-member, non-friend替换member函数
1. 需要直接访问private成员的public和protected成员函数称为功能颗粒度较低的函数;由若干其他public（或protected）函数集成而来的public成员函数，我愿称之为颗粒度高的函数, 这些函数应该尽可能放到类外。
2. 将高颗粒度函数提取至类外部可以允许我们从更多维度组织代码结构，并优化编译依赖关系.

条款24：若所有参数皆需类型转换，请为此采用non-member函数
1. 如果我们在使用操作符时希望操作符的任意操作数都可能发生隐式类型转换，那么应该把该操作符重载成非成员函数
   1. 如果一个操作符是成员函数，那么它的第一个操作数（即调用对象）不会发生隐式类型转换
   2. 编译器正式根据第一个操作数的类型来确定被调用的操作符到底属于哪一个类的。因而第一个操作数是不会发生隐式类型转换的

条款25：考虑写出一个不抛出异常的swap函数
TODO: 阅读C++ primer中类swap相关内容

## 实现
条款26:尽可能延后变量定义式的出现时间
1. 你不只应该延后变量的定义，直到非得使用该变量的前一刻为止，甚至应该尝试延后这份定义直到能够给它初值实参为止。如果这样，不仅能够避兔构造(和析构)非必要对象，还可以避免无意义的defout构造行为。

条款27:尽量少做转型动作
1. C++新式转型
   1. const_cast通常被用来将对象的常量性转除
   2. dynamic_cast.主要用来执行“安全向下转型”(safe downcasting),也就是用来决定某对象是否妇属继承体系中的某个类型
   3. statlc_cast用来强迫隐式转换：将non-const对象转为const对象，将int转为double等等
2. 
``` c++
class SpecialWindow: public Window {//derived class
public:
   virtual void onResize(){ //derived onResize实现代码
      static_cast<Window>(*this).onResize();//将*this转型为window,然后调用其onResize,这不可行
      ... //这里进行SpecialWindow专属行为.//
   }
}
```
它调用的并不是当前对象上的函数，而是稍早转型动作所建立的一个“*this对象之base class成分”的暂时副本身上的orResize。如果你只是想调用base class版本的onResize函数，令它作用于当前对象身上请使用
```c++
Window::onResize();
```
3. 之所以需要dynamic_cast,通常是因为你想在一个你认定为derived class对象身上执行derived class操作函数，但你的手上却只有一个“指向base”的pointer或reference,你只能靠它们来处理对象
   
条款28:避免返回handles指向对象内部成分
1. References、指针和迭代器统统都是所调的handles(号码牌，用来取得某个对象),而返回一个“代表对象内部数据”的handle,随之而来的便是“降低对象封装性”的风险, 可以通过对他们的返回值添加const解决
2. 返回handles就会暴露在“handle比其所指对象更长寿”的风险下, 可能会导致dangling handles：这种handles所指东西(的所属对象)不复存在。

条款29:为“异常安全”而努力是值得的
1. 异常安全函数(Exception-safe functions)即使发生异常也不会泄漏资源或允许任何数据结构败坏。这样的函数区分为三种可能的保证：基本型、强烈型、不抛异常型。
2. “强烈保证”往往能够以copy-and-swap实现出来（创建一个副本，先保留原来数据），但“强烈保证”并非对所有函数都可实现或具备现实意义。
3. 函数提供的“异常安全保证”遮常最高只等于其所调用之各个函数的“异常安
全保证”中的最弱者

条款30:透彻了解inlining的里里外外
1. Inline只是对编译器的一个申请，不是强制命令。隐喻方式是将函数定义于class定义式内；明确声明inline函数的做法则是在其定义式前加上关键字inline。
2. inlining在大多数C++程序中是编译期行为。
3. 编译器通常不对“通过函数指针而进行的调用”实施inlining,

条款31:将文件间的编译依存关系降至最低
1. 常用方法：
   1. 如果使用object references或object points可以完成任务，就不要使用objects
   //TODO: why
   2. **如果能够，尽量以class声明式替换class定义式。注意，当你声明一个函数而它用到某个class时，你并不需要该class的定义；纵使函数以by value方式传递该类型的参数(或返回值)亦然**：
   3. 为声明式和定义式提供不同的头文件，程序库客户应该总是include一个声明文件而非前置声明若干函数。

2. 支持“编译依存性最小化”的一般构想是；相依于声明式，不要相依于定义式。基于此构想的两个手段是Handle classes和Interface classes.
   1. Handle casses和Interface dlesses解除了接口和实现之间的耦合关系，从而降低文件间的编译依存性
   2. 缺点：
      1. 在Handle casses身上，成员函数必须通过implementation pointer取得对象数据。那会为每一次访问增加一层间接性。而每一个对象消耗的内存数量必须增加implementation pointer的大小。最后，implementation pointer必须初始化(在Handle class构造函数内),指向一个动态分配得来的implementation cbject,所以你将兼受因动态内存分配(及其后的释放动作)而来的额外开销，以及遭遇bad_alloc异常(内存不足)的可能性。
      2. 至于Interface classes,由于每个函数都是virtual,所以你必须为每次函数调用付出一个间接跳跃(indirect jump)成本。此外Interface class派生的对象必须内含一个vptr(virual table pointer),这个指针可能会增加存放对象所需的内存数量——实际取决于这个对象除了Interface class之外是否还有其他virtual函数来源。
3. 程序库头文件应该以“完全且仅有声明式”的形式存在。这种做法不论是否涉及templates都适用。

## 6 继承与面向对象设计

条款32:确定你的public继承塑模出is-a关系
1. public继承”意味is-a。适用于base classes身上的每一件事情一定也适用于derived classes身上，因为每一个derived class对象也都是一个base class对象。

条款33:避免遮掩继承而来的名称
1. 如果你继承base class并加上重载函数，而你又希望重新定义或覆写(推翻)其中一部分，那么你必须为那些原本会被遮掩的每个名称引入一个using声明式，否则某些你希望继承的名称会被遮掩。
2. 如果只想把父类某个在子类中某一个已经不可见的同名函数复用，可使用inline forwarding function
```c++
virtual void mf1()//转交函数(forwarding function);
{ Base::mfl();} //暗自成为inline
```
条款34:区分接口继承和实现继承 
1. 声明一个pure virual函数的日的是为了让derived classes只继承函数接口
2. 我们竟然可以为pure virual函数提供定义。也就是说你可以为Shape::draw供应一份实现代码，C++并不会发出怨言，但调用它的唯一途径是“调用时明确指出其class名称”;
3. pure virtual函数、simple(impure) virtual函数、non-virtual函数之间的差异，使你得以精确指定你想要derived classes继承的东西：只继承接口，或是继承接口和份缺省实现，或是继承接口和一份强制实现

条款35:考虑virtual函数以外的其他选择
1. 使用non-virtual interface(NVI)手法。它以public non-virtual成员函数包裹较低访问性(private或protected)的virtual函数。
2. 以std::function成员变量替换virtual函数，因而允许使用任何可调用物(callable entity)搭配一个兼容于需求的签名式
3. 将继承体系内的virtual函数替换为另一个继承体系内的virtual函数(内含一个指针成员变量指向其类对象)

条款36:绝不重新定义继承而来的non-virtual函数
条款37:绝不重新定义继承而来的缺省参数值
1. 绝对不要重新定义一个继承而来的缺省参数值，因为缺省参数值都是静态绑定，而virtual函数却是动态绑定。
2. 虚函数不要写缺省参数值，子类自然也不要改，虚函数要从始至终保持没有缺省参数值
条款38:通过复合塑模出has-a或"根据某物实现出"
条款39:明智而审慎地使用private继承
1. Private继承意味is-implemented-in-terms of(根据某物实现出)。它通常比复合(commposition)的级别低。但是当derived class需要访问protected base class的成员，或需要重新定义继承而来的virtual函数时，这么设计是合理的。
2. 和复合(composition)不同，private继承可以造成empty base最优化。这对致力于“对象尺寸最小化”的程序库开发者而言，可能很重要。
   1. “独立(非附属)”对象的大小一定不为零。也就是说，这个约束不适用于derived clasg对象内的base class成分，因为它们并非独立(非附属)。如果你继承Zmpty,而不是内含一个那种类型的对象:
   ```c++
   class HoldsAnInt:private Empty {
   private:
   int x;
   }
   ```
   几乎可以确定sizeof(HoldsAnInt)=sizeof(1nt)。这是所谓的EBO(empty base optimization)空白基类最优化
条款40:明智而审慎地使用多重继承
1. 与C++用来解析(resolving)重载函数调用的规则相符：在看到是否有个函数可取用之前，C++首先确认这个函数对此调用之言是最佳匹配。找出最佳匹配函数后才检验其可取用性. 为了解决这个歧义，你必须明白指出你要调用哪一个base class内的函数：mp.BorrowableItem::checkout(); //哎呀，原来是这个checkDOut...
2. virtual base的初始化责任是由继承体系中的最低层(mOst derived)class负责，这暗示:
   1. classes若派生自virtual bases而需要初始化，必须认知其virtual bases——不论那些bases距离多远.
   2. 当一个新的derived class加入继承体系中，它必须承担其virtual bases(不论直接或间接)的初始化责任

## 模板与泛型编程
条款41:了解隐式接口和编译期多态
1. classes和template都支持接口(interfaces)和多态(polymorphism)。对classes而言接口是显式的(explicit),以函数签名为中心。多态则是通过virtual
函数发生于运行期。
2. 对template参数而言，接口是隐式的(implicit),奠基于有效表达式。多态则是通过template具现化和函数重载解析(function overloading resolution)发生于
编译期。

条款42:了解typename的双重意义
1. 在我们知道C是什么之前，没有任何办法可以知道C::const_iterator是否为一个类型。而当编译器开始解析时，尚未确知C是什么东西。C++有个规则可以解析(resolve)此一歧义状态：如果解析器在template中遭遇一个嵌套从属名称，它便假设这名称不是个类型，除非你告诉它是。所以缺省情况下嵌套从属名称不是类型。
2. 一般性规则很简单：任何时候当你想要在template中指涉一个嵌套从属类型名称，就必须在紧临它的前一个位置放上关键字typename
3. typename只被用来验明嵌套从属类型名称；其他名称不该有它存在
4. “typename必须作为银套从属类型名称的前缀词”这一规则的例外是，typename不可以出现在base classes list内的嵌套从属类型名称之前，也不可在merber initialization list(成员初值列)中作为base class修饰符。

条款43:学习处理模板化基类内的名称
1. 编译器往往拒绝在templatized base classes(模板化基类，本例的MsgSender<Company>)内寻找继承而来的名称(本例的Sendclear)：它知道base class templates有可能被特化，而那个特化版本可能不提供和一般性template相同的接口。
2. 为了令C++“不进入templatized base classes观察”的行为失效。有三个办法：
   1. 第一是在base class函数调用动作之前加上"this->" 
   ```c++ 
   this->sendClear(info);//成立，假设sendClear将被继承，
   ```
   2. 使用using声明式
   ```c++ 
   using Msgsender<Company>::sendClear; //告诉编译器，请它假设sendClear位于base class内。
   ```
   这里的情况并不是base clans名称被derived elass名称遮掩，而是编译器不进入base class作用域内查找，于是我们通过using告诉它，请它那么做。
   3. 明白指出被调用的函数位于base class内：
   ```c++ 
   MsgSender<Company>::sendClear(info);:
   ```
   但这往往是最不让人满意的一个解法，因为如果被调用的是virtual函数，上述的明确资格修饰(explicit qualiication)会关闭“virtual绑定行为”。

条款44:将与参数无关的代码抽离templates
TODO:如何制造一个inline调用
1. Templates生成多个classes和多个函数，所以任何template代码都不该与某个造成膨胀的template参数产生相依关系。
2. 因非类型模板参数(non-type temulate parameters)而造成的代码膨胀，往往可消除，方法是以函数参数或class成员变量替换template参数。

条款45:运用成员函数模板接受所有兼容类型
1. 请使用member function templates(成员函数模板)生成“可接受所有兼容类型”的函数。
2. 如果你声明member templates用于 “泛化copy构造” 或 “泛化assignment操作”,你还是需要声明正常的copy构造函数和copy assignmemt操作符(在class内声明泛化copy构造函数(是个member template)并不会阻止编译器生成它们自己的capy构造函数(一个non-template),所以如果你想要控制copy构造的方方面面，你必须同时声明泛化apy构造函数和“正常的”copy构造函数)
条款46:需要类型转换时请为模板定义非成员函数
1. 在template实参推导过程中从不将隐式类型转换函数纳入考虑
2. 为了让类型转换可能发生于所有实参身上，我们需要一个non-member函数(条款24);**为了令这个函数被自动具现化，我们需要将它声明在class内部**；而在class内部声明non-member函数的唯一办法就是：令它成为一个friend

条款47:请使用traits classes表现类型信息
1. STL共有5种迭代器分类，对应于它们支持的操作：
   1. Input迭代器只能向前移动，一次一步，客户只可读取(不能涂写)它们所指的东西，而且只能读取一次。它们模仿指向输入文件的阅读指针(rad pointer);C++程序库中的istream_sterators是这一分类的代表。
   2. Output迭代器情况类似，但一切只为输出：它们只向前移动，一次一步，客户只可涂写它们所指的东西，而且只能涂写一次。它们模仿指向输出文件的涂写指针(write pointer);ostream_iterators是这一分类的代表。
   **这两类都只能向前移动，而且只能读或写其所指物最多一次，所以它们只适合“一次性操作算法”**
   3. forward可以做前述两种分类所能做的每一件事，而且可以读或写其所指物一次以上，这使得它们可施行于多次性操作算法
   4. Bidirectional迭代器比上一个分类威力更大；它除了可以向前移动，还可以向后移动。STL的list迭代器就属于这一分类，set,multiset,map和mutimap的迭代器也都是这一分类。
   5. random access迭代器比上一个分类威力更大, 它可以执行“迭代器算术”,也就是它可以在常量时间内向前或向后跳跃任意距离。vector,deque和string提供的迭代器都是这一分类。
2. iterator_traits以两个部分实现上述所言。首先它要求每一个“用户自定义的迭代器类型”必须嵌套一个typedef,名为iterator_category,用来确认适当的卷标结构(tag struct)。
```c++
template <...> //略而未写tempkatc参数
class deque {
public:
   class iterator {
   public:
   typedef random_access_iterator_tag iterator_category;
   };
   ...
};
```
至于iterator _traits,只是鹦鹉学舌般地响应itenator class的嵌套式typedef:
```c++
template<typename IterT>
struct iterator_traits{
   typedef typename IterT::iterator_category iterator_category;
   ...
};
```
为了支持指针迭代器， 1terator_traits特别针对指针类型提供一个偏特化版本(partial template specialization)
```c++
template<typename IterT>
struct Iterator_traits<IterT*> //针对内置指针
{
   typedef random_access_iterator_tag iterator_category;
   ...
};
```
我们可以对advance实践先前的伪码(pseudocode);
```c++
template<typename IterT,typename DlatT>
void advance(IterTe iter, DistT d)
{
if(typeid(typename std::iterator_traits<IterT>::iterator_category)
=typeid(std::random_access_iterator_tag))
```
TODO: 为什么if语句在运行期核定
此刻有更根本的问题要考虑。IterT类型在编译期间获知，所以iterator_trait<IterT>::iterator_category也可在编译期间确定。**但if语句却是在运行期才会核定**。我们真正想要的是一个条件式(也就是一个if…else语句)判断“编译期核定成功”之类型。恰巧C++有一个取得这种行为的办法，那就是重载(overloading)。
为了让advance的行为如我们所期望，我们需要做的是产生两版重载函数，内含advance的本质内容，但各自接受不同类型的Iterator_category对象。我将这两个函数取名为doAdvance:
```c++
template<typename IterT, typename DistT> //这份实现用于
void doAdvance(IterT& iter,DistT d,      //random access
   std::random_access_iterator_tag)       //迭代器
{  
      iter += d;
}
template<typename IterT, typename DistT>//这份实现用于 
vold doAdvance(IterTs iter,DistT d,    //bidirectional
std::bidirectional_iterator_cag)       //迭代器
{
   if(d >= 0) {while(d--) ++iter;}
   else{while(d++) --iter;}
}     
```
有了这些doAdvance重载版本，advance需要做的只是调用它们并额外传递一个对象，后者必须带有适当的迭代器分类。于是编译器运用重载解析机制
(overloading resolution)调用适当的实现代码：
```c++
template<typename IterT,   typename DistT>
void advance(IterT& iter,  DistT d)
{
   doAdvance(iter,d, std::iterator_traits<IterT>::iterator_category());
}
```
总结：
   1. 建立一组重载函数(身份像劳工)或函数模板(例如doAdvance),彼此间的差异只在于各自的traits参教。令每个函数实现码与其接受之raits信息相应和。
   2. 建立一个控制函数(身份像工头)或函数模板(例如advance),它调用上述那些“劳工函数”并传递traits class所提供的信息。

条款48:认识template元编程
1. (1) 类型测试发生于运行期而非编译期，(2)“运行期类型测试”代码会出现在(或说被连接于)可执行文件中
2. 编译器必须确保所有源码都有效，纵使是不会执行起来的代码! 而当iter不是random access迭代器时“iter +=d”无效