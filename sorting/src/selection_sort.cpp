#include "../include/selection_sort.h"
#include <chrono>

namespace Sorting {

template<typename T>
void SelectionSort<T>::sort(std::vector<T>& arr, std::function<bool(const T&, const T&)> comp) {
    if (arr.size() <= 1) return;
    
    size_t n = arr.size();
    for (size_t i = 0; i < n - 1; i++) {
        size_t minIndex = i;
        
        // 找到剩余元素中的最小值
        for (size_t j = i + 1; j < n; j++) {
            if (comp(arr[j], arr[minIndex])) {
                minIndex = j;
            }
        }
        
        // 交换到正确位置
        if (minIndex != i) {
            swap(arr[i], arr[minIndex]);
        }
    }
}

template<typename T>
void SelectionSort<T>::bidirectionalSort(std::vector<T>& arr, std::function<bool(const T&, const T&)> comp) {
    if (arr.size() <= 1) return;
    
    int left = 0;
    int right = arr.size() - 1;
    
    while (left < right) {
        int minIndex = left;
        int maxIndex = left;
        
        // 同时找最小值和最大值
        for (int i = left; i <= right; i++) {
            if (comp(arr[i], arr[minIndex])) {
                minIndex = i;
            }
            if (!comp(arr[i], arr[maxIndex]) && arr[i] != arr[maxIndex]) {
                maxIndex = i;
            }
        }
        
        // 将最小值交换到左端
        if (minIndex != left) {
            swap(arr[left], arr[minIndex]);
            // 如果最大值原本在left位置，现在它在minIndex位置
            if (maxIndex == left) {
                maxIndex = minIndex;
            }
        }
        
        // 将最大值交换到右端
        if (maxIndex != right) {
            swap(arr[right], arr[maxIndex]);
        }
        
        left++;
        right--;
    }
}

template<typename T>
typename SelectionSort<T>::SortStats SelectionSort<T>::sortWithStats(std::vector<T>& arr, 
                                                                     std::function<bool(const T&, const T&)> comp) {
    SortStats stats;
    if (arr.size() <= 1) return stats;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    size_t n = arr.size();
    for (size_t i = 0; i < n - 1; i++) {
        size_t minIndex = i;
        stats.selections++;
        
        for (size_t j = i + 1; j < n; j++) {
            stats.comparisons++;
            if (comp(arr[j], arr[minIndex])) {
                minIndex = j;
            }
        }
        
        if (minIndex != i) {
            swap(arr[i], arr[minIndex]);
            stats.swaps++;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    stats.timeMs = std::chrono::duration<double, std::milli>(end - start).count();
    
    return stats;
}

template<typename T>
int SelectionSort<T>::findExtremumIndex(const std::vector<T>& arr, int start, int end,
                                       std::function<bool(const T&, const T&)> comp) {
    if (start >= end || start < 0 || end >= static_cast<int>(arr.size())) {
        return -1;
    }
    
    int extremumIndex = start;
    for (int i = start + 1; i <= end; i++) {
        if (comp(arr[i], arr[extremumIndex])) {
            extremumIndex = i;
        }
    }
    
    return extremumIndex;
}

template<typename T>
bool SelectionSort<T>::isSorted(const std::vector<T>& arr, std::function<bool(const T&, const T&)> comp) {
    for (size_t i = 1; i < arr.size(); i++) {
        if (comp(arr[i], arr[i - 1])) {
            return false;
        }
    }
    return true;
}

template<typename T>
void SelectionSort<T>::swap(T& a, T& b) {
    T temp = std::move(a);
    a = std::move(b);
    b = std::move(temp);
}

// 便捷函数实现
template<typename T>
void selectionSort(std::vector<T>& arr) {
    SelectionSort<T>::sort(arr);
}

template<typename T>
void selectionSortDescending(std::vector<T>& arr) {
    SelectionSort<T>::sort(arr, std::greater<T>());
}

// 显式实例化常用类型
template class SelectionSort<int>;
template class SelectionSort<float>;
template class SelectionSort<double>;
template class SelectionSort<std::string>;

template void selectionSort<int>(std::vector<int>&);
template void selectionSort<float>(std::vector<float>&);
template void selectionSort<double>(std::vector<double>&);
template void selectionSort<std::string>(std::vector<std::string>&);

template void selectionSortDescending<int>(std::vector<int>&);
template void selectionSortDescending<float>(std::vector<float>&);
template void selectionSortDescending<double>(std::vector<double>&);

} // namespace Sorting 