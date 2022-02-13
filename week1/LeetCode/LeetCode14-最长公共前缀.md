# LeetCode14-最长公共前缀

## 题目描述

编写一个函数来查找字符串数组中的最长公共前缀。

如果不存在公共前缀，返回空字符串 ""。

 

示例 1：

输入：strs = ["flower","flow","flight"]
输出："fl"
示例 2：

输入：strs = ["dog","racecar","car"]
输出：""
解释：输入不存在公共前缀。

## 题解

### 方法一：横向比较：字符串两两比较，依次选取两个字符串的公共前缀

1、方法描述

（1）基本思想：依次比较前面字符串的公共前缀和后面的字符串，最终得到最长公共前缀

（2）公共前缀获取：通过下标标识公共前缀的位置，对应位置的字符相同时，公共前缀的右边界右移。最后，根据公共前缀的左右边界的下标截取相应的公共子串。

公共前缀的特点：

​        ①必须从字符串中第一个字符开始（即从任意一个字符串）

​        ②公共前缀的长度必须小于等于字符串中最短字符串的长度，即     公共前缀的长度小于等于最短字符串的长度

（3）多个字符串提取公共前缀的方法：依次比较每个字符串与前面得到的公共前缀，直到与最后一个字符串比较完毕，得到的前缀即为所有字符串的最长公共前缀。

初始条件：第一个字符串最为初始的前缀

公共前缀的更新：由公共前缀和当前遍历到的字符串的公共前缀直接得到

（4）判断条件：在两个获取公共前缀的字符串中最短字符串的长度范围内，对应位置的字符相同

（5）边界条件：

①字符串数组为空时，公共前缀为空字符串

②当寻找过程中如果得到的公共前缀为空字符串，则最长公共前缀一定为空字符串

2、具体实现

```
class Solution {
public:
    string CommonPrefix(const string& str1,
                        const string& str2) {
      //寻找两个字符串之间公共前缀
      int length=min(str1.size(),str2.size());//记录两个字符串的最小长度
      int right=0;//记录公共前缀右边界的位置
      while (right<length &&
             str1[right]==str2[right]) {
      /*公共前缀右边界更新的条件：
      ①右边界的位置小于两个字符串中较短字符串的长度
      ②对应位置的字符相同
        ++right;
      }
      return str1.substr(0,right);//利用C++中的截取字符串的函数substr()获取两个字符串的公共前缀
    }
    
    string longestCommonPrefix(vector<string>& strs) 
    {
    //寻找一组字符串的最长公共前缀
        if (strs.size()==0) {
          return " ";//字符串数组为空时，直接返回空字符串
        }
        string Perfix=strs[0];//初始化公共前缀
        for(int i=1;i<strs.size();++i) {
        //依次遍历字符串数组中的每个字符串
          Perfix=CommonPrefix(Perfix,strs[i]);//当前的公共前缀与当前字符串的公共前缀
          if (!Perfix.size()) {
          //如果得到的公共前缀时空字符串，则直接跳出循环
            break;
          }
        }
        return Perfix;//返回最后得到的最长公共前缀
    }
};
```

3、运行结果

![image-20220210191548497](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\image-20220210191548497.png)

### 方法二：纵向比较：字符串同时比较，依次选取最长公共前缀的每个字符

1、方法描述

（1）基本思想：同时比较每个字符串中相同位置上的字符，寻找所有字符串的公共前缀

（2）所有字符串同时比较：依次将后面的字符串与第一个字符串中相同下标的字符比较，直到字符不相同，或者到达某一个字符串的末尾。

（3）边界情况：当字符串数组中没有字符串时，最长公共前缀为空字符串。

（4）最长公共前缀的确定：

​         ①默认的最长公共前缀为第一个字符串

​         ②当字符不相等或者达到最短字符串的末尾时，比较结束，最长公共前缀为任意一个字符串中第一个字符到当前比较位置字符的子串。

2、具体实现

```
class Solution {
public:
    string longestCommonPrefix(vector<string>& strs) 
    {
       if (!strs.size()) {
         return " ";//边界情况，返回空字符串
       }
       int length=strs[0].size();//记录第一个字符的长度
       int n=strs.size();//记录字符串的数量
       for (int i=0;i<length;++i) {
         char c=strs[0][i];//记录当前比较的字符
         for (int j=1;j<n;++j) {
         //对每个字符串中相同位置上的字符进行比较判断
           if (i==strs[j].size() || 
              strs[j][i]!=c) {
            //如果比较到达某一字符串的末尾或者字符串中的当前字符与第一个字符串中相同位置的字符不相同
                return strs[j].substr(0,i);//返回从下标为0处到当前下标处截取的字符串
              }
         }
       }
       return strs[0];//默认情况下，返回字符串数组中的第一个字符串
    }
};
```

3、运行结果

![image-20220210195631609](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\image-20220210195631609.png)

### 方法三：分治法划分数组

1、方法描述

（1）基本思想：将数组划分，减少对每个字符串的比较

（2）分治策略：

分：从中间划分字符串数组

治：对左右两边的字符串数组依次求解最长公共前缀，然后求解左右公共前缀的公共前缀

（3）最长公共前缀：由划分后的左右两边得到的最长公共前缀依次比较每个位置上的字符，得到最后的最长公共前缀。

默认的最长公共前缀：左右前缀长度较短的那个

最长公共前缀的更新：当左右最长公共前缀相应位置的字符不相同时，则从左右公共前缀的第一个字符到当前下标指示的字符为两部分前缀的最长公共前缀。

2、具体实现

```
class Solution {
public:
    string CommonPerfix (const string& LCPleft,
                        const string& LCPright) {
      //寻找得到的左右公共前缀的公共前缀
      int minlength=min(LCPleft.size(),LCPright.size());//公共前缀的长度不超过左右前缀的最短长度
      for (int i=0;i<minlength;++i) {
      //遍历从第一个字符到长度较短的最长公共前缀的长度的每个字符
        if (LCPleft[i]!=LCPright[i]) {
        //当对应位置的字符不相等时，返回从第一个位置到当前位置构成的子串
          return LCPleft.substr(0,i);
        }
      }
      return LCPleft.substr(0,minlength);//默认情况下返回左右最长公共前缀中较短的那一个
    }
    
    string DivideCommonPerfix(vector<string>& strs,int start,int end) {
    //划分字符串数组，并求解划分后的左右最长公共前缀和最后的最长公共前缀
      if (start==end) {
          return strs[start];//数组中只有一个字符串时，则发挥这一个字符串最为最长公共前缀
      }
      int mid=(start+end)/2;//划分位置
      string LCPleft=DivideCommonPerfix(strs,start,mid);//递归求解左边部分的最长公共前缀
      string LCPright=DivideCommonPerfix(strs,mid+1,end);//递归求解右边部分的最长公共前缀
      return CommonPerfix(LCPleft,LCPright);//求解两部分最长公共前缀所在的字符串的最长公共前缀
    }
    string longestCommonPrefix(vector<string>& strs) {
    //求解字符串数组的最长公共前缀
       if (!strs.size()) {
         return " ";//字符串数组为空时，最长公共前缀为空字符串
       }
       return DivideCommonPerfix(strs,0,strs.size()-1);//字符串数组不为空时，划分数组
    }
};
```

3、运行结果

![image-20220210212440990](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\image-20220210212440990.png)

