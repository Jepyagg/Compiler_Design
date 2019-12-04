# Project Assignment 3 - Constructing Abstract Syntax Trees

### **學生 : 陳昱螢    學號 : 0616091**

## 作業內容：
* Extend parser to construct an abstract syntax tree (AST) for a given program written in P language and to serialize this AST into the designated format.

## 流程：
* 先建立 AST 會需要用到的 Node，再撰寫parser的action，接著嘗試visit AST的 node，將結果給印出來。
* AST 會需要的 Node:
    * AstNode 為以下 Node 的 parent
    * AstProgram
    * Program_body
    * Declaration_Node
    * Function_Node
    * Expression_Node 為以下 Node 的 parent
        * Const_Node
        * Binary_Operator_Node
        * Unary_Operator_Node
        * Variable_Reference_Node
        * Function_Call_expr_Node
    * Statement_Node 為以下 Node 的 parent
        * Compound_Node
        * Assignment_Node
        * Print_Node
        * Read_Node
        * If_Node
        * While_Node
        * For_Node
        * Return_Node
        * Function_Call_Node
    * Id_Node
    * Array_Node
    * Formal_Node
* 接著就根據需求，依序建造Node出來。
* 再來就是撰寫Vistor以及visit的動作。

## 作法：
* 一開始先依照tutorial裡面的example code 將大致的框架給寫出來，分別為```ast.h```, ```ast.cpp```, ```visitor.h```以及```visitor.cpp```。

* 原先沒什麼頭緒就是了，不太清楚要先建立怎樣的Node，之後我是按照測資來建立的，這樣建立好之後，要測試也比較方便，能知道自己是否有寫對。

* 一開始是最簡單的 Constvalue，分別是將```AstNode```,```AstProgram```, ```Program_body```,```Declaration_Node```, ```Compound_Node```和```Id_Node```建立出來。

* 建立好這些Node後，便在parser中，定義這些class的type，讓non-terminal可以知道回傳的type是什麼。

* 然後開始撰寫visior在visit到這些class的時候，分別該有怎樣的動作，印出正確的資訊。

* 當成功完成第一筆測資後，後續基本上就比較上手了，開始根據測資的要求建立對應的node以及完成在parser的action，讓class能順利被建立起來，並將指標回傳給上面的Node，藉此來將整棵樹給建立起來。

* 而我建立的順序是依照測資來建立的，測資的順序分別是```ConstantValue```,```variable```,```program```,```function```,```declaration```,```expression```,```simple```,```invocation```和```control```這樣。

* 而Node則大概是```AstNode```,```AstProgram```, ```Program_body```,```Declaration_Node```, ```Compound_Node```, ```Id_Node```,```Function_Node```,```Array_Node```,```Formal_Node```，再來則是```Expression```以及他的child node，最後才是```Statement```和他的child node。

* 而visitor的visit，我基本上是每建立一個新的Node後，就去將他visit該有的動作一起完成，這樣才不會有忘記visit的情況發生。

* 基本上寫到後來，動作就都差不多了，只差在Node裡面要儲存哪些資料，以及visit時，該有怎樣的動作。

* 當所有測資完成後，我才開始撰寫每個class的destructor就是了，避免memory leak的發生。

## 遇到的問題：

* 遇到的問題其實蠻多就是了...

* 一開始助教給的Makefile好像沒辦法compile到我寫的cpp，因此花了不少時間去看到底需要compile哪些資訊，然後在自己重寫了一個比較簡單的Makefile來compile我所需要的資訊。

* 研究 non-terminal 的 type 以及 class node 也花了不少時間在理解該給parser怎樣的資訊，然後要怎樣呼叫就是了。

* 雖然好像README裡面就有寫要怎樣算line_num以及col_num，不過也是到後來研究過才知道可以用```yylloc.first_line```,``` yylloc.first_column```來取得line_num以及col_num，若要在token是在中間的話則可以用```@1.first_line```, ```@1.first_column```，這個問題比較算是自己沒看懂就是了QQ

* 沒有增加```extern "C" int yylex(); extern "C" int yyparse();``` 這兩行時，會出現下圖的錯誤：
    * ![](https://i.imgur.com/6n4q9N7.png)

* 原先是按照tutorial裡面的example code來撰寫，在class中宣告```print()```，並在```ast.cpp```裡面將```print()```完成，然後再visit的時候呼叫來印出資訊，但有時無法將visitor給傳遞下去，因此最後就放棄這種寫法，直接將```print()```給移除了，將原本該由```print()```所進行的動作，改為直接在visit的時候就進行了，不要再去call```print()```來進行動作，這樣visitor也能順利的傳遞下去。

* 在寫```Expression```裡面的```function call node```時，有稍微煩惱，因為已經先讓```function call node```繼承```Statement Node```了，不太確定說，又讓他繼承```Expression Node```是否可行，所以後來就直接在parser中新增一個non-terminal叫做```function call expr```來給```Expression```使用，同時也建立一個```function call expr Node```繼承```Expression Node```，算是比較爛的解法就是了。

* 一開始編譯的時候都沒有用```-Wall```來輸出warning，還以為自己的程式沒什麼問題，沒想到一開下去，就飛天了，不過仔細研究後才發現其實都是同樣的問題。
    * 出現的warning 都是下圖這類型的：
        * ![](https://i.imgur.com/sQy2kRW.png)
    * 查了之後才知道說，是因為constructor中變數初始化的順序要跟聲明的變數順序一樣才行，不然就會出現警告。
    * 將聲明變數的順序修改過後，這些 Warning 就消了。

* 開始寫destructor，並使用```valgrind```測試時，也遇到一些小麻煩就是了，像是class 宣告的順序問題，明明有先```forward declartion```了，但還是出現了warning，只好將class的順序重新排過，不太確定是不是我搞錯用意了。

* 然後因為在編譯的時候沒有加上```-g```的參數，因此在使用```valgrind```時，沒有出現出錯的行數。（```-g```：編入除錯資訊(要使用GDB除錯一定要加)）。
    * 沒加上參數時：
        * ![](https://i.imgur.com/dWJdWCS.png)
    * 加上參數時：
        * 就有顯示出來是在哪個檔案的哪一個位置。
        * ![](https://i.imgur.com/sELv7HJ.png)

* 還有vector指標沒初始化，若沒有用到的話，在destructor中使用for(auto)迴圈去delete的時候就噴錯了。
    * ![](https://i.imgur.com/4M293WW.png)

* 因此在delete前先檢查是否NULL，但因為沒初始化，結果換成：
    * ```Conditional jump or move depends on uninitialised value(s)```
    * 因此都會先初始化成NULL，不過後來發現好像是跟我前面寫的constructor初始化的順序錯誤有關。
 
## 心得：
* 這次的作業感覺難度真的增加很多，也花費了非常多的時間在寫，從上個星期二開始看README以及tutorial，然後星期三開始搞懂說到底是要幹嘛，星期四瞭解class大概是要怎麼建立，星期五開始會建立Node，然後晚上開始狂寫到半夜完成一半的作業，星期日寫到半夜把整個作業完成。這個星期的星期二把memory leak清除掉，星期三寫完Report。整個作業算是花費了一個星期吧，很久沒寫過需要花這麼多時間的作業了...
    * 中間還夾雜MCSL的Lab跟期末專題的proposal，同時還有心理學的報告，讓我整週很充實的渡過了就是了...

* 這份作業我認為最難的地方就是一開始了，後面其實就還好了，因為都在做差不多的事情，像是在```parser```補個action，在```ast.h```裡面新增class，在```visitor.h```跟```visitor.cpp```補上動作跟宣告...等等，基本上就是差不多的動作啦
* HW3 spec：有什麼地方可以改進：
    * 我覺得最難的是理解說該如何從頭開始進行，因為一開始是什麼東西都沒有，因此光看README其實有點難知道要從哪邊下手，我是覺得可以像助教後來寫的code那樣，放一個簡單一點的code在裡面，這樣要開始下手其實可以比較簡單一些，可以知道說大概跑出來的樣子會是怎樣。
    
    * 看完助教給的README後，其實會讓人想用Visitor Pattern來寫這次的作業。一開始提供的Makefile以及資料夾那些卻都是C code，雖然說是可以用c code來寫Visitor Pattern，但是因為上面也都寫說比較推薦用C++來寫，因此得花不少時間來更改Makefile就是了(結果後來助教有提供就是了...QQ)。
    
    * 測資沒測到 expression 的 function call 就是了，因為那時候沒寫action，結果還是可以通過全部的測資XD
