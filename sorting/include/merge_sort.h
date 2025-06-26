#pragma once

#include <vector>
#include <functional>

/**
 * 归并排序 (Merge Sort)
 * ====================
 * 
 * 算法描述：
 * 归并排序是建立在归并操作上的一种有效的排序算法。该算法是采用分治法
 * (Divide and Conquer)的一个非常典型的应用。将已有序的子序列合并，
 * 得到完全有序的序列；即先使每个子序列有序，再使子序列段间有序。
 * 若将两个有序表合并成一个有序表，称为2-路归并。
 * 
 * 时间复杂度：
 * - 最佳情况：O(n log n) - 无论什么情况都是O(n log n)
 * - 平均情况：O(n log n) - 稳定的时间复杂度
 * - 最坏情况：O(n log n) - 时间复杂度稳定，不受输入数据影响
 * 
 * 空间复杂度：
 * - O(n) - 需要额外的数组空间来存储临时数据
 * 
 * 稳定性：
 * - 稳定排序 - 相等元素的相对位置不会改变
 * 
 * 优点：
 * - 时间复杂度稳定：始终为O(n log n)
 * - 稳定排序算法
 * - 适合外部排序（处理大文件）
 * - 可以很好地利用多核处理器并行化
 * - 对于链表排序特别有效
 * - 性能不受输入数据分布影响
 * 
 * 缺点：
 * - 需要额外的O(n)空间
 * - 不是原地排序算法
 * - 对于小数组，常数因子较大，效率不如插入排序
 * - 即使数组已经有序，仍需要完整的分治过程
 * 
 * 适用场景：
 * - 大规模数据排序
 * - 要求稳定排序的场景
 * - 外部排序（文件排序）
 * - 链表排序
 * - 并行排序实现
 * - 对时间复杂度稳定性要求高的场景
 * - 数据分布未知的情况
 * 
 * 算法优化：
 * - 小数组使用插入排序
 * - 自底向上的迭代实现
 * - 原地归并排序（复杂但节省空间）
 * - 多路归并排序
 * - 并行归并排序
 */

namespace Sorting {

/**
 * 归并排序模板类
 * 支持自定义比较函数和各种数据类型
 */
template<typename T>
class MergeSort {
public:
    /**
     * 基础归并排序（自顶向下递归实现）
     * @param arr 待排序的数组
     * @param comp 比较函数，默认为升序
     */
    static void sort(std::vector<T>& arr, 
                    std::function<bool(const T&, const T&)> comp = std::less<T>());

    /**
     * 自底向上的归并排序（迭代实现）
     * @param arr 待排序的数组
     * @param comp 比较函数，默认为升序
     */
    static void bottomUpSort(std::vector<T>& arr, 
                            std::function<bool(const T&, const T&)> comp = std::less<T>());

    /**
     * 优化版归并排序（小数组使用插入排序）
     * @param arr 待排序的数组
     * @param comp 比较函数，默认为升序
     */
    static void optimizedSort(std::vector<T>& arr, 
                             std::function<bool(const T&, const T&)> comp = std::less<T>());

    /**
     * 原地归并排序（节省空间但时间复杂度增加）
     * @param arr 待排序的数组
     * @param comp 比较函数，默认为升序
     */
    static void inPlaceSort(std::vector<T>& arr, 
                           std::function<bool(const T&, const T&)> comp = std::less<T>());

    /**
     * 指定范围的归并排序
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
        int merges = 0;          // 归并次数
        int arrayAccesses = 0;   // 数组访问次数
        int recursionDepth = 0;  // 最大递归深度
        double timeMs = 0.0;     // 执行时间（毫秒）
    };

    /**
     * 带统计信息的归并排序
     * @param arr 待排序的数组
     * @param comp 比较函数
     * @return 排序统计信息
     */
    static SortStats sortWithStats(std::vector<T>& arr, 
                                  std::function<bool(const T&, const T&)> comp = std::less<T>());

    /**
     * 多路归并排序（k路归并）
     * @param arr 待排序的数组
     * @param k 归并路数
     * @param comp 比较函数
     */
    static void kWayMergeSort(std::vector<T>& arr, int k = 4,
                             std::function<bool(const T&, const T&)> comp = std::less<T>());

    /**
     * 检查数组是否已经有序
     * @param arr 待检查的数组
     * @param comp 比较函数
     * @return 是否有序
     */
    static bool isSorted(const std::vector<T>& arr, 
                        std::function<bool(const T&, const T&)> comp = std::less<T>());

    /**
     * 归并两个已排序的数组
     * @param left 左数组
     * @param right 右数组
     * @param comp 比较函数
     * @return 归并后的数组
     */
    static std::vector<T> mergeTwoSortedArrays(const std::vector<T>& left, 
                                              const std::vector<T>& right,
                                              std::function<bool(const T&, const T&)> comp = std::less<T>());

private:
    // 递归归并排序的主函数
    static void mergeSortRecursive(std::vector<T>& arr, std::vector<T>& temp, 
                                  int left, int right,
                                  std::function<bool(const T&, const T&)> comp);

    // 归并操作
    static void merge(std::vector<T>& arr, std::vector<T>& temp, 
                     int left, int mid, int right,
                     std::function<bool(const T&, const T&)> comp);

    // 原地归并操作（复杂但节省空间）
    static void inPlaceMerge(std::vector<T>& arr, int left, int mid, int right,
                            std::function<bool(const T&, const T&)> comp);

    // 插入排序（用于小数组优化）
    static void insertionSort(std::vector<T>& arr, int left, int right,
                             std::function<bool(const T&, const T&)> comp);

    // 多路归并的辅助函数
    static void kWayMerge(std::vector<std::vector<T>>& arrays, std::vector<T>& result,
                         std::function<bool(const T&, const T&)> comp);
};

// 便捷函数
template<typename T>
void mergeSort(std::vector<T>& arr);

template<typename T>
void mergeSortDescending(std::vector<T>& arr);

template<typename T>
void bottomUpMergeSort(std::vector<T>& arr);

} // namespace Sorting 