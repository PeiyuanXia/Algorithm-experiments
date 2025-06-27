#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>

// 结构体-物品信息
typedef struct {
    int weight;       // 重量
    double value;     // 价值（保留两位小数）
    double ratio;     // 价值重量比（用于贪心排序）
    int index;        // 物品索引
    bool selected;    // 是否被选中
} Item;

// 采用随机发生成物品数据并写入 Items.txt
void generate_items(Item *items, int n) {
    // 使用时间作为随机数种子
    srand(time(NULL));
    FILE *fp = fopen("Items.txt", "w");
    if (!fp) {
        perror("无法打开文件");
        return;
    }

    fprintf(fp, "物品编号%12s物品重量%12s物品价值\n", "", "");

    for (int i = 0; i < n; i++) {
        // 物品重量为1~100之间的随机整数
        items[i].weight = rand() % 100 + 1;
        // 物品价值为100~1000之间的随机数，保留两位小数
        // 先生成10000-100000的整数，再除以100.0得到两位小数的100-100以内的随机数
        items[i].value = (double)(rand() % 90001 + 10000) / 100.0;
        items[i].index = i;

        fprintf(fp, "%4d%25d%26.2f\n",
                items[i].index + 1, items[i].weight, items[i].value);
    }

    fclose(fp);
    printf("物品信息已写入 Items.txt\n");
}

// 交换物品（用于排序）
void swap(Item *a, Item *b) {
    Item temp = *a;
    *a = *b;
    *b = temp;
}

// 快速排序（按价值重量比降序）
void quick_sort(Item *items, int low, int high) {
    if (low < high) {
        int pivot = low;
        int i = low, j = high;
        while (i < j) {
            // i从左往右找比基准大的元素；j从右往左找比基准小的元素
            while (items[i].ratio >= items[pivot].ratio && i <= high) i++;
            while (items[j].ratio < items[pivot].ratio && j >= low) j--;
            if (i < j) swap(&items[i], &items[j]);
        }
        swap(&items[pivot], &items[j]);
        quick_sort(items, low, j - 1);
        quick_sort(items, j + 1, high);
    }
}

// 1. 蛮力法（仅适用于n≤20，否则难以负载）
void brute_force(Item *items, int n, int capacity, double *max_value, int *total_weight, bool *selected) {
    if (n > 20) {
        fprintf(stderr, "蛮力法计算量过大（仅适用于n≤20）\n");
        return;
    }

    *max_value = 0;
    *total_weight = 0;

    // 枚举所有2^n种可能，result代表解向量
    for (int result = 1; result < (1 << n); result++) {
        double current_value = 0;
        int current_weight = 0;
        for (int i = 0; i < n; i++) {
            // 判断第i位是否为1
            if (result & (1 << i)) {
                current_weight += items[i].weight;
                current_value += items[i].value;
            }
        }
        if (current_weight <= capacity && current_value > *max_value) {
            *max_value = current_value;
            *total_weight = current_weight;
            // 记录最优组合
            for (int i = 0; i < n; i++) {
                selected[i] = (result & (1 << i)) != 0;
            }
        }
    }
}

// 2.1 动态规划法——使用二维数组填表求解
void dynamic_programming2D(Item *items, int n, int capacity, double *max_value, int *total_weight, bool *selected) {
    // 初始化二维DP数组和路径记录数组
    // 在构建表格时多加入一行和一列，便于正确处理边界条件
    double **dp = (double **)malloc((n + 1) * sizeof(double *));   // dp[i][j]表示前i个物品、容量j的最大价值
    int **prev = (int **)malloc((n + 1) * sizeof(int *));          // prev[i][j]容量为j时，记录是否选择第i个物品（1=选，0=不选）

    for (int i = 0; i <= n; i++) {
        // calloc会在分配内存的同时初始化为0
        dp[i] = (double *)calloc(capacity + 1, sizeof(double));
        prev[i] = (int *)calloc(capacity + 1, sizeof(int));
    }

    // 填充DP表（自底向上，逐行处理物品）
    for (int i = 1; i <= n; i++) {
         // 当前处理第i个物品（i从1开始，实际对应items[i-1]）
        int weight = items[i-1].weight;
        double value = items[i-1].value;

        for (int j = 1; j <= capacity; j++) { // 遍历所有容量
            if (weight > j) {
                // 无法装入当前物品，继承上一行状态
                dp[i][j] = dp[i-1][j];
                prev[i][j] = 0; // 未选当前物品
            } else {
                // 选择当前物品或不选当前物品，取价值较大者
                double take = dp[i-1][j - weight] + value;
                double not_take = dp[i-1][j];
                if (take > not_take) {
                    dp[i][j] = take;
                    prev[i][j] = 1; // 标记为选中当前物品
                } else {
                    dp[i][j] = not_take;
                    prev[i][j] = 0; // 未选当前物品
                }
            }
        }
    }

    // 获取最大价值
    *max_value = dp[n][capacity];

    // 回溯路径，确定选中的物品（从最后一个物品开始向前推导）
    *total_weight = 0;
    // 将selected数组全部初始化为false
    memset(selected, false, n * sizeof(bool));
    int i = n, j = capacity;

    while (i > 0 && j > 0) {
        if (prev[i][j] == 1) {
            selected[i-1] = true;  // 选中了第i个物品，实际对应items[i-1]！！
            *total_weight += items[i-1].weight;
            j -= items[i-1].weight; // 容量减去当前物品重量
        }
        i--; // 处理前一个物品
    }

    // 释放内存
    for (int i = 0; i <= n; i++) {
        free(dp[i]);
        free(prev[i]);
    }
    free(dp);
    free(prev);
}

// 2.2 动态规划法——使用一维滚动数组代替二维数组
void dynamic_programming1D(Item *items, int n, int capacity, double *max_value, int *total_weight, bool *selected) {
    // 初始化一维DP数组和路径记录数组
    double *dp = (double *)calloc(capacity + 1, sizeof(double));

    bool **prev = (bool **)malloc((n + 1) * sizeof(bool *));
    for (int i = 0; i <= n; i++) {
        prev[i] = (bool *)calloc(capacity + 1, sizeof(bool));
    }

    // 填充DP表（自底向上，逐行处理物品）
    for (int i = 1; i <= n; i++) {
        int weight = items[i-1].weight;
        double value = items[i-1].value;

        // 必须是倒序遍历容量，确保每个物品只被考虑一次
        // 正序的话会出错，在计算后面的dp[j]时可能出现一个物品别放入两次
        for (int j = capacity; j >= weight; j--) {
            double take = dp[j - weight] + value;
            double not_take = dp[j];
            if (take > not_take) {
                dp[j] = take;
                prev[i][j] = true;
            }
        }
    }

    // 获取最大价值
    *max_value = dp[capacity];

    // 回溯路径，确定选中的物品
    *total_weight = 0;
    memset(selected, false, n * sizeof(bool));
    int j = capacity;

    for (int i = n; i > 0; i--) {
        if (prev[i][j] == 1) {
            selected[i-1] = true;
            *total_weight += items[i-1].weight;
            j -= items[i-1].weight;
        }
    }

    // 释放内存
    for (int i = 0; i <= n; i++) {
        free(prev[i]);
    }
    free(prev);
    free(dp);
}

// 3. 贪心法（输出近似解，非最优）
void greedy_algorithm(Item *items, int n, int capacity, double *max_value, int *total_weight, bool *selected) {
    Item *sorted_items = (Item *)malloc(n * sizeof(Item));
    memcpy(sorted_items, items, n * sizeof(Item));

    // 先按 价值/重量 进行递减排序
    for (int i = 0; i < n; i++) {
        sorted_items[i].ratio = sorted_items[i].value / sorted_items[i].weight;
    }
    quick_sort(sorted_items, 0, n - 1);

    *max_value = 0;
    *total_weight = 0;
    memset(selected, false, n * sizeof(bool));

    int remaining_capacity = capacity;
    // 0-1 背包问题只能整存，也因此贪心算法可能得不到精确解
    for (int i = 0; i < n; i++) {
        if (sorted_items[i].weight <= remaining_capacity) {
            selected[sorted_items[i].index] = true;  // index是排序之前的原索引
            *max_value += sorted_items[i].value;
            *total_weight += sorted_items[i].weight;
            remaining_capacity -= sorted_items[i].weight;
        }
    }

    free(sorted_items);
}

// 计算上界函数
double calculate_upper_bound(Item *items, int n, int capacity, int level, int current_weight, double current_value) {
    if (level >= n) return current_value; // 边界检查

    int remaining_capacity = capacity - current_weight;
    double upper_bound = current_value;

    // 从当前物品开始，按价值重量比降序计算上界
    for (int i = level; i < n; i++) {
        if (items[i].weight <= remaining_capacity) {
            upper_bound += items[i].value;
            remaining_capacity -= items[i].weight;
        } else {
            upper_bound += (items[i].value * remaining_capacity) / items[i].weight;
            break;
        }
    }

    return upper_bound;
}

// 回溯算法核心函数
void backtrack(Item *items, int n, int capacity, int level, int current_weight, double current_value, bool *selected, double *best_value, int *best_weight, bool *best_selected) {
    // 到达叶子节点或容量已用完
    if (level == n) {
        if (current_value > *best_value) {
            *best_value = current_value;
            *best_weight = current_weight;
            // 保存当前最优解的选择状态
            memcpy(best_selected, selected, n * sizeof(bool));
        }
        return;
    }

    // 计算上界，若上界小于等于当前最优解则剪枝
    double upper_bound = calculate_upper_bound(items, n, capacity, level, current_weight, current_value);
    if (upper_bound <= *best_value + 1e-9) {
        return;
    }

    // 尝试不选择当前物品
    backtrack(items, n, capacity, level + 1, current_weight, current_value,
             selected, best_value, best_weight, best_selected);

    // 尝试选择当前物品（如果可以）
    if (current_weight + items[level].weight <= capacity) {
        selected[level] = true;
        backtrack(items, n, capacity, level + 1,
                 current_weight + items[level].weight,
                 current_value + items[level].value,
                 selected, best_value, best_weight, best_selected);
        selected[level] = false; // 回溯后恢复状态
    }
}

// 回溯算法对外接口
void backtracking_algorithm(Item *items, int n, int capacity, double *max_value, int *total_weight, bool *selected) {
    // 先按价值重量比对物品排序，提高回溯效率
    Item *sorted_items = (Item *)malloc(n * sizeof(Item));
    memcpy(sorted_items, items, n * sizeof(Item));

    for (int i = 0; i < n; i++) {
        sorted_items[i].ratio = sorted_items[i].value / sorted_items[i].weight;
    }
    quick_sort(sorted_items, 0, n - 1);

    // 初始化最优解
    double best_value = 0;
    int best_weight = 0;
    bool *temp_selected = (bool *)calloc(n, sizeof(bool));
    bool *best_selected = (bool *)calloc(n, sizeof(bool)); // 保存最优选择状态

    // 执行回溯
    backtrack(sorted_items, n, capacity, 0, 0, 0, temp_selected, &best_value, &best_weight, best_selected);

    // 记录结果到原始索引
    for (int i = 0; i < n; i++) {
        if (best_selected[i]) {
            // 使用排序后数组中的原始索引
            selected[sorted_items[i].index] = true;
        }
    }

    // 返回结果
    *max_value = best_value;
    *total_weight = best_weight;

    // 释放内存
    free(temp_selected);
    free(best_selected);
    free(sorted_items);
}

// 测试单个算法——使用函数指针
void test_algorithm(void (*algorithm)(Item*, int, int, double*, int*, bool*), const char *name, Item *items, int n, int capacity) {
    clock_t start, end;
    double max_value = 0;
    int total_weight = 0;
    bool *selected = (bool *)malloc(n * sizeof(bool));
    memset(selected, false, n * sizeof(bool));

    start = clock();
    algorithm(items, n, capacity, &max_value, &total_weight, selected);
    end = clock();
    // 使用ms表示
    double time_ms = ((double)(end - start) / CLOCKS_PER_SEC) * 1000;

    printf("\n=== %s ===\n", name);
    printf("总价值: %.2f\n", max_value);
    printf("总重量: %d\n", total_weight);
    printf("执行时间: %.2f 毫秒\n", time_ms);
    printf("选中物品: ");
    for (int i = 0; i < n; i++) {
        if (selected[i]) printf("%d ", i + 1); // 物品编号从1开始
    }
    printf("\n");

    free(selected);
}

int main() {
    int n = 1000; // 测试物品数量（可修改为1000-320000）
    int capacity = 10000; // 背包容量 [10000, 100000, 1000000]
    Item *items = (Item *)malloc(n * sizeof(Item));

    generate_items(items, n); // 生成随机物品数据

    // 测试各算法
    if (n <= 20) test_algorithm(brute_force, "暴力法", items, n, capacity);
    test_algorithm(dynamic_programming2D, "使用二维数组的动态规划法", items, n, capacity);
    test_algorithm(dynamic_programming1D, "使用一维滚动数组的动态规划法", items, n, capacity);
    test_algorithm(greedy_algorithm, "贪心法", items, n, capacity);
    test_algorithm(backtracking_algorithm, "回溯法", items, n, capacity);

    free(items);
    return 0;
}
