# Project Assignment 5 - Code Generation

### **學生 : 陳昱螢    學號 : 0616091**

## 作業內容：
* generate RISC-V instructions for the P language.

## 流程：
* visit AST tree，並在每個 node 輸出對應的組合語言。

## 作法：
* 我是邊visit AST 邊使用symbol Table來處理宣告的部分，global variable就直接用symbol Table的資訊來生成，而沒有去visit declaration node，這樣才能正確區分constant value 跟 variable就是了。
* 接著就是visit 其他的節點然後生成對應的組語。

## 遇到的問題：
* 組語常常寫成錯誤的語法，結果在跑 .s 的檔案時，就會出錯，就只能自己一個一個語法找，最常發生在使用lw，像是寫成```lw t0, 0(t1)```，但是t1是數值而不是address，結果就會出錯了。
 
## 心得：
* 這次的作業相較之前兩次是感覺比較簡單一點，但是還是有點難度。如果對於組合語言蠻熟悉的話，應該就可以像助教講的一樣很快就寫完，但如果像我一樣，對於組合語言並不是那麼熟悉，就會寫的有點久，因為常常要思考接著的組語該是什麼，或是該用怎樣的組語才行，蠻多時間都是在處理這些問題的，因為用錯組語，所以就輸出錯的東西了QQ
* 這次的自由度感覺蠻大的，因為只要能跑出正確的結果就好了，組語並不用一樣，所以可以根據自己的習慣來撰寫，但前提是對組語熟悉的情況QQ
* 邊趕微處理機的Final Project跟邊寫Compiler的作業真的累。
* 希望隱藏測資可以簡單點