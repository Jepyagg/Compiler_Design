# Project Assignment 2 - Syntactic Definitions

### **學生 : 陳昱螢    學號 : 0616091**

## 作業內容：
* Create an LALR(1) grammar by using yacc

## 流程：
* 先在```scanner```加上要return的token，這樣才能在parser裡面使用。

* 開始實作 ```parser``` ，先將先前在```scanner```所return的token 宣告在上面，便開始寫```Syntactic Definitions```。

* ```Syntactic Definitions```分別有：
    * Program Units
        * Program
            * global variables
            * local variables
        * Function
    * Data Types and Declarations 
        * Variable
        * Constant
    * Statements
        * compound
        * simple
        * expressions
        * array reference
        * function invocation
        * conditional
        * while
        * for
        * return
        * function invocation
* 接著便按照順序開始一個一個新增rule進去。

## 作法：

* 基本上就按照上面的順序開始實作，一開始先將```program```的部分先寫出來，然後開始思考當有無statements會是什麼情況，是否要留epsilon等等之類的。

* 接著因為```program```內部第一個是```variables```，所以就先撰寫```variables```的rule。因為```variables```可能會沒有，因此會選擇```epsilon```或是進入```variable_decl```。而在```variable_decl```裡面則是根據變數數量來決定要生成幾個```variable```，而```variable```則對應的多種不同的格式。

* 再來撰寫 ```functions```，基本上與```variables```的格式算是非常相近，只有在最後的格式上面有做修正。同時也撰寫需要用到的```arguments```以及```identifier_list```。

* ```arguments```和```identifier_list```的結構也都非常相似，基本上就是先判斷是否為```epsilon```，若不是的話則進到第二層，第二層就是負責一個或是多個型態的判斷。

* 接著開始撰寫 ```statements``` 的部分，```statements```分成比較多個小組合，便依序撰寫每個小組合的部分。

* 首先是 ```compound```，基本上就是直接進到```variables```和```statements```裡面就是了。

* 再來是 ```simple```，將```simple```可能發生的組合依序列出來即可。

* 最後則是算是重要的```expressions```，因為```expressions```的組合比較多，且有優先順序的問題，因此要想好絲路才行。首先因為```-```可以當作```binary subtraction operator```或是```unary negation operator```，所以當遇到```unary negation operator```時，要將他的優先度提高，這樣才不會出錯，而且其他的```operator```，則在一開始先用```%left```設定好優先順序，避免之後會出錯，最後則按照要求將rule列出來即可，並且把會用到的```array reference```和```function invocation```完成。

* ```statements``` 的部分完成後，便開始撰寫```conditional```的部分。```conditional```的情況種類則比較多，像是有無else等等的，若有的話則有不同的對應方式，所以要先區隔出else的情況來，接著在按照```statements```數量來進行下去。

* 最後則是將```while```、```for```和```return```完成，這三個基本上照著規則寫下去即可。

## 遇到的問題：

* 在一開始將```scanner```的return 寫好，並且在```parser```上加好token後去```make```，結果就噴了一堆```error```。
    * ![](https://i.imgur.com/QuFr0Ty.png)

* 研究了一陣子才發現是因為沒有```yy.tab.h```的關係，助教給的```Makefile```並沒有編譯出來，需要自己先編譯好或是修改```Makefile```才行。之後又花了不少時間研究了一下```Makefile```，後來發現只需要在```parser```的```$(YACC) -o $@ -v $<```裡面加上```-d```即可，並將編譯的順序做調換，先讓```parser```編譯再編譯```scanner```，讓```scanner```可以找到include的.h檔案。

* 在寫```parser```的時候也遇到不少問題就是了，但都不是什麼大問題，比較多都是文法寫錯，或是不小心寫成```left recursion```之類的，真正大的bug倒是沒幾個就是了。    

## 心得：

* 這次的作業除了一開始```Makefile```的問題以外，其實都還好，跟第一次差不多，只是會需要時間上手就是了。助教的```README```寫得很好，所以在撰寫```parser```時輕鬆了不少，基本上只要照著```README```的敘述去刻，大致上都不會出錯，算是省下了不少的時間。雖然給了一堆測資來測試，不過還是蠻擔心自己寫的```parser```會有一堆漏洞就是了，希望隱藏的測資不要太難就是了。