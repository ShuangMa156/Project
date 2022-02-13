/*二分查找算法的理解与运用
重点：理解循环不变式的含义，并设置正确的循环条件
*/
#include <iostream>
#include <cmath>
#include <cassert>
#include <ctime>
#include "myuntil.h"//该头文件用于生成测试数据
using namespace std;
template<typename T>
int BinarySearch1(T arr[],
                 int n,
                 T target) {
  int left=0,right=n-1; //在[left...right]的范围内寻找target
  while (left<=right) { //循环条件包含等号
    //int mid=(left+right)/2;//存在整型变量溢出的风险
    int mid=left+(right-left)/2;
    if (arr[mid]==target)
        return target;
    if (target>arr[mid])
        left=mid+1; //target在[mid+1...right]中
    else //target<arr[mid]
        right=mid-1; //target在[left...mid-1]中
  }
  return -1;
}
template<typename T>
int BinarySearch2(T arr[],
                 int n,
                 T target) {
  int left=0,right=n; //在[left...right)的范围内寻找target
  while (left<right) { //循环条件不包含等号
    //int mid=(left+right)/2;//存在整型变量溢出的风险
    int mid=left+(right-left)/2;
    if (arr[mid]==target)
        return target;
    if (target>arr[mid])
        left=mid+1; //target在[mid+1...right)中
    else //target<arr[mid]
        right=mid; //target在[left...mid)中
  }
  return -1;
}
int main()
{
    int n=100000;
    int* data=myuntil::generateOrderedArray(n);
    clock_t startTime1=clock();
    for (int i=0;i<n;++i) {
        assert(i==BinarySearch1(data,n,i));
    }
    clock_t endTime1=clock();
    cout<<"BinarySearch1 test complete"<<endl;
    cout<<"Time cost:"<<double(endTime1-startTime1)/CLOCKS_PER_SEC<<" s"<<endl;
    cout<<endl;
    clock_t startTime2=clock();
    for (int i=0;i<n;++i) {
        assert(i==BinarySearch2(data,n,i));
    }
    clock_t endTime2=clock();
    cout<<"BinarySearch2 test complete"<<endl;
    cout<<"Time cost:"<<double(endTime2-startTime2)/CLOCKS_PER_SEC<<" s"<<endl;
    return 0;
}
