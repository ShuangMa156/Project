# LeetCode167-两数之和II-输入有序数组

## 题目描述

给你一个下标从 1 开始的整数数组 numbers ，该数组已按 非递减顺序排列  ，请你从数组中找出满足相加之和等于目标数 target 的两个数。如果设这两个数分别是 numbers[index1] 和 numbers[index2] ，则 1 <= index1 < index2 <= numbers.length 。

以长度为 2 的整数数组 [index1, index2] 的形式返回这两个整数的下标 index1 和 index2。

你可以假设每个输入 只对应唯一的答案 ，而且你 不可以 重复使用相同的元素。

你所设计的解决方案必须只使用常量级的额外空间。


示例 1：

输入：numbers = [2,7,11,15], target = 9
输出：[1,2]
解释：2 与 7 之和等于目标数 9 。因此 index1 = 1, index2 = 2 。返回 [1, 2] 。
示例 2：

输入：numbers = [2,3,4], target = 6
输出：[1,3]
解释：2 与 4 之和等于目标数 6 。因此 index1 = 1, index2 = 3 。返回 [1, 3] 。
示例 3：

输入：numbers = [-1,0], target = -1
输出：[1,2]
解释：-1 与 0 之和等于目标数 -1 。因此 index1 = 1, index2 = 2 。返回 [1, 2] 。


提示：

2 <= numbers.length <= 3 * 104
-1000 <= numbers[i] <= 1000
numbers 按 非递减顺序 排列
-1000 <= target <= 1000
仅存在一个有效答案

## 题解

### 方法一：二分查找思路，寻找满足特定和的另一个值

1、方法描述

（1）基本思想：由于数组有序，所以可以从值较小的元素开始，然后与该元素右边的元素匹配，查找满足条件的元素。利用二分法的思想，通过与另一目标值的比较可以逐步划分查找范围的大小。

（2）目标值的寻找：

当前处理元素：数组中第i+1个元素(下标为i,因为计算机存储时数组下标默认从0开始)

查找区间：数组中在该元素右边的所有元素，即下标为[i+1..numbers.size()-1]

目标值：与numbers[i]的求和结果为target，即目标值为 target-numbers[i]

目标值的查找：将目标值与查找范围中点位置的元素值比较，直到二者的值相等

区间缩小：

①当目标值等于查找范围中点位置的元素值，返回当前处理元素在数组中的位置和该中点位置                   元素在原数组中的位置

②当目标值小于查找范围中点位置的元素值，则将查找范围缩小为中点位置到可查找范围的右边界

③当目标值大于查找范围中点位置的元素值，则将查找范围缩小为可查找范围的左边界到中点位置

查找条件：查找范围[low..high]是一个有效的区间，即 low<=high

（3）结果返回：

由于题目要求数组存储的过程中元素下标从1开始，且最终结果要返回两个元素在数组中的下标，而且是唯一的。

①当查找到满足条件的两个元素后，直接返回它们在数组中的位置（下标+1）

②考虑到可能存在无解的情况（题目中说有唯一解），则返回非法的数组下标示意

2、具体实现

```
class Solution {
public:
    vector<int> twoSum(vector<int>& numbers, int target) {
        for (int i=0;i<numbers.size();++i) { //从数组中第一个元素开始，依次查找符合条件的两个元素
            int low=i+1; //可查找范围的左边界
            int high=numbers.size()-1; //可查找范围的右边界
            while (low<=high) { //寻找是否存在numbers[i]的对应元素
                int mid=(high-low)/2+low; //可查找范围的中间位置
                if (numbers[mid]==target-numbers[i]) { //中间位置的元素为目标元素，直接返回结果
                     return {i+1,mid+1};
                } else if (numbers[mid] < target-numbers[i]) { //中间位置的元素小于目标元素，则扩大左边界到中间位置的右边
                     low=mid+1;
                } else { //中间位置的元素大于目标元素，则缩小右边界到中间位置的左边
                     high=mid-1;
                }
            }
        }
        return {-1,-1}; //未找到符合条件的目标值，则返回非法下标
    }
};
/*时间复杂度：O(nlgn)
  空间复杂度：O(1)
*/
```

3、具体实现

![image-20220315205949332](C:\Users\DELL\AppData\Roaming\Typora\typora-user-images\image-20220315205949332.png)

### 方法二：对撞指针

1、方法描述

（1）基本思想：利用数组有序的特点，通过两个指针分别指向两个参与运算的操作数,先考虑数组中的最小值与最大值之和是否符合目标值，然后再逐渐移动两个元素值所在的位置，直到找到符合条件的两个元素。

（2）目标值的寻找：

初始化两个指针：左指针指向数组中第一个元素，右指针指向数组中的最后一个的元素

判断：当前两个指针指向的元素之和是否等于目标值，若等于，则返回这两个指针所指向的元素在数组中的位置。

指针的移动：

​                  ①左指针移动：两个指针指向的元素值之和小于目标值

​                  ②右指针移动：两个指针所指向的元素之和大于目标值

（3）结果返回：

由于题目要求数组存储的过程中元素下标从1开始，且最终结果要返回两个元素在数组中的下标，而且是唯一的。

①当查找到满足条件的两个元素后，直接返回它们在数组中的位置（下标+1）

②考虑到可能存在无解的情况（题目中说有唯一解），则抛出异常

2、具体实现

```
class Solution {
public:
    vector<int> twoSum(vector<int>& numbers, int target) {
      //方法：对撞指针，O(n)
      assert(numbers.size()>=2);
      int left=0;
      int right=numbers.size()-1;
      while (left<right) { //由于结果为两个元素，所以边界条件为i<j
        if (numbers[left]+numbers[rigth]==target) {
          int result[2]={left+1,right+1};//数组下标从1开始，元素位置从0开始，所以要加1
          return vector<int>(result,result+2);
        }
        else if (numbers[left]+numbers[right]<target) {
          left++;
        } else {
            right--;
        }
      }
      throw invalid_argument("The input has no solution.");//抛出异常
    }
};
/*时间复杂度：O(n)
  空间复杂度：O(1)
*/
```

3、运行结果

![image-20220315213444741](C:\Users\DELL\AppData\Roaming\Typora\typora-user-images\image-20220315213444741.png)

