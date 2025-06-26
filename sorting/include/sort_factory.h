#pragma once

#include "bubble_sort.h"
#include "selection_sort.h"
#include "insertion_sort.h"
#include "quick_sort.h"
#include "merge_sort.h"
#include <vector>
#include <functional>
#include <string>
#include <chrono>

/**
 * 排序算法工厂类
 * ===============
 * 
 * 提供统一的排序接口，支持多种排序算法的选择和比较。
 * 包含算法推荐系统，根据数据特征自动选择最优算法。
 */

namespace Sorting {

/**
 * 排序算法类型枚举
 */
enum class SortType {
    BubbleSort,         // 冒泡排序
    SelectionSort,      // 选择排序
    InsertionSort,      // 插入排序
    QuickSort,          // 快速排序
    MergeSort,          // 归并排序
    OptimizedQuickSort, // 优化快速排序
    OptimizedMergeSort, // 优化归并排序
    ThreeWayQuickSort,  // 三路快速排序
    BottomUpMergeSort,  // 自底向上归并排序
    Auto                // 自动选择最优算法
};

/**
 * 数据特征描述
 */
struct DataCharacteristics {
    size_t size = 0;                    // 数据规模
    bool isNearlySorted = false;        // 是否基本有序
    bool hasManyDuplicates = false;     // 是否有大量重复元素
    bool isMemoryLimited = false;       // 是否内存受限
    bool requiresStability = false;     // 是否要求稳定性
    bool isRealTimeProcessing = false;  // 是否实时处理
};

/**
 * 排序性能统计
 */
struct SortPerformance {
    SortType algorithm;
    double timeMs = 0.0;
    int comparisons = 0;
    int swaps = 0;
    int memoryUsage = 0;  // 额外内存使用量
    std::string description;
};

/**
 * 排序算法工厂类
 */
template<typename T>
class SortFactory {
public:
    /**
     * 使用指定算法排序
     * @param arr 待排序数组
     * @param type 排序算法类型
     * @param comp 比较函数
     */
    static void sort(std::vector<T>& arr, SortType type,
                    std::function<bool(const T&, const T&)> comp = std::less<T>());

    /**
     * 自动选择最优算法进行排序
     * @param arr 待排序数组
     * @param characteristics 数据特征
     * @param comp 比较函数
     * @return 选择的算法类型
     */
    static SortType autoSort(std::vector<T>& arr, 
                            const DataCharacteristics& characteristics = DataCharacteristics{},
                            std::function<bool(const T&, const T&)> comp = std::less<T>());

    /**
     * 推荐最优排序算法
     * @param size 数据规模
     * @param characteristics 数据特征
     * @return 推荐的算法类型
     */
    static SortType recommendAlgorithm(size_t size, const DataCharacteristics& characteristics);

    /**
     * 性能基准测试
     * @param testData 测试数据
     * @param algorithms 要测试的算法列表
     * @return 性能统计结果
     */
    static std::vector<SortPerformance> benchmark(const std::vector<T>& testData,
                                                  const std::vector<SortType>& algorithms = {});

    /**
     * 获取算法信息
     * @param type 算法类型
     * @return 算法描述信息
     */
    static std::string getAlgorithmInfo(SortType type);

    /**
     * 获取算法复杂度信息
     * @param type 算法类型
     * @return 时间和空间复杂度描述
     */
    static std::string getComplexityInfo(SortType type);

    /**
     * 检查数组是否已排序
     * @param arr 数组
     * @param comp 比较函数
     * @return 是否已排序
     */
    static bool isSorted(const std::vector<T>& arr,
                        std::function<bool(const T&, const T&)> comp = std::less<T>());

    /**
     * 分析数据特征
     * @param arr 数据数组
     * @return 数据特征描述
     */
    static DataCharacteristics analyzeData(const std::vector<T>& arr);

    /**
     * 生成测试数据
     */
    struct TestDataGenerator {
        static std::vector<T> randomData(size_t size, T minVal = T{}, T maxVal = T{});
        static std::vector<T> sortedData(size_t size, T start = T{});
        static std::vector<T> reverseSortedData(size_t size, T start = T{});
        static std::vector<T> nearlySortedData(size_t size, double sortedRatio = 0.9);
        static std::vector<T> duplicateData(size_t size, size_t uniqueCount);
    };

private:
    // 执行具体的排序算法
    static void executeSortAlgorithm(std::vector<T>& arr, SortType type,
                                    std::function<bool(const T&, const T&)> comp);

    // 测量排序性能
    static SortPerformance measurePerformance(std::vector<T> arr, SortType type,
                                             std::function<bool(const T&, const T&)> comp);

    // 检查数据是否基本有序
    static bool isNearlySorted(const std::vector<T>& arr, double threshold = 0.8);

    // 检查是否有大量重复元素
    static bool hasManyDuplicates(const std::vector<T>& arr, double threshold = 0.3);
};

// 便捷函数
template<typename T>
void sortAuto(std::vector<T>& arr);

template<typename T>
void sortWithType(std::vector<T>& arr, SortType type);

template<typename T>
SortType getBestAlgorithm(size_t size, bool isNearlySorted = false, 
                         bool hasDuplicates = false, bool requiresStability = false);

} // namespace Sorting 