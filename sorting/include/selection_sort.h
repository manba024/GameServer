#pragma once

#include <vector>
#include <functional>

/**
 * 选择排序 (Selection Sort)
 * ========================
 * 
 * 算法描述：
 * 选择排序是一种简单直观的排序算法。它的工作原理是每次从待排序的数据元素中
 * 选出最小（或最大）的一个元素，存放在序列的起始位置，然后再从剩余的未排序
 * 元素中继续寻找最小（或最大）元素，然后放到已排序序列的末尾。
 * 以此类推，直到全部待排序的数据元素的个数为零。
 * 
 * 时间复杂度：
 * - 最佳情况：O(n²) - 即使数组已经有序，仍需要进行n²次比较
 * - 平均情况：O(n²) - 随机数组
 * - 最坏情况：O(n²) - 任何情况下都是O(n²)
 * 
 * 空间复杂度：
 * - O(1) - 只使用常数级别的额外空间
 * 
 * 稳定性：
 * - 不稳定排序 - 相等元素的相对位置可能会改变
 * - 例如：[5, 8, 5, 2, 9] 排序后第一个5会和2交换位置
 * 
 * 优点：
 * - 实现简单，代码简洁
 * - 原地排序，不需要额外的存储空间
 * - 交换次数少，每轮只交换一次（最多n-1次交换）
 * - 对于小规模数据集性能可接受
 * - 不受输入数据初始状态影响，性能稳定
 * 
 * 缺点：
 * - 时间复杂度固定为O(n²)，无法优化
 * - 不是稳定排序算法
 * - 比较次数多，即使数据已经有序也要全部比较
 * - 对于大数据集效率低下
 * 
 * 适用场景：
 * - 小规模数据集（通常n < 100）
 * - 内存空间受限的环境
 * - 不关心稳定性的简单排序需求
 * - 交换操作代价很高的情况（因为交换次数少）
 * - 教学演示，理解排序算法的基本思想
 * 
 * 算法变种：
 * - 双向选择排序：同时找最大值和最小值
 * - 堆排序：选择排序的高效版本，使用堆来快速找到最值
 */

namespace Sorting {

/**
 * 选择排序模板类
 * 支持自定义比较函数和各种数据类型
 */
template<typename T>
class SelectionSort {
public:
    /**
     * 基础选择排序
     * @param arr 待排序的数组
     * @param comp 比较函数，默认为升序
     */
    static void sort(std::vector<T>& arr, 
                    std::function<bool(const T&, const T&)> comp = std::less<T>());

    /**
     * 双向选择排序（同时选择最大值和最小值）
     * @param arr 待排序的数组
     * @param comp 比较函数，默认为升序
     */
    static void bidirectionalSort(std::vector<T>& arr, 
                                 std::function<bool(const T&, const T&)> comp = std::less<T>());

    /**
     * 获取排序过程的详细统计信息
     */
    struct SortStats {
        int comparisons = 0;    // 比较次数
        int swaps = 0;          // 交换次数
        int selections = 0;     // 选择次数
        double timeMs = 0.0;    // 执行时间（毫秒）
    };

    /**
     * 带统计信息的选择排序
     * @param arr 待排序的数组
     * @param comp 比较函数
     * @return 排序统计信息
     */
    static SortStats sortWithStats(std::vector<T>& arr, 
                                  std::function<bool(const T&, const T&)> comp = std::less<T>());

    /**
     * 找到数组中指定范围内的最值索引
     * @param arr 数组
     * @param start 起始索引
     * @param end 结束索引
     * @param comp 比较函数
     * @return 最值元素的索引
     */
    static int findExtremumIndex(const std::vector<T>& arr, int start, int end,
                                std::function<bool(const T&, const T&)> comp = std::less<T>());

    /**
     * 检查数组是否已经有序
     * @param arr 待检查的数组
     * @param comp 比较函数
     * @return 是否有序
     */
    static bool isSorted(const std::vector<T>& arr, 
                        std::function<bool(const T&, const T&)> comp = std::less<T>());

private:
    // 交换两个元素
    static void swap(T& a, T& b);
};

// 便捷函数
template<typename T>
void selectionSort(std::vector<T>& arr);

template<typename T>
void selectionSortDescending(std::vector<T>& arr);

} // namespace Sorting 