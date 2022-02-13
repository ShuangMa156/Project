#ifndef MYUNTIL_H_INCLUDED
#define MYUNTIL_H_INCLUDED
#include <iostream>
#include <algorithm>
#include <cassert>
using namespace std;
namespace myuntil {
  int *generateRandomArray(int n,
                           int rangeL,
                           int rangeR) {
    assert(n>0 && rangeL<=rangeR);
    int *arr=new int[n];
    srand(time(NULL));
    for (int i=0;i<n;++i) {
      arr[i]=rand()%(rangeR-rangeL);
    }
    return arr;
  }

  int *generateOrderedArray(int n) {
    assert(n>0);
    int *arr=new int[n];
    for (int i=0;i<n;++i) {
      arr[i]=i;
    }
    return arr;
  }
}

#endif // MYUNTIL_H_INCLUDED
