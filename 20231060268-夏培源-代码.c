#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>

// �ṹ��-��Ʒ��Ϣ
typedef struct {
    int weight;       // ����
    double value;     // ��ֵ��������λС����
    double ratio;     // ��ֵ�����ȣ�����̰������
    int index;        // ��Ʒ����
    bool selected;    // �Ƿ�ѡ��
} Item;

// ���������������Ʒ���ݲ�д�� Items.txt
void generate_items(Item *items, int n) {
    // ʹ��ʱ����Ϊ���������
    srand(time(NULL));
    FILE *fp = fopen("Items.txt", "w");
    if (!fp) {
        perror("�޷����ļ�");
        return;
    }

    fprintf(fp, "��Ʒ���%12s��Ʒ����%12s��Ʒ��ֵ\n", "", "");

    for (int i = 0; i < n; i++) {
        // ��Ʒ����Ϊ1~100֮����������
        items[i].weight = rand() % 100 + 1;
        // ��Ʒ��ֵΪ100~1000֮����������������λС��
        // ������10000-100000���������ٳ���100.0�õ���λС����100-100���ڵ������
        items[i].value = (double)(rand() % 90001 + 10000) / 100.0;
        items[i].index = i;

        fprintf(fp, "%4d%25d%26.2f\n",
                items[i].index + 1, items[i].weight, items[i].value);
    }

    fclose(fp);
    printf("��Ʒ��Ϣ��д�� Items.txt\n");
}

// ������Ʒ����������
void swap(Item *a, Item *b) {
    Item temp = *a;
    *a = *b;
    *b = temp;
}

// �������򣨰���ֵ�����Ƚ���
void quick_sort(Item *items, int low, int high) {
    if (low < high) {
        int pivot = low;
        int i = low, j = high;
        while (i < j) {
            // i���������ұȻ�׼���Ԫ�أ�j���������ұȻ�׼С��Ԫ��
            while (items[i].ratio >= items[pivot].ratio && i <= high) i++;
            while (items[j].ratio < items[pivot].ratio && j >= low) j--;
            if (i < j) swap(&items[i], &items[j]);
        }
        swap(&items[pivot], &items[j]);
        quick_sort(items, low, j - 1);
        quick_sort(items, j + 1, high);
    }
}

// 1. ����������������n��20���������Ը��أ�
void brute_force(Item *items, int n, int capacity, double *max_value, int *total_weight, bool *selected) {
    if (n > 20) {
        fprintf(stderr, "���������������󣨽�������n��20��\n");
        return;
    }

    *max_value = 0;
    *total_weight = 0;

    // ö������2^n�ֿ��ܣ�result���������
    for (int result = 1; result < (1 << n); result++) {
        double current_value = 0;
        int current_weight = 0;
        for (int i = 0; i < n; i++) {
            // �жϵ�iλ�Ƿ�Ϊ1
            if (result & (1 << i)) {
                current_weight += items[i].weight;
                current_value += items[i].value;
            }
        }
        if (current_weight <= capacity && current_value > *max_value) {
            *max_value = current_value;
            *total_weight = current_weight;
            // ��¼�������
            for (int i = 0; i < n; i++) {
                selected[i] = (result & (1 << i)) != 0;
            }
        }
    }
}

// 2.1 ��̬�滮������ʹ�ö�ά����������
void dynamic_programming2D(Item *items, int n, int capacity, double *max_value, int *total_weight, bool *selected) {
    // ��ʼ����άDP�����·����¼����
    // �ڹ������ʱ�����һ�к�һ�У�������ȷ����߽�����
    double **dp = (double **)malloc((n + 1) * sizeof(double *));   // dp[i][j]��ʾǰi����Ʒ������j������ֵ
    int **prev = (int **)malloc((n + 1) * sizeof(int *));          // prev[i][j]����Ϊjʱ����¼�Ƿ�ѡ���i����Ʒ��1=ѡ��0=��ѡ��

    for (int i = 0; i <= n; i++) {
        // calloc���ڷ����ڴ��ͬʱ��ʼ��Ϊ0
        dp[i] = (double *)calloc(capacity + 1, sizeof(double));
        prev[i] = (int *)calloc(capacity + 1, sizeof(int));
    }

    // ���DP���Ե����ϣ����д�����Ʒ��
    for (int i = 1; i <= n; i++) {
         // ��ǰ�����i����Ʒ��i��1��ʼ��ʵ�ʶ�Ӧitems[i-1]��
        int weight = items[i-1].weight;
        double value = items[i-1].value;

        for (int j = 1; j <= capacity; j++) { // ������������
            if (weight > j) {
                // �޷�װ�뵱ǰ��Ʒ���̳���һ��״̬
                dp[i][j] = dp[i-1][j];
                prev[i][j] = 0; // δѡ��ǰ��Ʒ
            } else {
                // ѡ��ǰ��Ʒ��ѡ��ǰ��Ʒ��ȡ��ֵ�ϴ���
                double take = dp[i-1][j - weight] + value;
                double not_take = dp[i-1][j];
                if (take > not_take) {
                    dp[i][j] = take;
                    prev[i][j] = 1; // ���Ϊѡ�е�ǰ��Ʒ
                } else {
                    dp[i][j] = not_take;
                    prev[i][j] = 0; // δѡ��ǰ��Ʒ
                }
            }
        }
    }

    // ��ȡ����ֵ
    *max_value = dp[n][capacity];

    // ����·����ȷ��ѡ�е���Ʒ�������һ����Ʒ��ʼ��ǰ�Ƶ���
    *total_weight = 0;
    // ��selected����ȫ����ʼ��Ϊfalse
    memset(selected, false, n * sizeof(bool));
    int i = n, j = capacity;

    while (i > 0 && j > 0) {
        if (prev[i][j] == 1) {
            selected[i-1] = true;  // ѡ���˵�i����Ʒ��ʵ�ʶ�Ӧitems[i-1]����
            *total_weight += items[i-1].weight;
            j -= items[i-1].weight; // ������ȥ��ǰ��Ʒ����
        }
        i--; // ����ǰһ����Ʒ
    }

    // �ͷ��ڴ�
    for (int i = 0; i <= n; i++) {
        free(dp[i]);
        free(prev[i]);
    }
    free(dp);
    free(prev);
}

// 2.2 ��̬�滮������ʹ��һά������������ά����
void dynamic_programming1D(Item *items, int n, int capacity, double *max_value, int *total_weight, bool *selected) {
    // ��ʼ��һάDP�����·����¼����
    double *dp = (double *)calloc(capacity + 1, sizeof(double));

    bool **prev = (bool **)malloc((n + 1) * sizeof(bool *));
    for (int i = 0; i <= n; i++) {
        prev[i] = (bool *)calloc(capacity + 1, sizeof(bool));
    }

    // ���DP���Ե����ϣ����д�����Ʒ��
    for (int i = 1; i <= n; i++) {
        int weight = items[i-1].weight;
        double value = items[i-1].value;

        // �����ǵ������������ȷ��ÿ����Ʒֻ������һ��
        // ����Ļ�������ڼ�������dp[j]ʱ���ܳ���һ����Ʒ���������
        for (int j = capacity; j >= weight; j--) {
            double take = dp[j - weight] + value;
            double not_take = dp[j];
            if (take > not_take) {
                dp[j] = take;
                prev[i][j] = true;
            }
        }
    }

    // ��ȡ����ֵ
    *max_value = dp[capacity];

    // ����·����ȷ��ѡ�е���Ʒ
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

    // �ͷ��ڴ�
    for (int i = 0; i <= n; i++) {
        free(prev[i]);
    }
    free(prev);
    free(dp);
}

// 3. ̰�ķ���������ƽ⣬�����ţ�
void greedy_algorithm(Item *items, int n, int capacity, double *max_value, int *total_weight, bool *selected) {
    Item *sorted_items = (Item *)malloc(n * sizeof(Item));
    memcpy(sorted_items, items, n * sizeof(Item));

    // �Ȱ� ��ֵ/���� ���еݼ�����
    for (int i = 0; i < n; i++) {
        sorted_items[i].ratio = sorted_items[i].value / sorted_items[i].weight;
    }
    quick_sort(sorted_items, 0, n - 1);

    *max_value = 0;
    *total_weight = 0;
    memset(selected, false, n * sizeof(bool));

    int remaining_capacity = capacity;
    // 0-1 ��������ֻ�����棬Ҳ���̰���㷨���ܵò�����ȷ��
    for (int i = 0; i < n; i++) {
        if (sorted_items[i].weight <= remaining_capacity) {
            selected[sorted_items[i].index] = true;  // index������֮ǰ��ԭ����
            *max_value += sorted_items[i].value;
            *total_weight += sorted_items[i].weight;
            remaining_capacity -= sorted_items[i].weight;
        }
    }

    free(sorted_items);
}

// �����Ͻ纯��
double calculate_upper_bound(Item *items, int n, int capacity, int level, int current_weight, double current_value) {
    if (level >= n) return current_value; // �߽���

    int remaining_capacity = capacity - current_weight;
    double upper_bound = current_value;

    // �ӵ�ǰ��Ʒ��ʼ������ֵ�����Ƚ�������Ͻ�
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

// �����㷨���ĺ���
void backtrack(Item *items, int n, int capacity, int level, int current_weight, double current_value, bool *selected, double *best_value, int *best_weight, bool *best_selected) {
    // ����Ҷ�ӽڵ������������
    if (level == n) {
        if (current_value > *best_value) {
            *best_value = current_value;
            *best_weight = current_weight;
            // ���浱ǰ���Ž��ѡ��״̬
            memcpy(best_selected, selected, n * sizeof(bool));
        }
        return;
    }

    // �����Ͻ磬���Ͻ�С�ڵ��ڵ�ǰ���Ž����֦
    double upper_bound = calculate_upper_bound(items, n, capacity, level, current_weight, current_value);
    if (upper_bound <= *best_value + 1e-9) {
        return;
    }

    // ���Բ�ѡ��ǰ��Ʒ
    backtrack(items, n, capacity, level + 1, current_weight, current_value,
             selected, best_value, best_weight, best_selected);

    // ����ѡ��ǰ��Ʒ��������ԣ�
    if (current_weight + items[level].weight <= capacity) {
        selected[level] = true;
        backtrack(items, n, capacity, level + 1,
                 current_weight + items[level].weight,
                 current_value + items[level].value,
                 selected, best_value, best_weight, best_selected);
        selected[level] = false; // ���ݺ�ָ�״̬
    }
}

// �����㷨����ӿ�
void backtracking_algorithm(Item *items, int n, int capacity, double *max_value, int *total_weight, bool *selected) {
    // �Ȱ���ֵ�����ȶ���Ʒ������߻���Ч��
    Item *sorted_items = (Item *)malloc(n * sizeof(Item));
    memcpy(sorted_items, items, n * sizeof(Item));

    for (int i = 0; i < n; i++) {
        sorted_items[i].ratio = sorted_items[i].value / sorted_items[i].weight;
    }
    quick_sort(sorted_items, 0, n - 1);

    // ��ʼ�����Ž�
    double best_value = 0;
    int best_weight = 0;
    bool *temp_selected = (bool *)calloc(n, sizeof(bool));
    bool *best_selected = (bool *)calloc(n, sizeof(bool)); // ��������ѡ��״̬

    // ִ�л���
    backtrack(sorted_items, n, capacity, 0, 0, 0, temp_selected, &best_value, &best_weight, best_selected);

    // ��¼�����ԭʼ����
    for (int i = 0; i < n; i++) {
        if (best_selected[i]) {
            // ʹ������������е�ԭʼ����
            selected[sorted_items[i].index] = true;
        }
    }

    // ���ؽ��
    *max_value = best_value;
    *total_weight = best_weight;

    // �ͷ��ڴ�
    free(temp_selected);
    free(best_selected);
    free(sorted_items);
}

// ���Ե����㷨����ʹ�ú���ָ��
void test_algorithm(void (*algorithm)(Item*, int, int, double*, int*, bool*), const char *name, Item *items, int n, int capacity) {
    clock_t start, end;
    double max_value = 0;
    int total_weight = 0;
    bool *selected = (bool *)malloc(n * sizeof(bool));
    memset(selected, false, n * sizeof(bool));

    start = clock();
    algorithm(items, n, capacity, &max_value, &total_weight, selected);
    end = clock();
    // ʹ��ms��ʾ
    double time_ms = ((double)(end - start) / CLOCKS_PER_SEC) * 1000;

    printf("\n=== %s ===\n", name);
    printf("�ܼ�ֵ: %.2f\n", max_value);
    printf("������: %d\n", total_weight);
    printf("ִ��ʱ��: %.2f ����\n", time_ms);
    printf("ѡ����Ʒ: ");
    for (int i = 0; i < n; i++) {
        if (selected[i]) printf("%d ", i + 1); // ��Ʒ��Ŵ�1��ʼ
    }
    printf("\n");

    free(selected);
}

int main() {
    int n = 1000; // ������Ʒ���������޸�Ϊ1000-320000��
    int capacity = 10000; // �������� [10000, 100000, 1000000]
    Item *items = (Item *)malloc(n * sizeof(Item));

    generate_items(items, n); // ���������Ʒ����

    // ���Ը��㷨
    if (n <= 20) test_algorithm(brute_force, "������", items, n, capacity);
    test_algorithm(dynamic_programming2D, "ʹ�ö�ά����Ķ�̬�滮��", items, n, capacity);
    test_algorithm(dynamic_programming1D, "ʹ��һά��������Ķ�̬�滮��", items, n, capacity);
    test_algorithm(greedy_algorithm, "̰�ķ�", items, n, capacity);
    test_algorithm(backtracking_algorithm, "���ݷ�", items, n, capacity);

    free(items);
    return 0;
}
