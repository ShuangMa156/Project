# LeetCode349-两个数组的交集

## 题目描述

给定两个数组 nums1 和 nums2 ，返回 它们的交集 。输出结果中的每个元素一定是 唯一 的。我们可以 不考虑输出结果的顺序 。

示例 1：

输入：nums1 = [1,2,2,1], nums2 = [2,2]
输出：[2]
示例 2：

输入：nums1 = [4,9,5], nums2 = [9,4,9,8,4]
输出：[9,4]
解释：[4,9] 也是可通过的


提示：

1 <= nums1.length, nums2.length <= 1000
0 <= nums1[i], nums2[i] <= 1000

## 题解

### 方法一：利用容器类的特性解决问题

1、方法描述

（1）基本思想：利用集合中的元素的无序性的特点，运用数学思维直接求两个集合中的交集

（2）所用数据结构：集合set

（3）交集判断：元素出现在两个集合中

具体实现：将数组nums2中的元素与出现在数组nums1对应集合的元素比较，将值相同的元素保存到另一集合中

（4）返回结果：

要求的形式：向量形式的整型数组

交集结果：集合set

数据结构转换处理：遍历赋值

2、具体实现

```
class Solution {
public:
    vector<int> intersection(vector<int>& nums1, vector<int>& nums2) {
     set<int> record;//集合中的元素具有无序性，所以没有重复出现的元素
      for (int i=0;i<nums1.size();++i) {
        record.insert(nums1[i]);//当前record集合值包含nums1中的元素
    }
    set<int> resultSet;
    for (int i=0;i<nums2.size();++i) {
      if (record.find(nums2[i])!=record.end())
        resultSet.insert(nums2[i]);//将nums2中与nums1中相同的元素放入集合resultSet
    }
    vector<int> resultVector;
    for (set<int>::iterator iter=resultSet.begin();iter!=resultSet.end();iter++) {
        resultVector.push_back(*iter);//将集合ResultSet中的元素放到存放结果的向量resultVector中
    }
    return resultVector；
}
};
```

```
//代码优化版本
class Solution {
public:
    vector<int> intersection(vector<int>& nums1, vector<int>& nums2) {
    set<int> record(nums1.begin(),nums1.end());
    set<int> resultSet;
    for (int i=0;i<nums2.size();++i) {
      if (record.find(nums2[i])!=record.end())
        resultSet.insert(nums2[i]);
    }
    vector<int> result(resultSet.begin(),resultSet.end());
    return result;
    }
};
```

3、运行结果

![、](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\image-20220220211744507.png)

4、补充知识点---C++中集合类型的数据结构的相关操作

set作为一个容器也是用来存储同一数据类型的数据类型，并且能从一个数据集合中取出数据，在set中每个元素的值都唯一，而且系统能根据元素的值自动进行排序。应该注意的是set中数元素的值不能直接被改变。

（1）**它包含在头文件`#include<set>`中**

构造方法：
set< T > name;
就构造了一个存储数据T的的集合name

（2）基本操作：

#### 插入：

name.insert(T) ;
意为：向name集合里存入数据T
**注意如果集合中已经存在了某个元素，再次插入不会产生任何效果，集合中是不会出现重复元素的。**

#### 删除：

name.erase（T）；
意为：把name里的T删除
**注意如果集合里面没有T元素将不会有任何效果！**

#### 查找：

name.count(T);
意为：如果集合里有元素T，返回true，否则返回false
**注意这个查找的时间复杂度大概在O(log(n))，因为它是一种线性数据结构所以能够比较快速地查出这个元素**

#### 遍历

这个写法比较固定：

```delphi
for(set< T >::iterator it=name.begin();it!=name.end();it++)
```

可以当做模板背一下！

总结：
![在这里插入图片描述](https://img-blog.csdnimg.cn/20190709203608233.png)

```
begin()     　　 ,返回set容器的第一个元素

end() 　　　　 ,返回set容器的最后一个元素

clear()   　　     ,删除set容器中的所有的元素

empty() 　　　,判断set容器是否为空

max_size() 　 ,返回set容器可能包含的元素最大个数

size() 　　　　 ,返回当前set容器中的元素个数

rbegin　　　　 ,返回的值和end()相同

rend()　　　　 ,返回的值和rbegin()相同
```

另外，集合还有一个非常重要的特性：它里面不会有重复的数据，并且会帮你自动排序！（结构体类型的需要你重新定义符号才能够正常运行哦)

