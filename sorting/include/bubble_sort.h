#pragma once

#include <vector>
#include <functional>

/**
 * 冒泡排序 (Bubble Sort)
 * =====================
 * 
 * 算法描述：
 * 冒泡排序是一种简单的排序算法，通过重复地遍历要排序的数列，
 * 比较相邻的两个元素，如果它们的顺序错误就把它们交换过来。
 * 遍历数列的工作是重复地进行直到没有再需要交换，也就是说该数列已经排序完成。
 * 这个算法的名字由来是因为越小的元素会经由交换慢慢"浮"到数列的顶端。
 * 
 * 时间复杂度：
 * - 最佳情况：O(n) - 当数组已经有序时，只需要一次遍历
 * - 平均情况：O(n²) - 随机数组
 * - 最坏情况：O(n²) - 当数组逆序时
 * 
 * 空间复杂度：
 * - O(1) - 只使用常数级别的额外空间
 * 
 * 稳定性：
 * - 稳定排序 - 相等元素的相对位置不会改变
 * 
 * 优点：
 * - 实现简单，代码易懂
 * - 原地排序，不需要额外的存储空间
 * - 稳定排序算法
 * - 可以检测输入的列表是否已经排序
 * 
 * 缺点：
 * - 时间复杂度较高，对于大数据集效率低下
 * - 即使在最好的情况下也需要O(n)的时间
 * - 比较次数多，交换次数多
 * 
 * 适用场景：
 * - 小规模数据集（通常n < 50）
 * - 教学演示，帮助理解排序算法基本概念
 * - 作为其他复杂算法的一部分
 * - 当数据基本有序时，可以提前终止
 * - 对稳定性有要求的简单排序需求
 * 
 * 算法改进：
 * - 添加提前终止条件：如果某次遍历没有发生交换，说明已经有序
 * - 鸡尾酒排序：双向冒泡，从两端同时进行
 */

namespace Sorting {

/**
 * 冒泡排序模板类
 * 支持自定义比较函数和各种数据类型
 */
template<typename T>
class BubbleSort {
public:
    /**
     * 基础冒泡排序
     * @param arr 待排序的数组
     * @param comp 比较函数，默认为升序
     */
    static void sort(std::vector<T>& arr, 
                    std::function<bool(const T&, const T&)> comp = std::less<T>());

    /**
     * 优化版冒泡排序（提前终止）
     * @param arr 待排序的数组
     * @param comp 比较函数，默认为升序
     * @return 实际执行的轮数
     */
    static int optimizedSort(std::vector<T>& arr, 
                           std::function<bool(const T&, const T&)> comp = std::less<T>());

    /**
     * 鸡尾酒排序（双向冒泡排序）
     * @param arr 待排序的数组
     * @param comp 比较函数，默认为升序
     */
    static void cocktailSort(std::vector<T>& arr, 
                           std::function<bool(const T&, const T&)> comp = std::less<T>());

    /**
     * 获取排序过程的详细统计信息
     */
    struct SortStats {
        int comparisons = 0;    // 比较次数
        int swaps = 0;          // 交换次数
        int passes = 0;         // 遍历轮数
        double timeMs = 0.0;    // 执行时间（毫秒）
    };

    /**
     * 带统计信息的冒泡排序
     * @param arr 待排序的数组
     * @param comp 比较函数
     * @return 排序统计信息
     */
    static SortStats sortWithStats(std::vector<T>& arr, 
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
void bubbleSort(std::vector<T>& arr);

template<typename T>
void bubbleSortDescending(std::vector<T>& arr);

} // namespace Sorting 