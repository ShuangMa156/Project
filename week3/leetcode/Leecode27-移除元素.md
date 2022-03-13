# Leecode27-移除元素

## 题目描述

给你一个数组 nums 和一个值 val，你需要 原地 移除所有数值等于 val 的元素，并返回移除后数组的新长度。

不要使用额外的数组空间，你必须仅使用 O(1) 额外空间并 原地 修改输入数组。

元素的顺序可以改变。你不需要考虑数组中超出新长度后面的元素。

 

说明:

为什么返回数值是整数，但输出的答案是数组呢?

请注意，输入数组是以「引用」方式传递的，这意味着在函数里修改输入数组对于调用者是可见的。

你可以想象内部操作如下:

// nums 是以“引用”方式传递的。也就是说，不对实参作任何拷贝
int len = removeElement(nums, val);

// 在函数里修改输入数组对于调用者是可见的。
// 根据你的函数返回的长度, 它会打印出数组中 该长度范围内 的所有元素。
for (int i = 0; i < len; i++) {
    print(nums[i]);
}


示例 1：

输入：nums = [3,2,2,3], val = 3
输出：2, nums = [2,2]
解释：函数应该返回新的长度 2, 并且 nums 中的前两个元素均为 2。你不需要考虑数组中超出新长度后面的元素。例如，函数返回的新长度为 2 ，而 nums = [2,2,3,3] 或 nums = [2,2,0,0]，也会被视作正确答案。
示例 2：

输入：nums = [0,1,2,2,3,0,4,2], val = 2
输出：5, nums = [0,1,4,0,3]
解释：函数应该返回新的长度 5, 并且 nums 中的前五个元素为 0, 1, 3, 0, 4。注意这五个元素可为任意顺序。你不需要考虑数组中超出新长度后面的元素。

## 题解

### 方法一：下标法将数组中的元素分类

1、方法描述：

（1）基本思想：通过下标划分数组区间为元素值与指定值不相等的区间[0..k）和元素值与指定值相等的区间[k..nums.size()-1]

（2）初始化：默认数组中的所有元素值与指定值相等------>数组中的第一个元素k值对应的下标为数组中的第一个元素

（3）循环不变式：整型变量k始终指示数组中元素值与指定值等的区间中的第一个元素所在的位置。

（4）循环不变式的保持：遍历数组中的所有元素，将元素值与指定值不相等的元素放到k所指示的位置，然后k值后移。保证下标[0..k)指示元素值与指定值不相等的区间，[k..nums.size()-1]指示元素值与指定值相等的区间

2、具体实现

```
//保留原始数据：交换两个元素的位置
class Solution {
public:
    int removeElement(vector<int>& nums, int val) {
    int k=0;
    for (int i=0;i<nums.size();++i) {
      if(nums[i]!=val)
        swap(nums[i],nums[k++]);
    }
    return k;
    }
};
```

```
//不保留原始数据：直接将元素值与指定值相等的元素覆盖
class Solution {
public:
    int removeElement(vector<int>& nums, int val) {
    int k=0;
    for (int i=0;i<nums.size();++i) {
      if(nums[i]!=val)
        nums[k++]=nums[i];
    }
    return k;
    }
};
```

3、运行结果

![image-20220213181237765](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\image-20220213181237765.png)

![image-20220213181700488](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\image-20220213181700488.png)

### 方法二：双指针法，用指针分别指示原数组和存放结果的数组

1、方法描述

（1）基本思想：由于输出数组的长度一定小于等于输入数组的长度，所以可以将输出的数组直接写在输入数组上。

双指针：

​            右指针：指向当前将要处理的元素

​            左指针：指向下一个将要赋值的位置

（2）初始化：左指针指向数组中第一个元素所在的位置

（3）循环不变式：

​       ①区间[0..left)中的元素值都不等于指定的元素值val

​       ②left的值为输出数组的长度

（4）循环不变式的保持：

​       ①如果右指针指向的元素值不等于指定的元素值val，它一定是输出数组中的一个元素------>将右指针指向的元素赋值到左指针的位置，然后将左右指针同时右移

​       ②如果右指针指向的元素值等于指定的元素值val，它不在输出数组中----->左指针不动（区间[0..left)不用扩大），右指针右移

2、具体实现

```
//时间复杂度：O(n)，其中 n 为序列的长度。我们只需要遍历该序列至多两次。

//空间复杂度：O(1)。我们只需要常数的空间保存若干变量。
class Solution {
public:
    int removeElement(vector<int>& nums, int val) {
        int n = nums.size();
        int left = 0;
        for (int right = 0; right < n; right++) {
            if (nums[right] != val) {
                nums[left] = nums[right];
                left++;
            }
        }
        return left;
    }
};
```



```
//优化版本
/*如果要移除的元素恰好在数组的开头，例如序列[1,2,3,4,5]，当 val 为 11 时，我们需要把每一个元素都左移一位。注意到题目中说：「元素的顺序可以改变」。实际上我们可以直接将最后一个元素 5 移动到序列开头，取代元素 1，得到序列 [5,2,3,4]，同样满足题目要求。这个优化在序列中val 元素的数量较少时非常有效。

如果左指针left 指向的元素等于val，此时将右指针right 指向的元素复制到左指针 left 的位置，然后右指针right 左移一位。如果赋值过来的元素恰好也等于val，可以继续把右指针right 指向的元素的值赋值过来（左指针 left 指向的等于val 的元素的位置继续被覆盖），直到左指针指向的元素的值不等val 为止。

当左指针 \textit{left}left 和右指针 \textit{right}right 重合的时候，左右指针遍历完数组中所有的元素。

*/
class Solution {
public:
    int removeElement(vector<int>& nums, int val) {
    int left = 0;
    int right = nums.size();
    while (left < right) {
     if (nums[left] == val) {
        nums[left] = nums[right - 1];
        right--;
     } else {
          left++;
       }
    }
        return left;
    }
};
//时间复杂度：O(n)，其中 nn 为序列的长度。我们只需要遍历该序列至多一次。

//空间复杂度：O(1)。我们只需要常数的空间保存若干变量。
```

3、运行结果

![image-20220213182809576](C:\Users\Administrator\AppData\Roaming\Typora\typora-user-images\image-20220213182809576.png)

