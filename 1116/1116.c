#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

typedef struct {
    long long comparisons;
    double time_ms;
    size_t max_memory;
} Metrics;

long long g_comparisons = 0;
size_t g_current_mem = 0;
size_t g_max_mem = 0;

void add_mem(size_t bytes) {
    g_current_mem += bytes;
    if (g_current_mem > g_max_mem) {
        g_max_mem = g_current_mem;
    }
}

void sub_mem(size_t bytes) {
    if (g_current_mem >= bytes) {
        g_current_mem -= bytes;
    }
}

void reset_metrics() {
    g_comparisons = 0;
    g_current_mem = 0;
    g_max_mem = 0;
}

int rand32() {
    return ((rand() & 0x7FFF) << 15) | (rand() & 0x7FFF);
}

void copy_array(int* dest, const int* src, int n) {
    memcpy(dest, src, sizeof(int) * n);
}

int is_sorted(const int* arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        if (arr[i] > arr[i + 1]) return 0;
    }
    return 1;
}

void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void heapify(int arr[], int n, int i) {
    const size_t frame_size = 32;
    add_mem(frame_size);

    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n) {
        g_comparisons++;
        if (arr[left] > arr[largest]) {
            largest = left;
        }
    }

    if (right < n) {
        g_comparisons++;
        if (arr[right] > arr[largest]) {
            largest = right;
        }
    }

    if (largest != i) {
        swap(&arr[i], &arr[largest]);
        heapify(arr, n, largest);
    }

    sub_mem(frame_size);
}

void heap_sort(int arr[], int n) {
    add_mem(sizeof(int) * 2);

    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i);
    }

    for (int i = n - 1; i > 0; i--) {
        swap(&arr[0], &arr[i]);
        heapify(arr, i, 0);
    }

    sub_mem(sizeof(int) * 2);
}

void merge(int arr[], int temp[], int left, int mid, int right) {
    int i = left;
    int j = mid + 1;
    int k = left;

    while (i <= mid && j <= right) {
        g_comparisons++;
        if (arr[i] <= arr[j]) {
            temp[k++] = arr[i++];
        }
        else {
            temp[k++] = arr[j++];
        }
    }

    while (i <= mid) {
        temp[k++] = arr[i++];
    }

    while (j <= right) {
        temp[k++] = arr[j++];
    }

    for (i = left; i <= right; i++) {
        arr[i] = temp[i];
    }
}

void merge_sort_recursive(int arr[], int temp[], int left, int right) {
    const size_t frame_size = 40;
    add_mem(frame_size);

    if (left < right) {
        int mid = left + (right - left) / 2;
        merge_sort_recursive(arr, temp, left, mid);
        merge_sort_recursive(arr, temp, mid + 1, right);
        merge(arr, temp, left, mid, right);
    }

    sub_mem(frame_size);
}

void merge_sort(int arr[], int n) {
    size_t temp_size = sizeof(int) * n;
    int* temp = (int*)malloc(temp_size);
    add_mem(temp_size);

    merge_sort_recursive(arr, temp, 0, n - 1);

    free(temp);
    sub_mem(temp_size);
}

int median_of_three(int arr[], int low, int high) {
    int mid = low + (high - low) / 2;

    g_comparisons++;
    if (arr[low] > arr[mid]) swap(&arr[low], &arr[mid]);

    g_comparisons++;
    if (arr[low] > arr[high]) swap(&arr[low], &arr[high]);

    g_comparisons++;
    if (arr[mid] > arr[high]) swap(&arr[mid], &arr[high]);

    return mid;
}

int partition(int arr[], int low, int high, int pivot_type) {
    int pivot_idx = low;

    if (pivot_type == 1) {
        pivot_idx = low;
    }
    else if (pivot_type == 2) {
        pivot_idx = low + rand32() % (high - low + 1);
    }
    else if (pivot_type == 3) {
        pivot_idx = median_of_three(arr, low, high);
    }

    swap(&arr[low], &arr[pivot_idx]);
    int pivot = arr[low];
    int i = low + 1;
    int j = high;

    while (1) {
        while (i <= high) {
            g_comparisons++;
            if (arr[i] <= pivot) i++;
            else break;
        }
        while (j >= low + 1) {
            g_comparisons++;
            if (arr[j] > pivot) j--;
            else break;
        }
        if (i >= j) break;
        swap(&arr[i], &arr[j]);
    }
    swap(&arr[low], &arr[j]);
    return j;
}

void quick_sort_recursive(int arr[], int low, int high, int pivot_type) {
    const size_t frame_size = 40;
    add_mem(frame_size);

    if (low < high) {
        int p = partition(arr, low, high, pivot_type);
        quick_sort_recursive(arr, low, p - 1, pivot_type);
        quick_sort_recursive(arr, p + 1, high, pivot_type);
    }

    sub_mem(frame_size);
}

void quick_sort(int arr[], int n, int pivot_type) {
    quick_sort_recursive(arr, 0, n - 1, pivot_type);
}

Metrics run_benchmark(int* orig, int n, int algo_type) {
    int* arr = (int*)malloc(sizeof(int) * n);
    copy_array(arr, orig, n);
    reset_metrics();

    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    switch (algo_type) {
    case 0: heap_sort(arr, n); break;
    case 1: merge_sort(arr, n); break;
    case 2: quick_sort(arr, n, 1); break;
    case 3: quick_sort(arr, n, 2); break;
    case 4: quick_sort(arr, n, 3); break;
    }

    QueryPerformanceCounter(&end);

    if (!is_sorted(arr, n)) {
        printf("ERROR: Algorithm %d failed to sort size %d!\n", algo_type, n);
    }

    Metrics m;
    m.comparisons = g_comparisons;
    m.time_ms = (double)(end.QuadPart - start.QuadPart) * 1000.0 / freq.QuadPart;
    m.max_memory = g_max_mem;

    free(arr);
    return m;
}

int main() {
    srand((unsigned int)time(NULL));

    int sizes[] = { 1000, 2000, 4000, 8000, 16000, 20000 };
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    const char* algo_names[] = {
        "Heap Sort",
        "Merge Sort",
        "Quick (First)",
        "Quick (Random)",
        "Quick (Median3)"
    };

    for (int s = 0; s < num_sizes; s++) {
        int n = sizes[s];
        int* orig = (int*)malloc(sizeof(int) * n);

        for (int i = 0; i < n; i++) {
            orig[i] = rand32() % (n * 2);
        }

        printf("Data Size : %d\n\n", n);
        printf("%-15s %-14s %-11s %-13s\n", "Algorithm", "Comparisons", "Time(ms)", "Memory(Bytes)");
        printf("--------------------------------------------------------\n");

        for (int a = 0; a < 5; a++) {
            Metrics res = run_benchmark(orig, n, a);
            printf("%-15s %-14lld %-11.4f %-13zu\n",
                algo_names[a], res.comparisons, res.time_ms, res.max_memory);
        }
        printf("\n");

        free(orig);
    }

    return 0;
}