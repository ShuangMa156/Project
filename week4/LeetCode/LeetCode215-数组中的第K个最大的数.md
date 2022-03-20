# LeetCode215-数组中的第K个最大的数

## 题目描述

## 题解

### 方法一：调用C++中的排序函数先排序，根据下标输出指定位置的元素

1、方法描述

（1）基本思想：先排序，然后直接通过下标查找指定的元素

（2）排序：调用C++的库函数sort() （默认按照非降序排列）

（3）输出结果：下标索引排序后的数组中第K个元素所在的位置

2、具体实现

```
class Solution {
public:
    int findKthLargest(vector<int>& nums, int k) {
    sort(nums.begin(),nums.end()); //排序（默认按照非降序排列）
    return nums[nums.size()-k]; //输出第K个最大的元素
    }
};
```

3、运行结果

![image-20220313190504402](C:\Users\DELL\AppData\Roaming\Typora\typora-user-images\image-20220313190504402.png)

### 方法二：通过构建大根堆查找出数组中指定的元素

1、方法描述

（1）基本思想：根据大根堆的特性（根节点的元素值大于子节点的元素值），以及大根堆的存储关系，可以得到第K个最大的元素

（2）构建大根堆：

大根堆的特点：

建堆方法：

（3）输出第K个元素：依次

2、具体实现

```
class Solution {
public:
    /*  1、大根堆的特点
        2、大根堆的存储结构
        3、大根堆的建堆
        4、大根堆的元素查找
    */
    void adjust_Heap(vector<int>& nums,int root,int HeapSize) {
        if (root > HeapSize) return ; //对异常情况---根节点所在的位置超出了所建堆的大小
        int root_value=nums[root]; //保存根节点的值
        int child=root*2+1; //计算该堆的根节点所对应的子节点的位置
        while (child <= HeapSize) { //未遍历到堆中的最后一个节点，则在其子树中为根节点寻找合适的位置
            if (child+1<=HeapSize && nums[child] < nums[child+1]) { //该子节点的有兄弟且兄弟节点的值大于该子节点的值时，更新子节点所在的位置--->知道找到根节点的子树中值比较大的那个子节点，再与根节点作比较
                child++;
            }
            if (root_value >= nums[child]) {
                break; //根节点的值大于子节点的值时，退出循环
            } else {
                nums[root]=nums[child]; //根节点的值小于子节点的值时，交换两个节点的值
                root=child; //更新根节点的位置
                child=2*root+1; //更新子节点的位置
            }
        }
        nums[root]=root_value;//循环结束后，当前的根节点指向交换前其子节点的位置（调整后合适的位置），即root指向child所在的位置，将root原来的值放到该位置上
    }
    void build_maxHeap(vector<int>& nums) {
        int n = nums.size(); //整个堆的大小
        for (int root = (n-1)/2; root > -1; root --)
            adjust_Heap(nums, root, n - 1);//从最后的一个叶子节点n-1的根节点(n-1)/2开始建堆
    }
    int findKthLargest(vector<int>& nums, int k) {
        int n = nums.size(); //堆的大小
        build_maxHeap(nums); //构建最大堆
        for (int i = 0; i < k - 1; i ++) { //通过对的调整，将第K个元素放到所建堆的根节点的位置
            swap(nums[0], nums[n-1-i]); //交换根节点与自及诶单的位置
            adjust_Heap(nums, 0, n-1-i - 1); //调整堆
        }
        return nums[0]; //返回根节点的值，即数组中第K个最大元素
    }
};
```

3、运行结果



###  方法三：快速排序分治思想

1、方法描述

（1）基本思想：利用快速排序的划分操作将数组划分为两个部分，根据查找的元素，逐渐缩小查找范围

（2）划分数组：选取主元---->比较大小，划分为小于等于主元和大于主元两个部分

主元选取：数组中的第一个元素

划分条件：左边部分（小于等于）和右边部分（大于）不相交，即左边部分的最后一个元素所在的位置严格小于右边部分第一个元素所在的位置。

初始状态：左边部分[1..l]:小于等于

​                   右边部分[r..n-1]:大于

​                   中间部分(l..r):未处理

划分依据：元素值与选取的主元值的大小比较结果

​                 ①如果右边的值小于等于主元的值，则将该元素放到左边

​                 ②如果左边的值大于主元的值，则将该元素放到右边

对原数组排序：将元素按照比较结果排好序后，将主元放到中间位置

参数：待划分的数组、未处理的区域的左、右位置

（3）寻找第K个值：

判断：从数组的主元开始找起，通过位置判断是否是要找的元素

缩小范围：根据目标下标与主元下标的比较结果，判断应该查找哪一部分

​                ①主元的值大于所要查找的值：向左查找，修改查找范围的右边界

​                ②主元的值小于所要查找的值：向右查找，修改查找范围的左边界

2、具体实现

```
class Solution {
public:
    int partition(vector<int> & nums, int l, int r)
    {
        int pivot = nums[l]; //选取数组左边第一个元素作为主元
        while (l < r) //当数组的左右两部分未出现交叉时
        {
            while (l < r && nums[r] <= pivot) //如果右边的元素值小于等于主元，则将该元素放到左边
                r --;
            nums[l] = nums[r];
            while (l < r && nums[l] > pivot) //如果左边的元素值大于主元，则将该元素放到右边
                l ++;
            nums[r] = nums[l];
        }
        nums[l] = pivot;//将主元放到数组的中间（即此时left所指的位置或right所指的位置）
        return l; //返回主元所在的位置
    }
    int findKthLargest(vector<int>& nums, int k) 
    {
        int n = nums.size();
        int l = 0;
        int r = n - 1;
        while (true)
        {
            int idx = partition(nums, l, r); //找到主元所在的位置，缩小查找的范围，知道主元的值即为所要找的值
            if (idx == k - 1)
                return nums[idx];
            else if (idx < k - 1) //主元的值小于所要查找的值，则向右边查找
                l = idx + 1;
            else    //主元的值大于所要查找的值，则向右边查找
                r = idx - 1;
        }
    }
};
```

3、运行结果

![image-20220313222102178](C:\Users\DELL\AppData\Roaming\Typora\typora-user-images\image-20220313222102178.png)