# LeetCode283-移动零

## 题目描述

给定一个数组 nums，编写一个函数将所有 0 移动到数组的末尾，同时保持非零元素的相对顺序。

请注意 ，必须在不复制数组的情况下原地对数组进行操作。

示例 1:

输入: nums = [0,1,0,3,12]
输出: [1,3,12,0,0]
示例 2:

输入: nums = [0]
输出: [0]


提示:

1 <= nums.length <= 104
-2^31 <= nums[i] <= 2^31 - 1

## 题解

### 方法一：数组拷贝，先挑选出非零元素

1、方法描述

  (1)基本思想：将非零元素单独存放，最后按照顺序放回原数组，原数组中的剩下的位置即为元素0所在的位置。

（2）实现方法：由于辅助数组只保存原数组中非零元素，所以将元素从辅助数组赋值到原数组中，直接将原数组中对应位置的元素覆盖，再加上零元素的个数，元素的数量并未发生变化。

原数组遍历的过程中实现辅助数组的赋值操作，保证了原数组中非零元素的相对顺序不变。

2、具体实现

```
class Solution {
public:
    void moveZeroes(vector<int>& nums) {
        vector<int> nonZeroElements;
        for (int i=0;i<nums.size();++i) {
          if(nums[i])
            nonZeroElements.push_back(nums[i]);
        }
        for (int i=0;i<nonZeroElements.size();++i) {
          nums[i]=nonZeroElements[i];
        }
        for (int i=nonZeroElements.size();i<nums.size();++i) {
          nums[i]=0;
        }
    }
};
```

3、运行结果

![image-20220213172350971](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\image-20220213172350971.png)

### 方法二：双指针法，仅在原始数组中移动元素

1、方法描述

（1）基本思想：通过指针指示的数组下标，将原数组分为非零元素区和零元素区。

（2）实现：通过整型变量k标记数组中第一个零元素

数组中下标区间[0..k)为非零元素所在区间

数组中下标区间[k,nums.size()-1]为零元素所在区间

（3）循环不变式：k值表示的数组下标位置不变

（4）循环不变式的初始化和保持

初始化：指向数组中第一个元素所在的位置

保持：当当前元素值不为零时，将当前元素放到k所在的位置，并将k右移一位，保证k指向零元素区的第一个位置

2、具体实现

```
class Solution {
public:
    void moveZeroes(vector<int>& nums) {
        //时间复杂度O(n)
        //空间复杂度O(1)
        int k=0; //nums中，[0...k）的元素均为非0元素
        //遍历到第i个元素后，保证[0...i]中所有非0元素都按照顺序排列在[0...k)中
        for (int i=0;i<nums.size();++i) {
          if (nums[i]) {
            nums[k]=nums[i];
            k++;
            //nums[k++]=nums[i];
          }
        }
        //将nums中剩余的位置放置0
        for (int i=k;i<nums.size();++i) {
            nums[i]=0;
        }
    }
};
```

```
//优化版本：交换数组中的非零元素与零元素的位置
class Solution {
public:
    void moveZeroes(vector<int>& nums) {
        //时间复杂度O(n)
        //空间复杂度O(1)
        int k=0; //nums中，[0...k]的元素均为非0元素
        //遍历到第i个元素后，保证[0...i]中所有非0元素都按照顺序排列在[0...k)中、
        //同时，[k...i]为0
        for (int i=0;i<nums.size();++i) {
          if (nums[i]) {
            swap(nums[k],nums[i]);
            k++;
            //nums[k++]=nums[i];
          }
        }
    }
};
```

```
//优化版本考虑位置关系，如果k和i指向同一位置，则不交换
class Solution {
public:
    void moveZeroes(vector<int>& nums) {
        //时间复杂度O(n)
        //空间复杂度O(1)
        int k=0; //nums中，[0...k]的元素均为非0元素
        //遍历到第i个元素后，保证[0...i]中所有非0元素都按照顺序排列在[0...k)中、
        //同时，[k...i]为0
        for (int i=0;i<nums.size();++i) {
          if (nums[i]) {
            if (i!=k) {
              swap(nums[k++],nums[i]);
            }
            else  //i==k
              k++;
          }
        }
    }
};
```

3、运行结果

![image-20220213174148739](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\image-20220213174148739.png)

![image-20220213174355858](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\image-20220213174355858.png)

![image-20220213174702615](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\image-20220213174702615.png)

