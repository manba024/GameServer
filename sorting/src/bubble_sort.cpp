#include "../include/bubble_sort.h"
#include <chrono>

namespace Sorting {

template<typename T>
void BubbleSort<T>::sort(std::vector<T>& arr, std::function<bool(const T&, const T&)> comp) {
    if (arr.size() <= 1) return;
    
    size_t n = arr.size();
    for (size_t i = 0; i < n - 1; i++) {
        for (size_t j = 0; j < n - i - 1; j++) {
            if (comp(arr[j + 1], arr[j])) {
                swap(arr[j], arr[j + 1]);
            }
        }
    }
}

template<typename T>
int BubbleSort<T>::optimizedSort(std::vector<T>& arr, std::function<bool(const T&, const T&)> comp) {
    if (arr.size() <= 1) return 0;
    
    size_t n = arr.size();
    int passes = 0;
    
    for (size_t i = 0; i < n - 1; i++) {
        bool swapped = false;
        passes++;
        
        for (size_t j = 0; j < n - i - 1; j++) {
            if (comp(arr[j + 1], arr[j])) {
                swap(arr[j], arr[j + 1]);
                swapped = true;
            }
        }
        
        // 如果没有发生交换，数组已经有序
        if (!swapped) {
            break;
        }
    }
    
    return passes;
}

template<typename T>
void BubbleSort<T>::cocktailSort(std::vector<T>& arr, std::function<bool(const T&, const T&)> comp) {
    if (arr.size() <= 1) return;
    
    int left = 0;
    int right = arr.size() - 1;
    
    while (left < right) {
        // 向右冒泡
        for (int i = left; i < right; i++) {
            if (comp(arr[i + 1], arr[i])) {
                swap(arr[i], arr[i + 1]);
            }
        }
        right--;
        
        // 向左冒泡
        for (int i = right; i > left; i--) {
            if (comp(arr[i], arr[i - 1])) {
                swap(arr[i - 1], arr[i]);
            }
        }
        left++;
    }
}

template<typename T>
typename BubbleSort<T>::SortStats BubbleSort<T>::sortWithStats(std::vector<T>& arr, 
                                                               std::function<bool(const T&, const T&)> comp) {
    SortStats stats;
    if (arr.size() <= 1) return stats;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    size_t n = arr.size();
    for (size_t i = 0; i < n - 1; i++) {
        stats.passes++;
        bool swapped = false;
        
        for (size_t j = 0; j < n - i - 1; j++) {
            stats.comparisons++;
            if (comp(arr[j + 1], arr[j])) {
                swap(arr[j], arr[j + 1]);
                stats.swaps++;
                swapped = true;
            }
        }
        
        if (!swapped) break;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    stats.timeMs = std::chrono::duration<double, std::milli>(end - start).count();
    
    return stats;
}

template<typename T>
bool BubbleSort<T>::isSorted(const std::vector<T>& arr, std::function<bool(const T&, const T&)> comp) {
    for (size_t i = 1; i < arr.size(); i++) {
        if (comp(arr[i], arr[i - 1])) {
            return false;
        }
    }
    return true;
}

template<typename T>
void BubbleSort<T>::swap(T& a, T& b) {
    T temp = std::move(a);
    a = std::move(b);
    b = std::move(temp);
}

// 便捷函数实现
template<typename T>
void bubbleSort(std::vector<T>& arr) {
    BubbleSort<T>::sort(arr);
}

template<typename T>
void bubbleSortDescending(std::vector<T>& arr) {
    BubbleSort<T>::sort(arr, std::greater<T>());
}

// 显式实例化常用类型
template class BubbleSort<int>;
template class BubbleSort<float>;
template class BubbleSort<double>;
template class BubbleSort<std::string>;

template void bubbleSort<int>(std::vector<int>&);
template void bubbleSort<float>(std::vector<float>&);
template void bubbleSort<double>(std::vector<double>&);
template void bubbleSort<std::string>(std::vector<std::string>&);

template void bubbleSortDescending<int>(std::vector<int>&);
template void bubbleSortDescending<float>(std::vector<float>&);
template void bubbleSortDescending<double>(std::vector<double>&);

} // namespace Sorting 