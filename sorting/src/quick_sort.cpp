#include "../include/quick_sort.h"
#include <chrono>
#include <stack>
#include <random>

namespace Sorting {

template<typename T>
void QuickSort<T>::sort(std::vector<T>& arr, std::function<bool(const T&, const T&)> comp) {
    if (arr.size() <= 1) return;
    sortRange(arr, 0, static_cast<int>(arr.size()) - 1, comp);
}

template<typename T>
void QuickSort<T>::optimizedSort(std::vector<T>& arr, std::function<bool(const T&, const T&)> comp) {
    if (arr.size() <= 1) return;
    
    std::function<void(int, int)> quickSortOptimized = [&](int left, int right) {
        while (left < right) {
            // 小数组使用插入排序
            if (right - left < 10) {
                insertionSort(arr, left, right, comp);
                return;
            }
            
            // 三数取中选择基准
            int pivotIndex = medianOfThree(arr, left, right, comp);
            swap(arr[pivotIndex], arr[right]);
            
            int pivot = partitionLomuto(arr, left, right, comp);
            
            // 递归处理较小的部分，迭代处理较大的部分（尾递归优化）
            if (pivot - left < right - pivot) {
                quickSortOptimized(left, pivot - 1);
                left = pivot + 1;
            } else {
                quickSortOptimized(pivot + 1, right);
                right = pivot - 1;
            }
        }
    };
    
    quickSortOptimized(0, static_cast<int>(arr.size()) - 1);
}

template<typename T>
void QuickSort<T>::threeWaySort(std::vector<T>& arr, std::function<bool(const T&, const T&)> comp) {
    if (arr.size() <= 1) return;
    
    std::function<void(int, int)> threeWayQuickSort = [&](int left, int right) {
        if (left >= right) return;
        
        std::pair<int, int> partition_result = threeWayPartition(arr, left, right, comp);
        int lt = partition_result.first;
        int gt = partition_result.second;
        threeWayQuickSort(left, lt - 1);
        threeWayQuickSort(gt + 1, right);
    };
    
    threeWayQuickSort(0, static_cast<int>(arr.size()) - 1);
}

template<typename T>
void QuickSort<T>::iterativeSort(std::vector<T>& arr, std::function<bool(const T&, const T&)> comp) {
    if (arr.size() <= 1) return;
    
    std::stack<std::pair<int, int>> stack;
    stack.push({0, static_cast<int>(arr.size()) - 1});
    
    while (!stack.empty()) {
        std::pair<int, int> range = stack.top();
        stack.pop();
        
        int left = range.first;
        int right = range.second;
        
        if (left < right) {
            int pivot = partitionLomuto(arr, left, right, comp);
            stack.push(std::make_pair(left, pivot - 1));
            stack.push(std::make_pair(pivot + 1, right));
        }
    }
}

template<typename T>
void QuickSort<T>::sortRange(std::vector<T>& arr, int left, int right,
                            std::function<bool(const T&, const T&)> comp) {
    if (left >= right) return;
    
    int pivot = partitionLomuto(arr, left, right, comp);
    sortRange(arr, left, pivot - 1, comp);
    sortRange(arr, pivot + 1, right, comp);
}

template<typename T>
typename QuickSort<T>::SortStats QuickSort<T>::sortWithStats(std::vector<T>& arr, 
                                                            std::function<bool(const T&, const T&)> comp) {
    SortStats stats;
    if (arr.size() <= 1) return stats;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::function<void(int, int, int)> quickSortWithStats = [&](int left, int right, int depth) {
        if (left >= right) return;
        
        stats.recursionDepth = std::max(stats.recursionDepth, depth);
        stats.partitions++;
        
        int pivot = partitionLomuto(arr, left, right, comp);
        quickSortWithStats(left, pivot - 1, depth + 1);
        quickSortWithStats(pivot + 1, right, depth + 1);
    };
    
    quickSortWithStats(0, static_cast<int>(arr.size()) - 1, 1);
    
    auto end = std::chrono::high_resolution_clock::now();
    stats.timeMs = std::chrono::duration<double, std::milli>(end - start).count();
    
    return stats;
}

template<typename T>
void QuickSort<T>::sortWithPivotStrategy(std::vector<T>& arr, PivotStrategy strategy,
                                        std::function<bool(const T&, const T&)> comp) {
    if (arr.size() <= 1) return;
    
    std::function<void(int, int)> quickSortWithStrategy = [&](int left, int right) {
        if (left >= right) return;
        
        int pivotIndex = left;
        switch (strategy) {
            case PivotStrategy::First:
                pivotIndex = left;
                break;
            case PivotStrategy::Last:
                pivotIndex = right;
                break;
            case PivotStrategy::Middle:
                pivotIndex = left + (right - left) / 2;
                break;
            case PivotStrategy::Random: {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(left, right);
                pivotIndex = dis(gen);
                break;
            }
            case PivotStrategy::MedianOfThree:
                pivotIndex = medianOfThree(arr, left, right, comp);
                break;
        }
        
        swap(arr[pivotIndex], arr[right]);
        int pivot = partitionLomuto(arr, left, right, comp);
        
        quickSortWithStrategy(left, pivot - 1);
        quickSortWithStrategy(pivot + 1, right);
    };
    
    quickSortWithStrategy(0, static_cast<int>(arr.size()) - 1);
}

template<typename T>
int QuickSort<T>::partitionLomuto(std::vector<T>& arr, int left, int right,
                                 std::function<bool(const T&, const T&)> comp) {
    T pivot = arr[right];
    int i = left - 1;
    
    for (int j = left; j < right; j++) {
        if (comp(arr[j], pivot) || (!comp(pivot, arr[j]) && !comp(arr[j], pivot))) {
            i++;
            swap(arr[i], arr[j]);
        }
    }
    
    swap(arr[i + 1], arr[right]);
    return i + 1;
}

template<typename T>
int QuickSort<T>::partitionHoare(std::vector<T>& arr, int left, int right,
                                std::function<bool(const T&, const T&)> comp) {
    T pivot = arr[left];
    int i = left - 1;
    int j = right + 1;
    
    while (true) {
        do {
            i++;
        } while (comp(arr[i], pivot));
        
        do {
            j--;
        } while (comp(pivot, arr[j]));
        
        if (i >= j) return j;
        
        swap(arr[i], arr[j]);
    }
}

template<typename T>
std::pair<int, int> QuickSort<T>::threeWayPartition(std::vector<T>& arr, int left, int right,
                                                   std::function<bool(const T&, const T&)> comp) {
    T pivot = arr[left];
    int lt = left;      // arr[left..lt-1] < pivot
    int i = left + 1;   // arr[lt..i-1] == pivot
    int gt = right + 1; // arr[gt..right] > pivot
    
    while (i < gt) {
        if (comp(arr[i], pivot)) {
            swap(arr[lt++], arr[i++]);
        } else if (comp(pivot, arr[i])) {
            swap(arr[i], arr[--gt]);
        } else {
            i++;
        }
    }
    
    return {lt, gt};
}

template<typename T>
int QuickSort<T>::medianOfThree(std::vector<T>& arr, int left, int right,
                               std::function<bool(const T&, const T&)> comp) {
    int mid = left + (right - left) / 2;
    
    if (comp(arr[right], arr[left])) {
        swap(arr[left], arr[right]);
    }
    if (comp(arr[mid], arr[left])) {
        swap(arr[left], arr[mid]);
    }
    if (comp(arr[right], arr[mid])) {
        swap(arr[mid], arr[right]);
    }
    
    return mid;
}

template<typename T>
void QuickSort<T>::swap(T& a, T& b) {
    T temp = std::move(a);
    a = std::move(b);
    b = std::move(temp);
}

template<typename T>
void QuickSort<T>::insertionSort(std::vector<T>& arr, int left, int right,
                                std::function<bool(const T&, const T&)> comp) {
    for (int i = left + 1; i <= right; i++) {
        T key = std::move(arr[i]);
        int j = i - 1;
        
        while (j >= left && comp(key, arr[j])) {
            arr[j + 1] = std::move(arr[j]);
            j--;
        }
        
        arr[j + 1] = std::move(key);
    }
}

// 便捷函数实现
template<typename T>
void quickSort(std::vector<T>& arr) {
    QuickSort<T>::sort(arr);
}

template<typename T>
void quickSortDescending(std::vector<T>& arr) {
    QuickSort<T>::sort(arr, std::greater<T>());
}

template<typename T>
void threeWayQuickSort(std::vector<T>& arr) {
    QuickSort<T>::threeWaySort(arr);
}

// 显式实例化常用类型
template class QuickSort<int>;
template class QuickSort<float>;
template class QuickSort<double>;
template class QuickSort<std::string>;

template void quickSort<int>(std::vector<int>&);
template void quickSort<float>(std::vector<float>&);
template void quickSort<double>(std::vector<double>&);
template void quickSort<std::string>(std::vector<std::string>&);

template void quickSortDescending<int>(std::vector<int>&);
template void quickSortDescending<float>(std::vector<float>&);
template void quickSortDescending<double>(std::vector<double>&);

template void threeWayQuickSort<int>(std::vector<int>&);
template void threeWayQuickSort<float>(std::vector<float>&);
template void threeWayQuickSort<double>(std::vector<double>&);

} // namespace Sorting 