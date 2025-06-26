#include "../include/insertion_sort.h"
#include <chrono>

namespace Sorting {

template<typename T>
void InsertionSort<T>::sort(std::vector<T>& arr, std::function<bool(const T&, const T&)> comp) {
    if (arr.size() <= 1) return;
    
    for (size_t i = 1; i < arr.size(); i++) {
        T key = std::move(arr[i]);
        int j = static_cast<int>(i) - 1;
        
        // 将大于key的元素向右移动
        while (j >= 0 && comp(key, arr[j])) {
            arr[j + 1] = std::move(arr[j]);
            j--;
        }
        
        arr[j + 1] = std::move(key);
    }
}

template<typename T>
void InsertionSort<T>::binaryInsertionSort(std::vector<T>& arr, std::function<bool(const T&, const T&)> comp) {
    if (arr.size() <= 1) return;
    
    for (size_t i = 1; i < arr.size(); i++) {
        T key = std::move(arr[i]);
        
        // 使用二分查找找到插入位置
        int pos = binarySearch(arr, 0, static_cast<int>(i) - 1, key, comp);
        
        // 将元素向右移动
        for (int j = static_cast<int>(i) - 1; j >= pos; j--) {
            arr[j + 1] = std::move(arr[j]);
        }
        
        arr[pos] = std::move(key);
    }
}

template<typename T>
void InsertionSort<T>::sortRange(std::vector<T>& arr, int left, int right,
                                std::function<bool(const T&, const T&)> comp) {
    if (left >= right) return;
    
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

template<typename T>
typename InsertionSort<T>::SortStats InsertionSort<T>::sortWithStats(std::vector<T>& arr, 
                                                                     std::function<bool(const T&, const T&)> comp) {
    SortStats stats;
    if (arr.size() <= 1) return stats;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 1; i < arr.size(); i++) {
        T key = std::move(arr[i]);
        int j = static_cast<int>(i) - 1;
        stats.insertions++;
        
        while (j >= 0 && comp(key, arr[j])) {
            stats.comparisons++;
            arr[j + 1] = std::move(arr[j]);
            stats.moves++;
            j--;
        }
        
        if (j >= 0) stats.comparisons++; // 最后一次比较
        arr[j + 1] = std::move(key);
        stats.moves++;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    stats.timeMs = std::chrono::duration<double, std::milli>(end - start).count();
    
    return stats;
}

template<typename T>
int InsertionSort<T>::binarySearch(const std::vector<T>& arr, int left, int right, 
                                  const T& key, std::function<bool(const T&, const T&)> comp) {
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        if (comp(key, arr[mid])) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }
    
    return left;
}

template<typename T>
bool InsertionSort<T>::isSorted(const std::vector<T>& arr, std::function<bool(const T&, const T&)> comp) {
    for (size_t i = 1; i < arr.size(); i++) {
        if (comp(arr[i], arr[i - 1])) {
            return false;
        }
    }
    return true;
}

template<typename T>
void InsertionSort<T>::shiftRight(std::vector<T>& arr, int pos, int count) {
    for (int i = 0; i < count; i++) {
        if (pos + i + 1 < static_cast<int>(arr.size())) {
            arr[pos + i + 1] = std::move(arr[pos + i]);
        }
    }
}

// 便捷函数实现
template<typename T>
void insertionSort(std::vector<T>& arr) {
    InsertionSort<T>::sort(arr);
}

template<typename T>
void insertionSortDescending(std::vector<T>& arr) {
    InsertionSort<T>::sort(arr, std::greater<T>());
}

template<typename T>
void binaryInsertionSort(std::vector<T>& arr) {
    InsertionSort<T>::binaryInsertionSort(arr);
}

// 显式实例化常用类型
template class InsertionSort<int>;
template class InsertionSort<float>;
template class InsertionSort<double>;
template class InsertionSort<std::string>;

template void insertionSort<int>(std::vector<int>&);
template void insertionSort<float>(std::vector<float>&);
template void insertionSort<double>(std::vector<double>&);
template void insertionSort<std::string>(std::vector<std::string>&);

template void insertionSortDescending<int>(std::vector<int>&);
template void insertionSortDescending<float>(std::vector<float>&);
template void insertionSortDescending<double>(std::vector<double>&);

template void binaryInsertionSort<int>(std::vector<int>&);
template void binaryInsertionSort<float>(std::vector<float>&);
template void binaryInsertionSort<double>(std::vector<double>&);

} // namespace Sorting 