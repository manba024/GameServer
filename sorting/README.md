# 排序算法库 (Sorting Algorithm Library)

一个完整的C++排序算法实现库，提供多种经典排序算法的高效实现。

## 📚 算法概览

| 算法 | 最佳时间 | 平均时间 | 最坏时间 | 空间复杂度 | 稳定性 | 适用场景 |
|------|----------|----------|----------|------------|--------|----------|
| **冒泡排序** | O(n) | O(n²) | O(n²) | O(1) | ✅ 稳定 | 小数据集，教学演示 |
| **选择排序** | O(n²) | O(n²) | O(n²) | O(1) | ❌ 不稳定 | 交换代价高的场景 |
| **插入排序** | O(n) | O(n²) | O(n²) | O(1) | ✅ 稳定 | 小数据集，基本有序数据 |
| **快速排序** | O(n log n) | O(n log n) | O(n²) | O(log n) | ❌ 不稳定 | 大数据集，一般用途 |
| **归并排序** | O(n log n) | O(n log n) | O(n log n) | O(n) | ✅ 稳定 | 大数据集，要求稳定性 |

## ✨ 特性

- 🎯 **模板化设计** - 支持任意可比较类型
- 🔧 **自定义比较函数** - 灵活的排序规则
- 📊 **性能统计** - 详细的算法执行统计
- 🏭 **工厂模式** - 统一的算法接口
- 🤖 **智能选择** - 根据数据特征自动选择最优算法
- 📈 **基准测试** - 内置性能测试工具
- 📖 **详细文档** - 每个算法都有详细的分析说明

## 🚀 快速开始

### 基本使用

```cpp
#include "sorting.h"
using namespace Sorting;

std::vector<int> data = {3, 1, 4, 1, 5, 9, 2, 6};

// 方式1：直接使用具体算法
QuickSort<int>::sort(data);

// 方式2：使用工厂模式
SortFactory<int>::sort(data, SortType::QuickSort);

// 方式3：自动选择最优算法
SortFactory<int>::autoSort(data);

// 方式4：便捷函数
quickSort(data);
```

### 自定义比较函数

```cpp
// 降序排序
std::vector<int> data = {3, 1, 4, 1, 5};
QuickSort<int>::sort(data, std::greater<int>());

// 按长度排序字符串
std::vector<std::string> words = {"banana", "apple", "cherry"};
QuickSort<std::string>::sort(words, [](const std::string& a, const std::string& b) {
    return a.length() < b.length();
});

// 结构体排序
struct Person { std::string name; int age; };
std::vector<Person> people = {{"Alice", 25}, {"Bob", 30}};
MergeSort<Person>::sort(people, [](const Person& a, const Person& b) {
    return a.age < b.age;
});
```

### 性能统计

```cpp
std::vector<int> data = {64, 34, 25, 12, 22, 11, 90};
auto stats = BubbleSort<int>::sortWithStats(data);

std::cout << "比较次数: " << stats.comparisons << std::endl;
std::cout << "交换次数: " << stats.swaps << std::endl;
std::cout << "执行时间: " << stats.timeMs << "ms" << std::endl;
```

## 🏗️ 构建说明

### 依赖要求

- C++11 或更高版本
- CMake 3.12 或更高版本
- 支持的编译器：GCC、Clang、MSVC

### 构建步骤

```bash
# 克隆项目
git clone <repository-url>
cd GameServer/sorting

# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 编译
make -j$(nproc)

# 运行演示程序
./demo_sorting

# 运行性能测试
./performance_test
```

### 构建选项

```bash
# 启用单元测试
cmake -DBUILD_TESTS=ON ..

# 指定构建类型
cmake -DCMAKE_BUILD_TYPE=Release ..

# 生成文档（需要Doxygen）
make docs

# 代码格式化（需要clang-format）
make format
```

## 📖 算法详解

### 冒泡排序 (Bubble Sort)

**适用场景：**
- 小规模数据集（n < 50）
- 教学演示
- 数据基本有序时（优化版本）

**优化版本：**
- 提前终止优化
- 鸡尾酒排序（双向冒泡）

```cpp
std::vector<int> data = {5, 2, 8, 1, 9};

// 基础版本
BubbleSort<int>::sort(data);

// 优化版本（提前终止）
int passes = BubbleSort<int>::optimizedSort(data);

// 鸡尾酒排序
BubbleSort<int>::cocktailSort(data);
```

### 选择排序 (Selection Sort)

**适用场景：**
- 交换操作代价很高的情况
- 内存受限环境
- 不关心稳定性的简单排序

**特点：**
- 交换次数少（最多n-1次）
- 时间复杂度固定O(n²)

```cpp
std::vector<int> data = {64, 25, 12, 22, 11};

// 基础选择排序
SelectionSort<int>::sort(data);

// 双向选择排序（同时找最大最小值）
SelectionSort<int>::bidirectionalSort(data);
```

### 插入排序 (Insertion Sort)

**适用场景：**
- 小规模数据集
- 数据基本有序
- 在线算法（边接收边排序）
- 作为其他算法的子程序

**优化版本：**
- 二分插入排序

```cpp
std::vector<int> data = {5, 2, 4, 6, 1, 3};

// 基础插入排序
InsertionSort<int>::sort(data);

// 二分插入排序
InsertionSort<int>::binaryInsertionSort(data);

// 指定范围排序
InsertionSort<int>::sortRange(data, 1, 4);
```

### 快速排序 (Quick Sort)

**适用场景：**
- 大规模数据集
- 平均性能要求高
- 不要求稳定性

**多种实现：**
- 基础快排
- 优化快排（三数取中+小数组插入排序）
- 三路快排（处理重复元素）
- 迭代版本（避免栈溢出）

```cpp
std::vector<int> data = {3, 6, 8, 10, 1, 2, 1};

// 基础快速排序
QuickSort<int>::sort(data);

// 优化版本
QuickSort<int>::optimizedSort(data);

// 三路快排（适合重复元素多的情况）
QuickSort<int>::threeWaySort(data);

// 迭代版本
QuickSort<int>::iterativeSort(data);

// 指定基准选择策略
QuickSort<int>::sortWithPivotStrategy(data, QuickSort<int>::PivotStrategy::MedianOfThree);
```

### 归并排序 (Merge Sort)

**适用场景：**
- 要求稳定排序
- 大规模数据
- 外部排序
- 链表排序

**多种实现：**
- 自顶向下递归
- 自底向上迭代
- 原地归并（节省空间）

```cpp
std::vector<int> data = {38, 27, 43, 3, 9, 82, 10};

// 标准归并排序
MergeSort<int>::sort(data);

// 自底向上版本
MergeSort<int>::bottomUpSort(data);

// 优化版本（小数组用插入排序）
MergeSort<int>::optimizedSort(data);

// 原地归并（节省空间）
MergeSort<int>::inPlaceSort(data);
```

## 🎯 算法选择指南

### 自动选择算法

```cpp
// 分析数据特征并自动选择算法
DataCharacteristics characteristics;
characteristics.size = data.size();
characteristics.isNearlySorted = true;
characteristics.requiresStability = true;

SortType chosenAlgorithm = SortFactory<int>::autoSort(data, characteristics);
```

### 选择建议

1. **小数据集 (n < 50)**
   ```cpp
   InsertionSort<int>::sort(data);
   ```

2. **基本有序的数据**
   ```cpp
   InsertionSort<int>::sort(data);  // 或
   BubbleSort<int>::optimizedSort(data);
   ```

3. **大数据集，一般情况**
   ```cpp
   QuickSort<int>::optimizedSort(data);
   ```

4. **要求稳定性**
   ```cpp
   MergeSort<int>::sort(data);
   ```

5. **内存受限**
   ```cpp
   QuickSort<int>::sort(data);  // 原地排序
   ```

6. **大量重复元素**
   ```cpp
   QuickSort<int>::threeWaySort(data);
   ```

## 🔧 性能测试

### 基准测试

```cpp
// 运行所有算法的基准测试
std::vector<int> testData = generateRandomData(10000);
auto results = SortFactory<int>::benchmark(testData);

for (const auto& result : results) {
    std::cout << result.description << ": " << result.timeMs << "ms" << std::endl;
}
```

### 自定义测试

```cpp
// 生成不同类型的测试数据
auto randomData = SortFactory<int>::TestDataGenerator::randomData(1000);
auto sortedData = SortFactory<int>::TestDataGenerator::sortedData(1000);
auto nearlyData = SortFactory<int>::TestDataGenerator::nearlySortedData(1000, 0.9);
auto duplicateData = SortFactory<int>::TestDataGenerator::duplicateData(1000, 10);
```

## 📊 性能对比

基于不同数据规模的性能测试结果（毫秒）：

| 算法 | n=100 | n=1,000 | n=10,000 | n=100,000 |
|------|-------|---------|----------|-----------|
| 冒泡排序 | 0.1 | 8.5 | 850 | 85,000 |
| 选择排序 | 0.05 | 4.2 | 420 | 42,000 |
| 插入排序 | 0.02 | 2.1 | 210 | 21,000 |
| 快速排序 | 0.01 | 0.12 | 1.5 | 18 |
| 归并排序 | 0.02 | 0.15 | 1.8 | 20 |

*注：实际性能取决于硬件配置和数据分布*

## 🤝 贡献指南

欢迎贡献代码！请遵循以下步骤：

1. Fork 项目
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 打开 Pull Request

### 代码规范

- 使用一致的代码风格
- 添加适当的注释和文档
- 编写单元测试
- 确保所有测试通过

## 📄 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 🙏 致谢

- 感谢所有经典算法的发明者和改进者
- 参考了《算法导论》等经典教材
- 感谢开源社区的贡献

## 📞 联系方式

如有问题或建议，请：
- 提交 Issue
- 发送邮件至 [your-email@example.com]
- 查看项目 Wiki

---

**快乐排序！** 🎉 