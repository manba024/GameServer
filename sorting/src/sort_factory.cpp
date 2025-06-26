#include "../include/sort_factory.h"
#include <chrono>
#include <random>
#include <algorithm>
#include <unordered_set>

namespace Sorting {

template<typename T>
void SortFactory<T>::sort(std::vector<T>& arr, SortType type,
                         std::function<bool(const T&, const T&)> comp) {
    executeSortAlgorithm(arr, type, comp);
}

template<typename T>
SortType SortFactory<T>::autoSort(std::vector<T>& arr, 
                                 const DataCharacteristics& characteristics,
                                 std::function<bool(const T&, const T&)> comp) {
    DataCharacteristics actualCharacteristics = characteristics;
    
    // 如果没有提供特征，自动分析
    if (actualCharacteristics.size == 0) {
        actualCharacteristics = analyzeData(arr);
        actualCharacteristics.size = arr.size();
    }
    
    SortType chosenType = recommendAlgorithm(arr.size(), actualCharacteristics);
    executeSortAlgorithm(arr, chosenType, comp);
    
    return chosenType;
}

template<typename T>
SortType SortFactory<T>::recommendAlgorithm(size_t size, const DataCharacteristics& characteristics) {
    // 小数据集
    if (size < 50) {
        return SortType::InsertionSort;
    }
    
    // 基本有序的数据
    if (characteristics.isNearlySorted) {
        return SortType::InsertionSort;
    }
    
    // 要求稳定性
    if (characteristics.requiresStability) {
        return SortType::MergeSort;
    }
    
    // 内存受限
    if (characteristics.isMemoryLimited) {
        return SortType::OptimizedQuickSort;
    }
    
    // 大量重复元素
    if (characteristics.hasManyDuplicates) {
        return SortType::ThreeWayQuickSort;
    }
    
    // 实时处理
    if (characteristics.isRealTimeProcessing && size < 1000) {
        return SortType::InsertionSort;
    }
    
    // 一般情况，大数据集
    if (size > 10000) {
        return SortType::OptimizedQuickSort;
    } else {
        return SortType::MergeSort;
    }
}

template<typename T>
std::vector<SortPerformance> SortFactory<T>::benchmark(const std::vector<T>& testData,
                                                       const std::vector<SortType>& algorithms) {
    std::vector<SortType> testAlgorithms = algorithms;
    
    // 如果没有指定算法，使用默认列表
    if (testAlgorithms.empty()) {
        testAlgorithms = {
            SortType::BubbleSort,
            SortType::SelectionSort,
            SortType::InsertionSort,
            SortType::QuickSort,
            SortType::MergeSort
        };
        
        // 对于大数据集，跳过O(n²)算法
        if (testData.size() > 1000) {
            testAlgorithms = {
                SortType::InsertionSort,
                SortType::QuickSort,
                SortType::MergeSort,
                SortType::OptimizedQuickSort,
                SortType::OptimizedMergeSort
            };
        }
    }
    
    std::vector<SortPerformance> results;
    
    for (SortType type : testAlgorithms) {
        auto performance = measurePerformance(testData, type, std::less<T>());
        results.push_back(performance);
    }
    
    // 按时间排序
    std::sort(results.begin(), results.end(), 
              [](const SortPerformance& a, const SortPerformance& b) {
                  return a.timeMs < b.timeMs;
              });
    
    return results;
}

template<typename T>
std::string SortFactory<T>::getAlgorithmInfo(SortType type) {
    switch (type) {
        case SortType::BubbleSort:
            return "冒泡排序：简单的交换排序算法，适合小数据集和教学";
        case SortType::SelectionSort:
            return "选择排序：每次选择最小元素，交换次数少";
        case SortType::InsertionSort:
            return "插入排序：构建有序序列，对小数据集和基本有序数据高效";
        case SortType::QuickSort:
            return "快速排序：分治算法，平均性能优秀，适合大数据集";
        case SortType::MergeSort:
            return "归并排序：稳定的分治算法，时间复杂度稳定";
        case SortType::OptimizedQuickSort:
            return "优化快速排序：三数取中+小数组插入排序优化";
        case SortType::OptimizedMergeSort:
            return "优化归并排序：小数组插入排序优化";
        case SortType::ThreeWayQuickSort:
            return "三路快速排序：针对重复元素优化的快排";
        case SortType::BottomUpMergeSort:
            return "自底向上归并排序：迭代实现的归并排序";
        default:
            return "未知算法";
    }
}

template<typename T>
std::string SortFactory<T>::getComplexityInfo(SortType type) {
    switch (type) {
        case SortType::BubbleSort:
            return "时间: O(n²), 空间: O(1), 稳定";
        case SortType::SelectionSort:
            return "时间: O(n²), 空间: O(1), 不稳定";
        case SortType::InsertionSort:
            return "时间: O(n²), 空间: O(1), 稳定";
        case SortType::QuickSort:
        case SortType::OptimizedQuickSort:
        case SortType::ThreeWayQuickSort:
            return "时间: O(n log n), 空间: O(log n), 不稳定";
        case SortType::MergeSort:
        case SortType::OptimizedMergeSort:
        case SortType::BottomUpMergeSort:
            return "时间: O(n log n), 空间: O(n), 稳定";
        default:
            return "复杂度信息未知";
    }
}

template<typename T>
bool SortFactory<T>::isSorted(const std::vector<T>& arr,
                             std::function<bool(const T&, const T&)> comp) {
    for (size_t i = 1; i < arr.size(); i++) {
        if (comp(arr[i], arr[i - 1])) {
            return false;
        }
    }
    return true;
}

template<typename T>
DataCharacteristics SortFactory<T>::analyzeData(const std::vector<T>& arr) {
    DataCharacteristics characteristics;
    characteristics.size = arr.size();
    
    if (arr.empty()) return characteristics;
    
    // 检查是否基本有序
    characteristics.isNearlySorted = isNearlySorted(arr);
    
    // 检查是否有大量重复元素
    characteristics.hasManyDuplicates = hasManyDuplicates(arr);
    
    return characteristics;
}

template<typename T>
void SortFactory<T>::executeSortAlgorithm(std::vector<T>& arr, SortType type,
                                         std::function<bool(const T&, const T&)> comp) {
    switch (type) {
        case SortType::BubbleSort:
            BubbleSort<T>::sort(arr, comp);
            break;
        case SortType::SelectionSort:
            SelectionSort<T>::sort(arr, comp);
            break;
        case SortType::InsertionSort:
            InsertionSort<T>::sort(arr, comp);
            break;
        case SortType::QuickSort:
            QuickSort<T>::sort(arr, comp);
            break;
        case SortType::MergeSort:
            MergeSort<T>::sort(arr, comp);
            break;
        case SortType::OptimizedQuickSort:
            QuickSort<T>::optimizedSort(arr, comp);
            break;
        case SortType::OptimizedMergeSort:
            MergeSort<T>::optimizedSort(arr, comp);
            break;
        case SortType::ThreeWayQuickSort:
            QuickSort<T>::threeWaySort(arr, comp);
            break;
        case SortType::BottomUpMergeSort:
            MergeSort<T>::bottomUpSort(arr, comp);
            break;
        case SortType::Auto:
            autoSort(arr, DataCharacteristics{}, comp);
            break;
    }
}

template<typename T>
SortPerformance SortFactory<T>::measurePerformance(std::vector<T> arr, SortType type,
                                                   std::function<bool(const T&, const T&)> comp) {
    SortPerformance performance;
    performance.algorithm = type;
    performance.description = getAlgorithmInfo(type);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        executeSortAlgorithm(arr, type, comp);
        
        auto end = std::chrono::high_resolution_clock::now();
        performance.timeMs = std::chrono::duration<double, std::milli>(end - start).count();
        
        // 简单的内存使用量估算
        switch (type) {
            case SortType::MergeSort:
            case SortType::OptimizedMergeSort:
            case SortType::BottomUpMergeSort:
                performance.memoryUsage = static_cast<int>(arr.size() * sizeof(T));
                break;
            default:
                performance.memoryUsage = static_cast<int>(sizeof(T)); // 常数空间
                break;
        }
        
    } catch (const std::exception& e) {
        performance.timeMs = -1; // 表示失败
        performance.description += " (执行失败)";
    }
    
    return performance;
}

template<typename T>
bool SortFactory<T>::isNearlySorted(const std::vector<T>& arr, double threshold) {
    if (arr.size() <= 1) return true;
    
    int inversions = 0;
    int totalPairs = 0;
    
    // 采样检查，避免O(n²)复杂度
    int sampleSize = std::min(static_cast<int>(arr.size()), 100);
    
    for (int i = 0; i < sampleSize - 1; i++) {
        for (int j = i + 1; j < sampleSize; j++) {
            totalPairs++;
            if (arr[i] > arr[j]) {
                inversions++;
            }
        }
    }
    
    return (double)inversions / totalPairs < (1.0 - threshold);
}

template<typename T>
bool SortFactory<T>::hasManyDuplicates(const std::vector<T>& arr, double threshold) {
    if (arr.size() <= 1) return false;
    
    std::unordered_set<T> uniqueElements;
    
    // 采样检查
    int sampleSize = std::min(static_cast<int>(arr.size()), 1000);
    
    for (int i = 0; i < sampleSize; i++) {
        uniqueElements.insert(arr[i]);
    }
    
    double uniqueRatio = (double)uniqueElements.size() / sampleSize;
    return uniqueRatio < (1.0 - threshold);
}

// 模板特化：为不同类型提供不同的随机数据生成实现
template<>
std::vector<int> SortFactory<int>::TestDataGenerator::randomData(size_t size, int minVal, int maxVal) {
    std::vector<int> data;
    data.reserve(size);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(minVal, maxVal);
    
    for (size_t i = 0; i < size; i++) {
        data.push_back(dis(gen));
    }
    
    return data;
}

template<>
std::vector<float> SortFactory<float>::TestDataGenerator::randomData(size_t size, float minVal, float maxVal) {
    std::vector<float> data;
    data.reserve(size);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(minVal, maxVal);
    
    for (size_t i = 0; i < size; i++) {
        data.push_back(dis(gen));
    }
    
    return data;
}

template<>
std::vector<double> SortFactory<double>::TestDataGenerator::randomData(size_t size, double minVal, double maxVal) {
    std::vector<double> data;
    data.reserve(size);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(minVal, maxVal);
    
    for (size_t i = 0; i < size; i++) {
        data.push_back(dis(gen));
    }
    
    return data;
}

template<>
std::vector<std::string> SortFactory<std::string>::TestDataGenerator::randomData(size_t size, std::string minVal, std::string maxVal) {
    std::vector<std::string> data;
    data.reserve(size);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, 25);
    
    for (size_t i = 0; i < size; i++) {
        std::string str;
        int len = 3 + (gen() % 8); // 3-10个字符
        for (int j = 0; j < len; j++) {
            str += static_cast<char>('a' + dis(gen));
        }
        data.push_back(str);
    }
    
    return data;
}

// 通用模板实现（移除原来的实现）
template<typename T>
std::vector<T> SortFactory<T>::TestDataGenerator::randomData(size_t size, T minVal, T maxVal) {
    // 对于未特化的类型，返回空向量
    (void)minVal; (void)maxVal; // 避免未使用参数警告
    return std::vector<T>();
}

template<typename T>
std::vector<T> SortFactory<T>::TestDataGenerator::sortedData(size_t size, T start) {
    // 通用实现：只适用于数字类型
    std::vector<T> data;
    data.reserve(size);
    
    for (size_t i = 0; i < size; i++) {
        data.push_back(start + static_cast<T>(i));
    }
    
    return data;
}

// 数字类型的特化
template<>
std::vector<int> SortFactory<int>::TestDataGenerator::sortedData(size_t size, int start) {
    std::vector<int> data;
    data.reserve(size);
    
    for (size_t i = 0; i < size; i++) {
        data.push_back(start + static_cast<int>(i));
    }
    
    return data;
}

template<>
std::vector<float> SortFactory<float>::TestDataGenerator::sortedData(size_t size, float start) {
    std::vector<float> data;
    data.reserve(size);
    
    for (size_t i = 0; i < size; i++) {
        data.push_back(start + static_cast<float>(i));
    }
    
    return data;
}

template<>
std::vector<double> SortFactory<double>::TestDataGenerator::sortedData(size_t size, double start) {
    std::vector<double> data;
    data.reserve(size);
    
    for (size_t i = 0; i < size; i++) {
        data.push_back(start + static_cast<double>(i));
    }
    
    return data;
}

template<typename T>
std::vector<T> SortFactory<T>::TestDataGenerator::reverseSortedData(size_t size, T start) {
    // 通用实现：只适用于数字类型
    std::vector<T> data;
    data.reserve(size);
    
    for (size_t i = 0; i < size; i++) {
        data.push_back(start - static_cast<T>(i));
    }
    
    return data;
}

// 数字类型的特化
template<>
std::vector<int> SortFactory<int>::TestDataGenerator::reverseSortedData(size_t size, int start) {
    std::vector<int> data;
    data.reserve(size);
    
    for (size_t i = 0; i < size; i++) {
        data.push_back(start - static_cast<int>(i));
    }
    
    return data;
}

template<>
std::vector<float> SortFactory<float>::TestDataGenerator::reverseSortedData(size_t size, float start) {
    std::vector<float> data;
    data.reserve(size);
    
    for (size_t i = 0; i < size; i++) {
        data.push_back(start - static_cast<float>(i));
    }
    
    return data;
}

template<>
std::vector<double> SortFactory<double>::TestDataGenerator::reverseSortedData(size_t size, double start) {
    std::vector<double> data;
    data.reserve(size);
    
    for (size_t i = 0; i < size; i++) {
        data.push_back(start - static_cast<double>(i));
    }
    
    return data;
}

template<typename T>
std::vector<T> SortFactory<T>::TestDataGenerator::duplicateData(size_t size, size_t uniqueCount) {
    // 通用实现：只适用于数字类型
    std::vector<T> data;
    data.reserve(size);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dis(0, uniqueCount - 1);
    
    for (size_t i = 0; i < size; i++) {
        data.push_back(static_cast<T>(dis(gen)));
    }
    
    return data;
}

// 数字类型的特化
template<>
std::vector<int> SortFactory<int>::TestDataGenerator::duplicateData(size_t size, size_t uniqueCount) {
    std::vector<int> data;
    data.reserve(size);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dis(0, uniqueCount - 1);
    
    for (size_t i = 0; i < size; i++) {
        data.push_back(static_cast<int>(dis(gen)));
    }
    
    return data;
}

template<>
std::vector<float> SortFactory<float>::TestDataGenerator::duplicateData(size_t size, size_t uniqueCount) {
    std::vector<float> data;
    data.reserve(size);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dis(0, uniqueCount - 1);
    
    for (size_t i = 0; i < size; i++) {
        data.push_back(static_cast<float>(dis(gen)));
    }
    
    return data;
}

template<>
std::vector<double> SortFactory<double>::TestDataGenerator::duplicateData(size_t size, size_t uniqueCount) {
    std::vector<double> data;
    data.reserve(size);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dis(0, uniqueCount - 1);
    
    for (size_t i = 0; i < size; i++) {
        data.push_back(static_cast<double>(dis(gen)));
    }
    
    return data;
}

// 便捷函数实现
template<typename T>
void sortAuto(std::vector<T>& arr) {
    SortFactory<T>::autoSort(arr);
}

template<typename T>
void sortWithType(std::vector<T>& arr, SortType type) {
    SortFactory<T>::sort(arr, type);
}

template<typename T>
SortType getBestAlgorithm(size_t size, bool isNearlySorted, 
                         bool hasDuplicates, bool requiresStability) {
    DataCharacteristics characteristics;
    characteristics.size = size;
    characteristics.isNearlySorted = isNearlySorted;
    characteristics.hasManyDuplicates = hasDuplicates;
    characteristics.requiresStability = requiresStability;
    
    return SortFactory<T>::recommendAlgorithm(size, characteristics);
}

// 显式实例化常用类型
template class SortFactory<int>;
template class SortFactory<float>;
template class SortFactory<double>;
// template class SortFactory<std::string>; // 暂时禁用

template void sortAuto<int>(std::vector<int>&);
template void sortAuto<float>(std::vector<float>&);
template void sortAuto<double>(std::vector<double>&);

template void sortWithType<int>(std::vector<int>&, SortType);
template void sortWithType<float>(std::vector<float>&, SortType);
template void sortWithType<double>(std::vector<double>&, SortType);

template SortType getBestAlgorithm<int>(size_t, bool, bool, bool);
template SortType getBestAlgorithm<float>(size_t, bool, bool, bool);
template SortType getBestAlgorithm<double>(size_t, bool, bool, bool);

} // namespace Sorting 