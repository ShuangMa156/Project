# LeetCode75-颜色分类

## 题目描述

给定一个包含红色、白色和蓝色、共 n 个元素的数组 nums ，原地对它们进行排序，使得相同颜色的元素相邻，并按照红色、白色、蓝色顺序排列。

我们使用整数 0、 1 和 2 分别表示红色、白色和蓝色。

必须在不使用库的sort函数的情况下解决这个问题。

 

示例 1：

输入：nums = [2,0,2,1,1,0]
输出：[0,0,1,1,2,2]
示例 2：

输入：nums = [2,0,1]
输出：[0,1,2]


提示：

n == nums.length
1 <= n <= 300
nums[i] 为 0、1 或 2


进阶：

你可以不使用代码库中的排序函数来解决这道题吗？
你能想出一个仅使用常数空间的一趟扫描算法吗？

## 题解

### 方法一：计数排序分别统计

1、方法描述

（1）基本思想：对三种颜色分别进行计数统计，然后根据三种颜色的数量，依次覆盖原数组中对应位置的数值，使输入数组按指定的顺序排列。

（2）颜色分类：根据数值比较的结果将颜色分类

由于使用整数0、1、2分别表示红色、白色、蓝色，所以可以通过对原数组中0、1、2数值个数的统计，分别统计出三种颜色出现的次数。

（3）结果输出：根据统计得到的三种颜色的数量，按照红色-0、白色-1、蓝色-2的顺序依次将对应数量的颜色值写入原数组中，期间用控制原数组中下标移动的变量来指示要装入颜色的位置。

2、具体实现

```
class Solution {
public:
    void sortColors(vector<int>& nums) {
      int count[3]={0};//存放0、1、2三个元素出现的频率
      for (int i=0;i<nums.size();++i) {
        assert(nums[i]>=0 && nums[i]<=2);//断言，相当于错误处理，只有数值为0、1、2的元素才符合条件
        count[nums[i]]++;//统计对应颜色值的数量
      }
      int index=0;//下标，记录原数组中下一个要处理的元素，[0..index)表示已经处理过的元素，[index..nums.size()-1]表示未处理的元素
      for (int j=0;j<count[0];++j) {
        nums[index++]=0;//存放红色元素
      }
      for (int j=0;j<count[1];++j) {
        nums[index++]=1;//存放白色元素
      }
      for (int j=0;j<count[2];++j) {
        nums[index++]=2;//存放蓝色元素
      }
    }
};
```

```
//优化版
class Solution {
public:
    void sortColors(vector<int>& nums) {
     int count[3]={0};
     for (int i=0;i<nums.size();++i) {
       assert(nums[i]>=0 && nums[i]<=2);
       count[nums[i]]++;
     }
     int index=0;
     while (index<nums.size()) {//用while循环根据下标判断，避免多次循环
       if (index<count[0]) {
         nums[index++]=0;
       }
       else if (index<count[1]+count[0] && index>=count[0]) {
         nums[index++]=1;
       }
       else {
         nums[index++]=2;
       }
     }
    }
};
```

3、运行结果

![image-20220310201159391](C:\Users\DELL\AppData\Roaming\Typora\typora-user-images\image-20220310201159391.png)

![image-20220312222601789](C:\Users\DELL\AppData\Roaming\Typora\typora-user-images\image-20220312222601789.png)

### 方法二：三路快排

1、方法描述

（1）基本思想：将原数组划分为三个区域，分别存储红色-0、白色-1、蓝色-2,然后通过判断，移动元素到合适的位置。

（2）颜色分类：定义两个指针将数组分割为：红色[0..zero]、白色[zero+1..two-1]、蓝色[two..nums,size()-1]三部分，然后遍历数组，根据对应的颜色值将元素放到合适的区域

初始化：zero=-1,two=nums.size()

判断空间：(zero..two)

无需移动元素--处于白色区域：元素值为1，即nums[i]=1

需要移动到蓝色区域：元素值为2，即nums[i]=2--->前移two指针，并交换当前元素与two指针所指元素的位置

需要移动到红色区域：元素值为0，即nums[i]=0--->后移zero指针，并交换当前元素和zero指针所指的位置

（3）结果输出：交换处理后，原数组的元素已经按照颜色排好序

2、具体实现

```
class Solution {
public:
    void sortColors(vector<int>& nums) {
      int zero=-1;//初始化指向红色区域最后一个元素所在位置的指针
      int two=nums.size();//初始化指向蓝色区域第一个元素锁子啊位置的指针
      for (int i=zero+1;i<two;)//遍历位于白色区域（zero..two）区域的元素，挑选出蓝色区域和红色区域的元素并放置到对应的区域
      {
          if (nums[i]==1) ++i;//属于白色区域的元素
          else if (nums[i]==2) swap(nums[i],nums[--two]);//交换当前元素到蓝色区域（先移动two指针，再交换）
          else {
            assert(nums[i]==0);//断言，判断当前元素值是否为0，不是，则抛出异常
            swap(nums[i],nums[++zero]); //交换当前元素到红色区域（先移动zero指针，再交换）
            ++i; //移动下标，判断下一个元素
          }
      }
    }
};
```

3、运行结果

![image-20220310210104997](C:\Users\DELL\AppData\Roaming\Typora\typora-user-images\image-20220310210104997.png)

