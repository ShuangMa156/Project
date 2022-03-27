# LeetCode125-验证回文串

## 题目描述

给定一个字符串，验证它是否是回文串，只考虑字母和数字字符，可以忽略字母的大小写。

说明：本题中，我们将空字符串定义为有效的回文串。

示例 1:

输入: "A man, a plan, a canal: Panama"
输出: true
解释："amanaplanacanalpanama" 是回文串
示例 2:

输入: "race a car"
输出: false
解释："raceacar" 不是回文串


提示：

1 <= s.length <= 2 * 10^5
字符串 s 由 ASCII 字符组成

## 题解

![image-20220315215906621](C:\Users\DELL\AppData\Roaming\Typora\typora-user-images\image-20220315215906621.png)

### 方法一：筛选+双指针

1、方法描述：

（1）基本思想：将原字符串转化为满足要求的规格化字符串，然后利用双指针判断字符串是否是回文串。

（2）只考虑数字和字母，忽略字母的大小写：对字符串中每一个字符进行判断，将符合规则的字符按序存到另一个字符串中，完成字符串的过滤和字母大写转小写操作。

①字符串筛选：

 ⅰ符合条件的字符（筛选条件）：字母或数字

 ⅱ 如何筛选：判断字符是否属于大写字母（A~Z）、小写字母(a~z)、数组的范围区间(0~9)    

​        普通筛选：直接对字符的值进行比较

​             大写字母：(s[i] >= 'A') && (s[i] <= 'Z')

​             小写字母：(s[i] >= 'a') && (s[i] <= 'z')

​               数字：(s[i] >= '0') && (s[i] <= '9')

​       调用库函数筛选：cctype库中的isalnum()函数

​            功能：判断一个字符是否是字母或者数字（十进制），若为字母或者数字，则返回True(非0值)，否者返回False(0)

​            函数原型：int isalnum ( int c )

​           参数：c为要检测的字符。它可以是一个有效的字符（被转换为 int 类型），也可以是 EOF（表示无效的字符）。

​           头文件： #include<cctype>

②大写字母转小写字母：当判断字符是大写字母时，完成大写字母转换成小写字母的操作

ⅰ普通方法：先计算该字符与大写字母A的偏移值，再加上小写字母a的ACSII码值，即可得到该大写字母对应小写字母的ASCII码值，即(s[i]-'A')+'a'

ⅱ调用库函数tower()函数：

功能：把给定的字母转换为小写字母。

头文件：#include<cctype>

函数原型：int tolower(int c);

参数：c为要转化的字符，若参数 c 为大写字母则将该对应的小写字母返回。

返回值：返回转换后的小写字母，若不须转换则将参数c 值返回。

③存储结果到另一个字符串：当判断字符符合条件时，将该字符拼接到另一个字符串中

（3）验证回文串：对筛选后得到的新字符串进行判断，利用双指针，对整个字符串中相对于字符串开头和字符串结尾相同距离的字符进行比较判断。

①如果左右指针位置上的字符相等，则移动指针，继续比较，直到左右指针指向同一个位置的字符，跳出循环，返回true

②如果左右指针位置上的字符不相等，则直接返回false

2、具体实现：

```
class Solution {
public:
    bool isPalindrome(string s) {
        string news;
        for (int i=0;i<s.length();++i) { //对字符串中的字符进行筛选
            if((s[i] >= 'A') && (s[i] <= 'Z')) { //该字符属于大写字母
                news+=(s[i]-'A')+'a'; //将大写字母转化为小写字母，并拼接到新字符串末尾
            }
            else if ((s[i] >= 'a') && (s[i] <= 'z')) { //该字符属于小写字母
                news+=s[i]; //将该小写字母拼接到新字符串末尾
            }
            else if ((s[i] >= '0') && (s[i] <= '9')) { //该字符属于数字
                news+=s[i] ; //将该数字拼接到新字符串末尾
            }
        }
        int i=0; //初始化左指针的位置 
        int j=news.length()-1; //初始化右指针的位置
        while (i<=j) { //当左右指针未交错时，对字符串进行回文判断 
            if (news[i]==news[j]) { //左右指针指向的字符相等，则移动指针
                i++; //左指针右移
                j--; //右指针左移
            } else { //左右指针指向的字符不相等，则返回false
                return false;
            }
        }
        return true;
    }
};
```

```
//优化版本，调用C++的库函数进行筛选
class Solution {
public:
    bool isPalindrome(string s) {
        string str;
        for (char ch: s) {
            if (isalnum(ch)) { //依次判断字符中的每个字符是否是字母或数字
                str += tolower(ch); //如果是字母或数字，则拼接到新字符末尾
            }
        }
        int left = 0; 
        int right = str.size()-1;
        while (left < right) {
           if (str[left] != str[right]) { //左右指针指向的字符不相等，则返回false
                return false;
            }
            //左右指针指向的字符相等，则移动指针
            ++left; //左指针右移
            --right; //右指针左移
        }
        return true;
    }
};

```

3、运行结果：

![image-20220315221738217](C:\Users\DELL\AppData\Roaming\Typora\typora-user-images\image-20220315221738217.png)

![image-20220315222711404](C:\Users\DELL\AppData\Roaming\Typora\typora-user-images\image-20220315222711404.png)

### 方法二：直接调用c++中的翻转函数reverse()

1、方法描述

（1）基本思想：先将原字符串中符合条件的字符筛选出来。然后将原字符串反转，与原字符串比较

（2）字符串筛选：调用库函数isalnum()判断是否是字母或数字,tower()函数将大写字母转化为小写字母

（3）回文串判断：先反转筛选后的字符串，在比较其与原字符串是否相等

反转字符串：用函数reverser()实现字符串的反转

​      ①功能：用于反转在[first,last)范围内的顺序（包括first指向的元素，不包括last指向的元素），reverse函数**没有返回值**，可以对**数组、字符串、vector容器中**的元素进行翻转操作。

​      ②头文件：#include <algorithm>

​      ③函数原型：void reverse (BidirectionalIterator first,BidirectionalIterator last);

​     ④参数说明：first为开始位置，end为结束位置

​     ⑤返回值：reverse函数**没有返回值**

回文串判断：将原字符串与反转字符串比较的结果返回

   相等：是回文串---返回true

   不相等：不是回文串----返回false

2、具体实现

```
class Solution {
public:
    bool isPalindrome(string s) {
        string str;
        for (char ch: s) { //对原字符串中的字符进行筛选
            if (isalnum(ch)) {
                str += tolower(ch);
            }
        }
        string str_rev=str;
        reverse(str_rev.rbegin(), str_rev.rend()); //反转字符串
        return str==str_rev; //两个字符串的比较
    }
};

```

3、运行结果

![image-20220315222532572](C:\Users\DELL\AppData\Roaming\Typora\typora-user-images\image-20220315222532572.png)

