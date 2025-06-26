#pragma once

#include <vector>
#include <functional>

/**
 * 快速排序 (Quick Sort)
 * ====================
 * 
 * 算法描述：
 * 快速排序是由C. A. R. Hoare在1960年提出的一种划分交换排序。
 * 它采用了一种分治的策略，通常称其为分治法(Divide-and-ConquerMethod)。
 * 该方法的基本思想是：选择一个基准元素，通过一趟排序将待排序的数据分割成
 * 独立的两部分，其中一部分的所有数据都比另外一部分的所有数据都要小，
 * 然后再按此方法对这两部分数据分别进行快速排序，整个排序过程可以递归进行。
 * 
 * 时间复杂度：
 * - 最佳情况：O(n log n) - 每次划分都能将数组均匀分成两部分
 * - 平均情况：O(n log n) - 随机数组的期望时间复杂度
 * - 最坏情况：O(n²) - 每次选择的基准都是最大或最小值（如已排序数组）
 * 
 * 空间复杂度：
 * - 最佳情况：O(log n) - 递归调用栈的深度
 * - 最坏情况：O(n) - 递归深度退化为线性
 * 
 * 稳定性：
 * - 不稳定排序 - 相等元素的相对位置可能会改变
 * 
 * 优点：
 * - 平均时间复杂度优秀：O(n log n)
 * - 原地排序，不需要额外的大量存储空间
 * - 实际性能通常比其他O(n log n)算法更好
 * - 分治思想，便于并行化实现
 * - 对于大数据集表现优秀
 * 
 * 缺点：
 * - 最坏情况时间复杂度为O(n²)
 * - 不是稳定排序算法
 * - 对于小数组效率不如插入排序
 * - 递归实现可能导致栈溢出
 * 
 * 适用场景：
 * - 大规模数据集排序
 * - 对平均性能要求高的场景
 * - 内存受限但需要高效排序的环境
 * - 不要求稳定性的排序需求
 * - 作为标准库排序算法的基础
 * 
 * 算法优化：
 * - 三数取中法选择基准
 * - 小数组使用插入排序
 * - 三路快排处理重复元素
 * - 尾递归优化
 * - 迭代实现避免栈溢出
 */

namespace Sorting {

/**
 * 快速排序模板类
 * 支持自定义比较函数和各种数据类型
 */
template<typename T>
class QuickSort {
public:
    /**
     * 基础快速排序（递归实现）
     * @param arr 待排序的数组
     * @param comp 比较函数，默认为升序
     */
    static void sort(std::vector<T>& arr, 
                    std::function<bool(const T&, const T&)> comp = std::less<T>());

    /**
     * 优化版快速排序（三数取中 + 小数组插入排序）
     * @param arr 待排序的数组
     * @param comp 比较函数，默认为升序
     */
    static void optimizedSort(std::vector<T>& arr, 
                             std::function<bool(const T&, const T&)> comp = std::less<T>());

    /**
     * 三路快速排序（处理大量重复元素）
     * @param arr 待排序的数组
     * @param comp 比较函数，默认为升序
     */
    static void threeWaySort(std::vector<T>& arr, 
                            std::function<bool(const T&, const T&)> comp = std::less<T>());

    /**
     * 迭代版快速排序（避免递归栈溢出）
     * @param arr 待排序的数组
     * @param comp 比较函数，默认为升序
     */
    static void iterativeSort(std::vector<T>& arr, 
                             std::function<bool(const T&, const T&)> comp = std::less<T>());

    /**
     * 指定范围的快速排序
     * @param arr 待排序的数组
     * @param left 起始索引
     * @param right 结束索引
     * @param comp 比较函数
     */
    static void sortRange(std::vector<T>& arr, int left, int right,
                         std::function<bool(const T&, const T&)> comp = std::less<T>());

    /**
     * 获取排序过程的详细统计信息
     */
    struct SortStats {
        int comparisons = 0;     // 比较次数
        int swaps = 0;           // 交换次数
        int partitions = 0;      // 分区次数
        int recursionDepth = 0;  // 最大递归深度
        double timeMs = 0.0;     // 执行时间（毫秒）
    };

    /**
     * 带统计信息的快速排序
     * @param arr 待排序的数组
     * @param comp 比较函数
     * @return 排序统计信息
     */
    static SortStats sortWithStats(std::vector<T>& arr, 
                                  std::function<bool(const T&, const T&)> comp = std::less<T>());

    /**
     * 基准选择策略枚举
     */
    enum class PivotStrategy {
        First,          // 选择第一个元素
        Last,           // 选择最后一个元素
        Middle,         // 选择中间元素
        Random,         // 随机选择
        MedianOfThree   // 三数取中
    };

    /**
     * 使用指定基准选择策略的快速排序
     * @param arr 待排序的数组
     * @param strategy 基准选择策略
     * @param comp 比较函数
     */
    static void sortWithPivotStrategy(std::vector<T>& arr, PivotStrategy strategy,
                                     std::function<bool(const T&, const T&)> comp = std::less<T>());

private:
    // 分区操作（Lomuto分区方案）
    static int partitionLomuto(std::vector<T>& arr, int left, int right,
                              std::function<bool(const T&, const T&)> comp);

    // 分区操作（Hoare分区方案）
    static int partitionHoare(std::vector<T>& arr, int left, int right,
                             std::function<bool(const T&, const T&)> comp);

    // 三路分区（用于三路快排）
    static std::pair<int, int> threeWayPartition(std::vector<T>& arr, int left, int right,
                                                 std::function<bool(const T&, const T&)> comp);

    // 三数取中选择基准
    static int medianOfThree(std::vector<T>& arr, int left, int right,
                            std::function<bool(const T&, const T&)> comp);

    // 交换两个元素
    static void swap(T& a, T& b);

    // 插入排序（用于小数组优化）
    static void insertionSort(std::vector<T>& arr, int left, int right,
                             std::function<bool(const T&, const T&)> comp);
};

// 便捷函数
template<typename T>
void quickSort(std::vector<T>& arr);

template<typename T>
void quickSortDescending(std::vector<T>& arr);

template<typename T>
void threeWayQuickSort(std::vector<T>& arr);

} // namespace Sorting 