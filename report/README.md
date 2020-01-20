# Project Assignment 4 - Semantic Definitions

### **學生 : 陳昱螢    學號 : 0616091**

## 作業內容：
* Do some simple checking of semantic correctness.

## 流程：
* 先 visit AST 來建立 symbol table。
* 接著visit 各個 child node。
* 當visit 到底的時候，開始做semantic，並將結果輸出。

## 作法：
* 我是先寫了一個 visitor 來遍歷整個 AST ，建立相對應的 symbol table。因為一開始助教沒講清楚說 error 是用 stderr 來做輸出，因此為了要讓symbol table能先輸出，所以就寫了個 visitor 來遍歷整個 AST 建立所需的 symbol table。
* 接著才撰寫 semantic 的 visitor 再一次遍歷整個 AST 將錯誤的資訊輸出出來。

## 遇到的問題：

* 一開始為了要分開想了蠻久的，後來才想到可以先寫一個visitor來處理symbol table的部分，並處理重複宣告的問題，但先不輸出錯誤。
* 當建立好之後，再用semantic來檢查有沒有重複宣告，若在symbol table找得到的話，表示有宣告，並記錄找到一次，當找到第二次的時候，便表示說重複宣告了，若整個table都找不到的話，就表示說沒有宣告。
* 一開始以為compound裡面看不到外面的compound的table，又花了很多時間再處理要搜尋哪些table，結果後來問了助教才發現說，是看得到的，就讓這個問題變不見了，之前寫好的解法也有點白寫的感覺了QQ
* 在處理 memory leak時，以為可以重複 delete memory，結果就噴了一堆錯誤的說@@，每次 delete完都有指向 nullptr的說，結果還是會有問題，只好重新檢查destructor，確定一次delete 只會對應到一個new的memory，來讓程式不會出現錯誤訊息。
 
## 心得：
* 我覺得這次的 spec 沒有很清楚，一開始常常不知道怎樣的情況要繼續怎樣的情況不用繼續，每天看 issue 就會發現又有東西要改了QQ
* 實作上其實也有點難度，跟上次一樣，一開始完全沒有頭緒，也是想了兩三天才想到大概的作法，接著才開始撰寫。一開始為了讓symbol table跟semantic的輸出順序正確，想了超級久的，結果後來助教說分別是用stdout 跟 stderr 來輸出，這樣感覺當初就不用花時間想那麼久了，還多寫了一個 visitor 來處理 symbol table 的問題QQ
* 寫了一個星期多的說，感覺好累QQ
* 希望下個作業不要這麼難了
* 希望隱藏測資可以簡單點

## HW4 [BOUNS]：

* Multi-pass :
    * 為了能達到 multi-pass 想了一下，後來發現其實不會很麻煩，只需要將建好的 symbol table 的資訊儲存在該 Node 上就可以了，像這次的作業，會有 symbol table 的 Node 只有 ```Program```, ```Compound```和 ```For``` 這三個 Node ，因此只需要將所建立的 table 儲存到這三個 Node 上面即可，因此我是先使用 visitor 建立 table ，並儲存到 Node 上面，接著再做 semantic 的時候，再從節點讀取需要的資訊即可。
    
* No memory leak :
    * 我應該是都有寫好 destructor 了，testcase 都能正常通過，希望隱藏測資不要出問題QQ