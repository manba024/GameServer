#include "../include/sorting.h"
#include <iostream>
#include <vector>
#include <random>
#include <iomanip>
#include <algorithm>
#include <string>
#include <chrono>

using namespace Sorting;

// 打印数组
template<typename T>
void printArray(const std::vector<T>& arr, const std::string& title = "") {
    if (!title.empty()) {
        std::cout << title << ": ";
    }
    
    if (arr.size() <= 20) {
        std::cout << "[";
        for (size_t i = 0; i < arr.size(); i++) {
            std::cout << arr[i];
            if (i < arr.size() - 1) std::cout << ", ";
        }
        std::cout << "]";
    } else {
        std::cout << "[" << arr[0] << ", " << arr[1] << ", ..., " 
                  << arr[arr.size()-2] << ", " << arr[arr.size()-1] << "]";
        std::cout << " (size: " << arr.size() << ")";
    }
    std::cout << std::endl;
}

// 生成测试数据
std::vector<int> generateRandomData(size_t size, int minVal = 1, int maxVal = 1000) {
    std::vector<int> data;
    data.reserve(size);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(minVal, maxVal);
    
    for (size_t i = 0; i < size; i++) {
        data.push_back(dis(gen));
    }
    
    return data;
}

std::vector<int> generateNearlySortedData(size_t size) {
    std::vector<int> data;
    for (size_t i = 0; i < size; i++) {
        data.push_back(static_cast<int>(i));
    }
    
    // 随机交换一些元素
    std::random_device rd;
    std::mt19937 gen(rd());
    size_t swapCount = size / 10; // 10%的元素被打乱
    
    for (size_t i = 0; i < swapCount; i++) {
        size_t idx1 = gen() % size;
        size_t idx2 = gen() % size;
        std::swap(data[idx1], data[idx2]);
    }
    
    return data;
}

// 基础排序演示
void demonstrateBasicSorting() {
    std::cout << "\n=== 基础排序算法演示 ===\n";
    
    std::vector<int> originalData = {64, 34, 25, 12, 22, 11, 90, 88, 76, 50, 42};
    printArray(originalData, "原始数据");
    
    // 冒泡排序
    {
        auto data = originalData;
        std::cout << "\n1. 冒泡排序:\n";
        auto stats = BubbleSort<int>::sortWithStats(data);
        printArray(data, "排序结果");
        std::cout << "统计: 比较" << stats.comparisons << "次, 交换" << stats.swaps 
                  << "次, 用时" << std::fixed << std::setprecision(3) << stats.timeMs << "ms\n";
    }
    
    // 选择排序
    {
        auto data = originalData;
        std::cout << "\n2. 选择排序:\n";
        SelectionSort<int>::sort(data);
        printArray(data, "排序结果");
    }
    
    // 插入排序
    {
        auto data = originalData;
        std::cout << "\n3. 插入排序:\n";
        InsertionSort<int>::sort(data);
        printArray(data, "排序结果");
    }
    
    // 快速排序
    {
        auto data = originalData;
        std::cout << "\n4. 快速排序:\n";
        QuickSort<int>::sort(data);
        printArray(data, "排序结果");
    }
    
    // 归并排序
    {
        auto data = originalData;
        std::cout << "\n5. 归并排序:\n";
        MergeSort<int>::sort(data);
        printArray(data, "排序结果");
    }
}

// 性能对比演示
void demonstratePerformanceComparison() {
    std::cout << "\n=== 性能对比演示 ===\n";
    
    std::vector<size_t> sizes = {100, 1000, 5000};
    
    for (size_t size : sizes) {
        std::cout << "\n数据规模: " << size << "\n";
        std::cout << std::string(50, '-') << "\n";
        
        auto testData = generateRandomData(size);
        
        // 测试各种算法
        struct TestResult {
            std::string name;
            double timeMs;
            bool completed;
        };
        
        std::vector<TestResult> results;
        
        // 冒泡排序（只在小数据集上测试）
        if (size <= 1000) {
            auto data = testData;
            auto start = std::chrono::high_resolution_clock::now();
            BubbleSort<int>::sort(data);
            auto end = std::chrono::high_resolution_clock::now();
            double time = std::chrono::duration<double, std::milli>(end - start).count();
            results.push_back({"冒泡排序", time, true});
        } else {
            results.push_back({"冒泡排序", 0, false});
        }
        
        // 选择排序（只在小数据集上测试）
        if (size <= 1000) {
            auto data = testData;
            auto start = std::chrono::high_resolution_clock::now();
            SelectionSort<int>::sort(data);
            auto end = std::chrono::high_resolution_clock::now();
            double time = std::chrono::duration<double, std::milli>(end - start).count();
            results.push_back({"选择排序", time, true});
        } else {
            results.push_back({"选择排序", 0, false});
        }
        
        // 插入排序
        {
            auto data = testData;
            auto start = std::chrono::high_resolution_clock::now();
            InsertionSort<int>::sort(data);
            auto end = std::chrono::high_resolution_clock::now();
            double time = std::chrono::duration<double, std::milli>(end - start).count();
            results.push_back({"插入排序", time, true});
        }
        
        // 快速排序
        {
            auto data = testData;
            auto start = std::chrono::high_resolution_clock::now();
            QuickSort<int>::sort(data);
            auto end = std::chrono::high_resolution_clock::now();
            double time = std::chrono::duration<double, std::milli>(end - start).count();
            results.push_back({"快速排序", time, true});
        }
        
        // 归并排序
        {
            auto data = testData;
            auto start = std::chrono::high_resolution_clock::now();
            MergeSort<int>::sort(data);
            auto end = std::chrono::high_resolution_clock::now();
            double time = std::chrono::duration<double, std::milli>(end - start).count();
            results.push_back({"归并排序", time, true});
        }
        
        // 打印结果
        std::cout << std::left << std::setw(15) << "算法" << std::setw(15) << "时间(ms)" << "状态\n";
        for (const auto& result : results) {
            std::cout << std::setw(15) << result.name;
            if (result.completed) {
                std::cout << std::setw(15) << std::fixed << std::setprecision(3) << result.timeMs;
                std::cout << "✅ 完成";
            } else {
                std::cout << std::setw(15) << "N/A";
                std::cout << "⏭️ 跳过（数据量太大）";
            }
            std::cout << "\n";
        }
    }
}

// 特殊情况演示
void demonstrateSpecialCases() {
    std::cout << "\n=== 特殊情况演示 ===\n";
    
    // 已排序数据
    std::cout << "\n1. 已排序数据:\n";
    std::vector<int> sortedData = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    printArray(sortedData, "原始数据");
    
    {
        auto data = sortedData;
        auto start = std::chrono::high_resolution_clock::now();
        BubbleSort<int>::optimizedSort(data);
        auto end = std::chrono::high_resolution_clock::now();
        double time = std::chrono::duration<double, std::milli>(end - start).count();
        std::cout << "优化冒泡排序用时: " << std::fixed << std::setprecision(3) << time << "ms\n";
    }
    
    {
        auto data = sortedData;
        auto start = std::chrono::high_resolution_clock::now();
        InsertionSort<int>::sort(data);
        auto end = std::chrono::high_resolution_clock::now();
        double time = std::chrono::duration<double, std::milli>(end - start).count();
        std::cout << "插入排序用时: " << std::fixed << std::setprecision(3) << time << "ms\n";
    }
    
    // 逆序数据
    std::cout << "\n2. 逆序数据:\n";
    std::vector<int> reverseData = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    printArray(reverseData, "原始数据");
    
    {
        auto data = reverseData;
        auto start = std::chrono::high_resolution_clock::now();
        QuickSort<int>::sort(data);
        auto end = std::chrono::high_resolution_clock::now();
        double time = std::chrono::duration<double, std::milli>(end - start).count();
        printArray(data, "快速排序结果");
        std::cout << "快速排序用时: " << std::fixed << std::setprecision(3) << time << "ms\n";
    }
    
    // 大量重复元素
    std::cout << "\n3. 大量重复元素:\n";
    std::vector<int> duplicateData = {5, 3, 5, 3, 5, 3, 5, 3, 5, 3};
    printArray(duplicateData, "原始数据");
    
    {
        auto data = duplicateData;
        auto start = std::chrono::high_resolution_clock::now();
        QuickSort<int>::threeWaySort(data);
        auto end = std::chrono::high_resolution_clock::now();
        double time = std::chrono::duration<double, std::milli>(end - start).count();
        printArray(data, "三路快排结果");
        std::cout << "三路快排用时: " << std::fixed << std::setprecision(3) << time << "ms\n";
    }
}

// 自定义比较函数演示
void demonstrateCustomComparator() {
    std::cout << "\n=== 自定义比较函数演示 ===\n";
    
    // 字符串排序
    std::vector<std::string> words = {"banana", "apple", "cherry", "date", "elderberry"};
    printArray(words, "原始字符串");
    
    // 按长度排序
    auto data1 = words;
    QuickSort<std::string>::sort(data1, [](const std::string& a, const std::string& b) {
        return a.length() < b.length();
    });
    printArray(data1, "按长度排序");
    
    // 降序排序
    auto data2 = words;
    QuickSort<std::string>::sort(data2, std::greater<std::string>());
    printArray(data2, "降序排序");
    
    // 结构体排序
    struct Person {
        std::string name;
        int age;
    };
    
    // 为Person定义输出操作符
    auto printPerson = [](const Person& p) {
        return p.name + "(" + std::to_string(p.age) + ")";
    };
    
    std::vector<Person> people = {
        {"Alice", 25}, {"Bob", 30}, {"Charlie", 20}, {"Diana", 35}
    };
    
    std::cout << "\n结构体排序:\n";
    std::cout << "原始数据: ";
    for (const auto& p : people) {
        std::cout << printPerson(p) << " ";
    }
    std::cout << "\n";
    
    // 按年龄排序
    std::sort(people.begin(), people.end(), [](const Person& a, const Person& b) {
        return a.age < b.age;
    });
    
    std::cout << "按年龄排序: ";
    for (const auto& p : people) {
        std::cout << printPerson(p) << " ";
    }
    std::cout << "\n";
}

// 算法选择建议
void demonstrateAlgorithmRecommendation() {
    std::cout << "\n=== 算法选择建议 ===\n";
    
    std::cout << "根据数据特征选择最优算法:\n\n";
    
    // 小数据集
    std::cout << "1. 小数据集 (n < 50):\n";
    std::cout << "   推荐: 插入排序\n";
    std::cout << "   原因: 实现简单，常数因子小，对小数据集效率高\n\n";
    
    // 基本有序
    std::cout << "2. 基本有序的数据:\n";
    std::cout << "   推荐: 插入排序 或 优化冒泡排序\n";
    std::cout << "   原因: 这些算法对有序数据有很好的自适应性\n\n";
    
    // 大数据集，一般情况
    std::cout << "3. 大数据集，一般情况:\n";
    std::cout << "   推荐: 优化快速排序\n";
    std::cout << "   原因: 平均性能优秀，实际应用中表现最好\n\n";
    
    // 要求稳定性
    std::cout << "4. 要求稳定性:\n";
    std::cout << "   推荐: 归并排序\n";
    std::cout << "   原因: 稳定排序，时间复杂度稳定\n\n";
    
    // 内存受限
    std::cout << "5. 内存受限:\n";
    std::cout << "   推荐: 快速排序 或 堆排序\n";
    std::cout << "   原因: 原地排序，空间复杂度低\n\n";
    
    // 大量重复元素
    std::cout << "6. 大量重复元素:\n";
    std::cout << "   推荐: 三路快速排序\n";
    std::cout << "   原因: 对重复元素有特殊优化\n\n";
}

int main() {
    std::cout << "排序算法库演示程序\n";
    std::cout << "==================\n";
    std::cout << getLibraryInfo() << "\n";
    
    try {
        // 基础演示
        demonstrateBasicSorting();
        
        // 性能对比
        demonstratePerformanceComparison();
        
        // 特殊情况
        demonstrateSpecialCases();
        
        // 自定义比较函数
        demonstrateCustomComparator();
        
        // 算法选择建议
        demonstrateAlgorithmRecommendation();
        
        std::cout << "\n=== 演示完成 ===\n";
        std::cout << "💡 提示: 在实际应用中，建议根据数据特征选择合适的算法\n";
        std::cout << "📚 更多信息请参考各算法头文件中的详细说明\n";
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 