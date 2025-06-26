#pragma once

#include <vector>
#include <functional>

/**
 * 插入排序 (Insertion Sort)
 * ========================
 * 
 * 算法描述：
 * 插入排序是一种简单直观的排序算法。它的工作原理是通过构建有序序列，
 * 对于未排序数据，在已排序序列中从后向前扫描，找到相应位置并插入。
 * 插入排序在实现上，通常采用in-place排序（即只需用到O(1)的额外空间的排序），
 * 因而在从后向前扫描过程中，需要反复把已排序元素逐步向后挪位，
 * 为最新元素提供插入空间。
 * 
 * 时间复杂度：
 * - 最佳情况：O(n) - 当数组已经有序时，只需要n-1次比较
 * - 平均情况：O(n²) - 随机数组，平均需要n²/4次比较和移动
 * - 最坏情况：O(n²) - 当数组逆序时，需要n²/2次比较和移动
 * 
 * 空间复杂度：
 * - O(1) - 只使用常数级别的额外空间
 * 
 * 稳定性：
 * - 稳定排序 - 相等元素的相对位置不会改变
 * 
 * 优点：
 * - 实现简单，代码简洁
 * - 对于小规模数据集非常高效
 * - 自适应性强：对于基本有序的数据集效率很高
 * - 稳定排序算法
 * - 原地排序，只需要O(1)的额外内存空间
 * - 在线算法：可以在接收数据的同时进行排序
 * 
 * 缺点：
 * - 对于大规模数据集效率较低
 * - 比选择排序多了数据移动的操作
 * - 最坏情况下时间复杂度为O(n²)
 * 
 * 适用场景：
 * - 小规模数据集（通常n < 50）
 * - 数据基本有序的情况
 * - 作为其他高级排序算法的子程序（如快速排序的优化）
 * - 在线排序：数据流式处理
 * - 对稳定性有要求的排序需求
 * - 简单排序需求，代码简洁性优于性能时
 * 
 * 算法优化：
 * - 二分插入排序：使用二分查找确定插入位置
 * - 希尔排序：插入排序的高效版本，通过间隔序列优化
 */

namespace Sorting {

/**
 * 插入排序模板类
 * 支持自定义比较函数和各种数据类型
 */
template<typename T>
class InsertionSort {
public:
    /**
     * 基础插入排序
     * @param arr 待排序的数组
     * @param comp 比较函数，默认为升序
     */
    static void sort(std::vector<T>& arr, 
                    std::function<bool(const T&, const T&)> comp = std::less<T>());

    /**
     * 二分插入排序（使用二分查找确定插入位置）
     * @param arr 待排序的数组
     * @param comp 比较函数，默认为升序
     */
    static void binaryInsertionSort(std::vector<T>& arr, 
                                   std::function<bool(const T&, const T&)> comp = std::less<T>());

    /**
     * 指定范围的插入排序（用于其他算法的子程序）
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
        int comparisons = 0;    // 比较次数
        int moves = 0;          // 移动次数
        int insertions = 0;     // 插入次数
        double timeMs = 0.0;    // 执行时间（毫秒）
    };

    /**
     * 带统计信息的插入排序
     * @param arr 待排序的数组
     * @param comp 比较函数
     * @return 排序统计信息
     */
    static SortStats sortWithStats(std::vector<T>& arr, 
                                  std::function<bool(const T&, const T&)> comp = std::less<T>());

    /**
     * 使用二分查找确定插入位置
     * @param arr 已排序的数组部分
     * @param left 搜索起始位置
     * @param right 搜索结束位置
     * @param key 要插入的值
     * @param comp 比较函数
     * @return 插入位置
     */
    static int binarySearch(const std::vector<T>& arr, int left, int right, 
                           const T& key, std::function<bool(const T&, const T&)> comp = std::less<T>());

    /**
     * 检查数组是否已经有序
     * @param arr 待检查的数组
     * @param comp 比较函数
     * @return 是否有序
     */
    static bool isSorted(const std::vector<T>& arr, 
                        std::function<bool(const T&, const T&)> comp = std::less<T>());

private:
    // 将元素向右移动一位
    static void shiftRight(std::vector<T>& arr, int pos, int count);
};

// 便捷函数
template<typename T>
void insertionSort(std::vector<T>& arr);

template<typename T>
void insertionSortDescending(std::vector<T>& arr);

template<typename T>
void binaryInsertionSort(std::vector<T>& arr);

} // namespace Sorting 