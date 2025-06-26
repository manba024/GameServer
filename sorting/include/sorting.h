#pragma once

/**
 * 排序算法库 - 总头文件
 * =====================
 * 
 * 这个库提供了多种经典排序算法的实现，包括：
 * - 冒泡排序 (Bubble Sort)
 * - 选择排序 (Selection Sort)  
 * - 插入排序 (Insertion Sort)
 * - 快速排序 (Quick Sort)
 * - 归并排序 (Merge Sort)
 * 
 * 特性：
 * - 模板化设计，支持任意可比较类型
 * - 自定义比较函数支持
 * - 详细的算法分析和复杂度说明
 * - 多种算法变种和优化
 * - 统一的工厂接口
 * - 性能基准测试工具
 * - 自动算法选择
 * 
 * 使用示例：
 * ```cpp
 * #include "sorting.h"
 * using namespace Sorting;
 * 
 * std::vector<int> data = {3, 1, 4, 1, 5, 9, 2, 6};
 * 
 * // 方式1：直接使用具体算法
 * QuickSort<int>::sort(data);
 * 
 * // 方式2：使用工厂模式
 * SortFactory<int>::sort(data, SortType::QuickSort);
 * 
 * // 方式3：自动选择最优算法
 * SortFactory<int>::autoSort(data);
 * 
 * // 方式4：便捷函数
 * quickSort(data);
 * ```
 */

// 包含所有排序算法头文件
#include "bubble_sort.h"
#include "selection_sort.h"
#include "insertion_sort.h"
#include "quick_sort.h"
#include "merge_sort.h"
#include "sort_factory.h"

/**
 * 排序算法复杂度对比表
 * ===================
 * 
 * | 算法     | 最佳时间 | 平均时间 | 最坏时间 | 空间复杂度 | 稳定性 | 适用场景 |
 * |----------|----------|----------|----------|------------|--------|----------|
 * | 冒泡排序 | O(n)     | O(n²)    | O(n²)    | O(1)       | 稳定   | 小数据集，教学 |
 * | 选择排序 | O(n²)    | O(n²)    | O(n²)    | O(1)       | 不稳定 | 交换代价高 |
 * | 插入排序 | O(n)     | O(n²)    | O(n²)    | O(1)       | 稳定   | 小数据集，基本有序 |
 * | 快速排序 | O(n logn)| O(n logn)| O(n²)    | O(logn)    | 不稳定 | 大数据集，一般用途 |
 * | 归并排序 | O(n logn)| O(n logn)| O(n logn)| O(n)       | 稳定   | 大数据集，要求稳定 |
 * 
 * 算法选择建议：
 * - n < 50: 插入排序
 * - n < 1000 且基本有序: 插入排序
 * - 要求稳定性: 归并排序
 * - 内存受限: 快速排序或堆排序
 * - 一般情况: 快速排序（优化版）
 * - 大量重复元素: 三路快速排序
 */

namespace Sorting {

/**
 * 库版本信息
 */
constexpr const char* VERSION = "1.0.0";
constexpr const char* BUILD_DATE = __DATE__;

/**
 * 获取库信息
 */
inline std::string getLibraryInfo() {
    return std::string("Sorting Library v") + VERSION + 
           " (Built on " + BUILD_DATE + ")";
}

/**
 * 打印算法对比表
 */
void printAlgorithmComparison();

/**
 * 运行所有算法的基准测试
 */
template<typename T>
void runFullBenchmark(const std::vector<T>& testData);

} // namespace Sorting 