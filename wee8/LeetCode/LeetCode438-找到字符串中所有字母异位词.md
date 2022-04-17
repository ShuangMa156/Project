# LeetCode438-找到字符串中所有字母异位词

## 题目描述

给定两个字符串 s 和 p，找到 s 中所有 p 的 异位词 的子串，返回这些子串的起始索引。不考虑答案输出的顺序。

异位词 指由相同字母重排列形成的字符串（包括相同的字符串）。

示例 1:

输入: s = "cbaebabacd", p = "abc"
输出: [0,6]
解释:
起始索引等于 0 的子串是 "cba", 它是 "abc" 的异位词。
起始索引等于 6 的子串是 "bac", 它是 "abc" 的异位词。
 示例 2:

输入: s = "abab", p = "ab"
输出: [0,1,2]
解释:
起始索引等于 0 的子串是 "ab", 它是 "ab" 的异位词。
起始索引等于 1 的子串是 "ba", 它是 "ab" 的异位词。
起始索引等于 2 的子串是 "ab", 它是 "ab" 的异位词。


提示:

1 <= s.length, p.length <= 3 * 10^4
s 和 p 仅包含小写字母

## 题目解析

要求在字符串s中寻找与字符串中的元素相同且数量相等的子串，各字符的顺序可不同，且字符串中均为小写字母。

返回值：返回符合要求的子串的起始位置构成的一个序列

## 题解

### 方法一：定长滑动窗口+字符统计

1、方法描述

（1）基本思想：保持窗口的大小与字符串p的长度相等（首先保证满足字符数量相等的条件），再通过词频统计，保证处于窗口中的字符与字符串p中的字符相同。

（2）判断方法：当两个字符串中对于26个字母的词频统计结果完全相同时，证明此时滑动窗口中的字符与字符串p中的字符相同。

①词频初始化：

大小：字符串p的大小

值：26个英文字符的ASCII码值（相对值，原因尚在探究中）的出现的频率的统计

scount初始化为字符串s中前p.size()个字符的统计结果

pcount初始化为字符串p中全部字符的统计结果

②词频变化：由滑动窗口的变化引起

窗口右移的过程中：窗口右边界新加入的字符词频增加，窗口左边界移出的字符词频减小

③结果保存：当前窗口中的额字符满足条件时，将此时窗口左边界的位置放到保存返回结果的变量中。

（3）窗口移动：若当前的窗口不满足条件，则窗口右移，且右移的位置不能超过字符串s与字符串p的大小差值。

滑动窗口的表现：对字符串s中与字符串p长度相等的字符进行词频统计。

左边界右移：将当前窗口原来的左边界的字符移出窗口（不对该字符进行词频统计，即将scount中对应位置的字符的值减1）

右边界右移：将当前窗口右边界后面的字符放到窗口中（对右边界后面的字符进行词频统计，即将scount中对应位置的字符的值加1）

（4）边界条件：当字符串s的长度小于字符串p的长度时，不能满足异位词的条件，则返回空

（5）结果返回：找到满足条件的子串时返回滑动窗口的左边界的位置并存入变量中，则最后返回找到所有满足条件的所有子串的开始位置构成的集合。

2、具体实现

```
class Solution {
public:
    vector<int> findAnagrams(string s, string p) {
       int m=s.size(); //字符串s的长度
       int n=p.size(); //字符串p的长度
       if (m<n) { //当字符串s的长度小于字符串p的长度时，返回空
           return vector<int>();
       }
       vector<int> result; //用于记录结果的变量
       vector<int> scount(26); //统计字符串s中的字母出现的频率
       vector<int> pcount(26); //统计字符串p中的字母出现的频率
       for(int i=0;i<n;++i) { //初始化对两个字符串的词频统计结果
           scount[s[i]-'a']++; //利用ASCII值（相对值）进行统计字符串s
           pcount[p[i]-'a']++; //利用ASCII值（相对值）进行统计字符串p
       }
       if (scount==pcount) { //当前滑动窗口中的字符与字符串p中的字符相同时保存当前滑动窗口左边界所在的位置,滑动窗口s[..n]
           result.push_back(0);
       }
       for(int i=0;i<m-n;++i) { //移动窗口，滑动窗口s[i+1..i+n]
           scount[s[i]-'a']--; //左边界右移，将字符串s中对该字符的词频统计剔除
           scount[s[i+n]-'a']++; //滑动窗口右移，维持窗口的大小与字符串p的长度相等
           if (scount==pcount) { 
               result.push_back(i+1); //返回滑动窗口的起始位置
           }
       }
       return result; //返回最后的结果
    }
};
/*
时间复杂度：O(m+(n−m)×Σ)，其中n为字符串s的长度，m为字符串 p的长度，Σ为所有可能的字符数。需要O(m)来统计字符串 p中每种字母的数量；需要O(m)来初始化滑动窗口；需要判断n-m+1个滑动窗口中每种字母的数量是否与字符串p中每种字母的数量相同，每次判断需O(Σ) 。因为s和p仅包含小写字母，所以Σ=26。

空间复杂度：O(Σ)。用于存储字符串p和滑动窗口中每种字母的数量。
*/
```

3、运行结果

![image-20220417203715767](C:\Users\DELL\AppData\Roaming\Typora\typora-user-images\image-20220417203715767.png)

## 方法二：变长滑动窗口+词频统计

1、方法描述：

（1）基本思想：使用变长的滑动窗口，通过词频统计结果判断当前的滑动窗口中的字符是否满足条件。

（2）词频统计：

①初始化：只对字符串p中的所有字符进行词频统计

②窗口移动：保证窗口内的字符的词频统计不大于0

（3）窗口移动：

左边界移动：当右边界不能移动时，就移动左边，直到左边界移动到右边界的右边作为下一个字符继续进行

右边界移动：当前窗口的右边界后面的字符时字符串p中的字符（判断：词频统计结果大于0）

（4）判断方法：当前滑动窗口的大小与字符串p的大小相等时，返回此时滑动窗口左边界的位置

2、具体实现：

```
class Solution {
public:
    vector<int> findAnagrams(string s, string p) {
        int freq[128]={0}; 
        for (int i=0;i<p.length();++i) {
            freq[p[i]]++;
        }
        int left=0; //滑动窗口s[left..right),即s[left..right]
        int right=0;
        vector<int> result;
        while (right <s.length()) {
            if (freq[s[right]]>0) { //右边界可以扩展（字符在p中出现过）
                freq[s[right]]--; //将该字符将入滑动窗口
                right++; //滑动窗口右移
                if (right-left ==p.length()) { //判断当前窗口是否满足条件 
                result.push_back(left); //保存窗口左边界的位置
                }
            }
            else {
                freq[s[left]]++; //左边界的字符词频增加，即将该字符从窗口中剔除
                left++; //左边界右移
            }
        }
        return result;
    }
};
```

```
class Solution {
public:
    vector<int> findAnagrams(string s, string p) {
        int m = s.size(), n = p.size();
        if(m < n) return {};
        vector<int> hashTable(26);
        for(auto ch : p) ++hashTable[ch - 'a'];
        vector<int> ret;
        for(int l = 0, r = 0; r < m; ++r) {
            --hashTable[s[r] - 'a'];
            while(hashTable[s[r] - 'a'] < 0) {
                ++hashTable[s[l] - 'a'];
                ++l;
            }
            if(r - l + 1 == n) ret.push_back(l);
        }
        return ret;
    }
};
```

3、运行结果

   ![image-20220417215834847](C:\Users\DELL\AppData\Roaming\Typora\typora-user-images\image-20220417215834847.png)

