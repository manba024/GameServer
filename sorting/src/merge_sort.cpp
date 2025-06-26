#include "../include/merge_sort.h"
#include <chrono>
#include <queue>

namespace Sorting {

template<typename T>
void MergeSort<T>::sort(std::vector<T>& arr, std::function<bool(const T&, const T&)> comp) {
    if (arr.size() <= 1) return;
    
    std::vector<T> temp(arr.size());
    mergeSortRecursive(arr, temp, 0, static_cast<int>(arr.size()) - 1, comp);
}

template<typename T>
void MergeSort<T>::bottomUpSort(std::vector<T>& arr, std::function<bool(const T&, const T&)> comp) {
    if (arr.size() <= 1) return;
    
    int n = static_cast<int>(arr.size());
    std::vector<T> temp(n);
    
    // 自底向上归并，子数组大小从1开始，每次翻倍
    for (int size = 1; size < n; size *= 2) {
        for (int left = 0; left < n - size; left += 2 * size) {
            int mid = left + size - 1;
            int right = std::min(left + 2 * size - 1, n - 1);
            merge(arr, temp, left, mid, right, comp);
        }
    }
}

template<typename T>
void MergeSort<T>::optimizedSort(std::vector<T>& arr, std::function<bool(const T&, const T&)> comp) {
    if (arr.size() <= 1) return;
    
    std::vector<T> temp(arr.size());
    
    std::function<void(int, int)> optimizedMergeSort = [&](int left, int right) {
        if (left >= right) return;
        
        // 小数组使用插入排序
        if (right - left < 10) {
            insertionSort(arr, left, right, comp);
            return;
        }
        
        int mid = left + (right - left) / 2;
        optimizedMergeSort(left, mid);
        optimizedMergeSort(mid + 1, right);
        
        // 如果已经有序，跳过归并
        if (!comp(arr[mid + 1], arr[mid])) {
            return;
        }
        
        merge(arr, temp, left, mid, right, comp);
    };
    
    optimizedMergeSort(0, static_cast<int>(arr.size()) - 1);
}

template<typename T>
void MergeSort<T>::inPlaceSort(std::vector<T>& arr, std::function<bool(const T&, const T&)> comp) {
    if (arr.size() <= 1) return;
    
    std::function<void(int, int)> inPlaceMergeSort = [&](int left, int right) {
        if (left >= right) return;
        
        int mid = left + (right - left) / 2;
        inPlaceMergeSort(left, mid);
        inPlaceMergeSort(mid + 1, right);
        inPlaceMerge(arr, left, mid, right, comp);
    };
    
    inPlaceMergeSort(0, static_cast<int>(arr.size()) - 1);
}

template<typename T>
void MergeSort<T>::sortRange(std::vector<T>& arr, int left, int right,
                            std::function<bool(const T&, const T&)> comp) {
    if (left >= right) return;
    
    std::vector<T> temp(right - left + 1);
    
    std::function<void(int, int)> mergeSortRange = [&](int l, int r) {
        if (l >= r) return;
        
        int mid = l + (r - l) / 2;
        mergeSortRange(l, mid);
        mergeSortRange(mid + 1, r);
        merge(arr, temp, l, mid, r, comp);
    };
    
    mergeSortRange(left, right);
}

template<typename T>
typename MergeSort<T>::SortStats MergeSort<T>::sortWithStats(std::vector<T>& arr, 
                                                            std::function<bool(const T&, const T&)> comp) {
    SortStats stats;
    if (arr.size() <= 1) return stats;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<T> temp(arr.size());
    
    std::function<void(int, int, int)> mergeSortWithStats = [&](int left, int right, int depth) {
        if (left >= right) return;
        
        stats.recursionDepth = std::max(stats.recursionDepth, depth);
        
        int mid = left + (right - left) / 2;
        mergeSortWithStats(left, mid, depth + 1);
        mergeSortWithStats(mid + 1, right, depth + 1);
        
        stats.merges++;
        
        // 统计归并过程
        int i = left, j = mid + 1, k = left;
        while (i <= mid && j <= right) {
            stats.comparisons++;
            stats.arrayAccesses += 2;
            if (comp(arr[i], arr[j])) {
                temp[k++] = arr[i++];
            } else {
                temp[k++] = arr[j++];
            }
        }
        
        while (i <= mid) {
            temp[k++] = arr[i++];
            stats.arrayAccesses++;
        }
        
        while (j <= right) {
            temp[k++] = arr[j++];
            stats.arrayAccesses++;
        }
        
        for (int i = left; i <= right; i++) {
            arr[i] = temp[i];
            stats.arrayAccesses++;
        }
    };
    
    mergeSortWithStats(0, static_cast<int>(arr.size()) - 1, 1);
    
    auto end = std::chrono::high_resolution_clock::now();
    stats.timeMs = std::chrono::duration<double, std::milli>(end - start).count();
    
    return stats;
}

template<typename T>
void MergeSort<T>::kWayMergeSort(std::vector<T>& arr, int k, std::function<bool(const T&, const T&)> comp) {
    if (arr.size() <= 1 || k <= 1) {
        sort(arr, comp);
        return;
    }
    
    std::function<void(int, int)> kWayMergeSortRecursive = [&](int left, int right) {
        if (right - left < k) {
            sortRange(arr, left, right, comp);
            return;
        }
        
        int size = (right - left + 1) / k;
        std::vector<std::vector<T>> subArrays(k);
        
        // 分割数组
        for (int i = 0; i < k; i++) {
            int start = left + i * size;
            int end = (i == k - 1) ? right : start + size - 1;
            
            subArrays[i].assign(arr.begin() + start, arr.begin() + end + 1);
            kWayMergeSortRecursive(0, static_cast<int>(subArrays[i].size()) - 1);
        }
        
        // k路归并
        std::vector<T> result;
        kWayMerge(subArrays, result, comp);
        
        // 复制回原数组
        for (int i = 0; i < static_cast<int>(result.size()); i++) {
            arr[left + i] = result[i];
        }
    };
    
    kWayMergeSortRecursive(0, static_cast<int>(arr.size()) - 1);
}

template<typename T>
bool MergeSort<T>::isSorted(const std::vector<T>& arr, std::function<bool(const T&, const T&)> comp) {
    for (size_t i = 1; i < arr.size(); i++) {
        if (comp(arr[i], arr[i - 1])) {
            return false;
        }
    }
    return true;
}

template<typename T>
std::vector<T> MergeSort<T>::mergeTwoSortedArrays(const std::vector<T>& left, 
                                                 const std::vector<T>& right,
                                                 std::function<bool(const T&, const T&)> comp) {
    std::vector<T> result;
    result.reserve(left.size() + right.size());
    
    size_t i = 0, j = 0;
    while (i < left.size() && j < right.size()) {
        if (comp(left[i], right[j])) {
            result.push_back(left[i++]);
        } else {
            result.push_back(right[j++]);
        }
    }
    
    while (i < left.size()) {
        result.push_back(left[i++]);
    }
    
    while (j < right.size()) {
        result.push_back(right[j++]);
    }
    
    return result;
}

template<typename T>
void MergeSort<T>::mergeSortRecursive(std::vector<T>& arr, std::vector<T>& temp, 
                                     int left, int right,
                                     std::function<bool(const T&, const T&)> comp) {
    if (left >= right) return;
    
    int mid = left + (right - left) / 2;
    mergeSortRecursive(arr, temp, left, mid, comp);
    mergeSortRecursive(arr, temp, mid + 1, right, comp);
    merge(arr, temp, left, mid, right, comp);
}

template<typename T>
void MergeSort<T>::merge(std::vector<T>& arr, std::vector<T>& temp, 
                        int left, int mid, int right,
                        std::function<bool(const T&, const T&)> comp) {
    int i = left, j = mid + 1, k = left;
    
    // 归并两个有序子数组
    while (i <= mid && j <= right) {
        if (comp(arr[i], arr[j])) {
            temp[k++] = arr[i++];
        } else {
            temp[k++] = arr[j++];
        }
    }
    
    // 复制剩余元素
    while (i <= mid) {
        temp[k++] = arr[i++];
    }
    
    while (j <= right) {
        temp[k++] = arr[j++];
    }
    
    // 复制回原数组
    for (int i = left; i <= right; i++) {
        arr[i] = temp[i];
    }
}

template<typename T>
void MergeSort<T>::inPlaceMerge(std::vector<T>& arr, int left, int mid, int right,
                               std::function<bool(const T&, const T&)> comp) {
    // 原地归并的简化实现，时间复杂度较高但节省空间
    int start2 = mid + 1;
    
    // 如果已经有序，直接返回
    if (!comp(arr[start2], arr[mid])) {
        return;
    }
    
    while (left <= mid && start2 <= right) {
        if (!comp(arr[start2], arr[left])) {
            left++;
        } else {
            T value = arr[start2];
            int index = start2;
            
            // 向右移动元素
            while (index != left) {
                arr[index] = arr[index - 1];
                index--;
            }
            arr[left] = value;
            
            left++;
            mid++;
            start2++;
        }
    }
}

template<typename T>
void MergeSort<T>::insertionSort(std::vector<T>& arr, int left, int right,
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

template<typename T>
void MergeSort<T>::kWayMerge(std::vector<std::vector<T>>& arrays, std::vector<T>& result,
                            std::function<bool(const T&, const T&)> comp) {
    // 使用优先队列进行k路归并
    struct Element {
        T value;
        int arrayIndex;
        int elementIndex;
        std::function<bool(const T&, const T&)> comparator;
        
        Element(const T& v, int ai, int ei, std::function<bool(const T&, const T&)> c)
            : value(v), arrayIndex(ai), elementIndex(ei), comparator(c) {}
        
        bool operator>(const Element& other) const {
            return !comparator(value, other.value);
        }
    };
    
    std::priority_queue<Element, std::vector<Element>, std::greater<Element>> pq;
    
    // 初始化优先队列
    for (int i = 0; i < static_cast<int>(arrays.size()); i++) {
        if (!arrays[i].empty()) {
            pq.emplace(arrays[i][0], i, 0, comp);
        }
    }
    
    // k路归并
    while (!pq.empty()) {
        Element current = pq.top();
        pq.pop();
        
        result.push_back(current.value);
        
        // 如果当前数组还有元素，加入下一个元素
        if (current.elementIndex + 1 < static_cast<int>(arrays[current.arrayIndex].size())) {
            pq.emplace(
                arrays[current.arrayIndex][current.elementIndex + 1],
                current.arrayIndex,
                current.elementIndex + 1,
                comp
            );
        }
    }
}

// 便捷函数实现
template<typename T>
void mergeSort(std::vector<T>& arr) {
    MergeSort<T>::sort(arr);
}

template<typename T>
void mergeSortDescending(std::vector<T>& arr) {
    MergeSort<T>::sort(arr, std::greater<T>());
}

template<typename T>
void bottomUpMergeSort(std::vector<T>& arr) {
    MergeSort<T>::bottomUpSort(arr);
}

// 显式实例化常用类型
template class MergeSort<int>;
template class MergeSort<float>;
template class MergeSort<double>;
template class MergeSort<std::string>;

template void mergeSort<int>(std::vector<int>&);
template void mergeSort<float>(std::vector<float>&);
template void mergeSort<double>(std::vector<double>&);
template void mergeSort<std::string>(std::vector<std::string>&);

template void mergeSortDescending<int>(std::vector<int>&);
template void mergeSortDescending<float>(std::vector<float>&);
template void mergeSortDescending<double>(std::vector<double>&);

template void bottomUpMergeSort<int>(std::vector<int>&);
template void bottomUpMergeSort<float>(std::vector<float>&);
template void bottomUpMergeSort<double>(std::vector<double>&);

} // namespace Sorting 