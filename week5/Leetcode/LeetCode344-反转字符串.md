# LeetCode344-反转字符串

## 题目描述

编写一个函数，其作用是将输入的字符串反转过来。输入字符串以字符数组 s 的形式给出。

不要给另外的数组分配额外的空间，你必须原地修改输入数组、使用 O(1) 的额外空间解决这一问题。



示例 1：

输入：s = ["h","e","l","l","o"]
输出：["o","l","l","e","h"]
示例 2：

输入：s = ["H","a","n","n","a","h"]
输出：["h","a","n","n","a","H"]


提示：

1 <= s.length <= 105
s[i] 都是 ASCII 码表中的可打印字符

## 题解

### 方法一：对撞指针，相互交换位置

1、方法描述

（1）基本思想：双指针分别标记要交换的两个符号的位置，通过指针的移动和和交换操作实现字符串的反转

（2）字符串反转：通过数组下标，是相对于字符串开始的第一个字符和最后一个字符的偏移位置相同的符号相互交换，直到所有可以交换的数组元素全部交换完毕。

双指针：

​      ①左指针left----标记左边要交换的元素在数组中的位置，初始化为0(字符串数组中第一个元素的位置)

​     ② 右指针right----标记右边要交换的元素在数组中的位置，初始化    s.size()-1（字符串数组中最后一个元素的位置）

指针移动：

​       ①当left<right，表示字符串数组中的元素还未全部完成交换

​          左指针右移：left++

​          右指针左移：right--

​       ②当left>=right，表示字符串数组中的元素已全部完成交换

​           反转字符串结束，返回

（3）如何不给数组分配额外的空间：在原数组中交换两个元素所在的位置

​       操作：找到要交换的两个元素s[left]和s[right],调用swap()函数完成两个元素的交换



![image-20220327212213942](C:\Users\DELL\AppData\Roaming\Typora\typora-user-images\image-20220327212213942.png)

2、具体实现

```
class Solution {
public:
    void reverseString(vector<char>& s) {
    int left=0;
    int right=s.size()-1;
    while (left < right) {
        swap(s[left++],s[right--]);
    }
    }
};
//时间复杂度：O(N)，其中N为字符数组的长度，一共执行了N/2次的交换。
//空间复杂度：O(1)，只使用了常数空间来存放若干变量。
```

```
//优化版本，减少交换次数
class Solution {
public:
    void reverseString(vector<char>& s) {
    int left=0;
    int right=s.size()-1;
    while (left < right) {
        if (s[left]!=s[right]) { //增加判断，减少交换次数
            swap(s[left],s[right]);
        } 
        left++;
        right--;
    }
    }
};
```

```
//使用异运算交换
class Solution {
public:
    void reverseString(vector<char>& s) {
        int lpos = 0;
        int rpos = s.size()-1;
        while (lpos < rpos){
            s[lpos] ^= s[rpos];
            s[rpos] ^= s[lpos];
            s[lpos] ^= s[rpos];
            lpos++;
            rpos--;
        }
    }
};
```

```
//使用加减法运算实现交换
class Solution {
public:
    void reverseString(vector<char>& s) {

        int l = s.size();
        int left = 0;
        int right = l - 1;
        int a, b, c;
        while (left < right)
        {
            s[right] = s[left] + s[right];
            s[left] = s[right] - s[left];
            s[right] = s[right] - s[left];
            left++;
            right--;
        }

    }
};
```

3、运行结果

![image-20220315215223235](C:\Users\DELL\AppData\Roaming\Typora\typora-user-images\image-20220315215223235.png)





