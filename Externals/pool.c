#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define POOL_SIZE 2048 * 2048  // 메모리 풀 크기 (4MB)

typedef struct MemoryBlock {
    struct MemoryBlock* next;
    size_t size;
} MemoryBlock;

static char memory_pool[POOL_SIZE];  // 메모리 풀
static size_t pool_offset = 0;       // 메모리 풀에서 할당된 오프셋
static MemoryBlock* free_list = NULL;  // 풀의 자유 리스트

// 메모리 풀에서 메모리 할당
void* pool_malloc(size_t size) {
    // 메모리 풀에서 충분한 공간이 있는지 확인
    if (pool_offset + size > POOL_SIZE) {
        return NULL;  // 메모리 풀에 할당할 공간이 부족하면 NULL 반환
    }

    // 현재 메모리 풀에서 메모리 할당
    void* ptr = memory_pool + pool_offset;
    pool_offset += size;  // 오프셋 증가
    return ptr;
}

// 메모리 풀에서 메모리 해제
void pool_free(void* ptr) {
    // 메모리 풀에서는 `free()`를 호출하지 않고, 풀 시스템에 맞춰 해제
    // 메모리 블록을 자유 리스트에 추가하여 재사용 가능하게 만듦
    if (ptr == NULL) {
        return;
    }

    MemoryBlock* block = (MemoryBlock*)ptr - 1;  // 메모리 블록 헤더를 찾아서
    block->next = free_list;  // 기존 자유 리스트에 추가
    free_list = block;        // 자유 리스트의 첫 번째 항목으로 추가
}

// 메모리 풀 정리 (게임 종료 시 호출)
void pool_cleanup() {
    // 자유 리스트의 모든 메모리 블록을 제거하고 풀을 초기화
    free_list = NULL;  // 자유 리스트를 비우기
    pool_offset = 0;   // 풀 오프셋 초기화
    memset(memory_pool, 0, POOL_SIZE);  // 풀 메모리 초기화
}

// 메모리 풀에서 할당된 메모리 상태 출력 (디버깅용)
void print_pool_status() {
    printf("Memory pool status:\n");
    printf("  Pool size: %zu bytes\n", POOL_SIZE);
    printf("  Allocated offset: %zu bytes\n", pool_offset);
    printf("  Free list head: %p\n", (void*)free_list);
}

// int main() {
//     // 예시로 메모리 풀에서 몇 가지 메모리 할당 및 해제 테스트
//     int* arr1 = (int*)pool_malloc(sizeof(int) * 10);
//     if (arr1) {
//         for (int i = 0; i < 10; i++) {
//             arr1[i] = i;
//         }
//     }

//     char* str = (char*)pool_malloc(50 * sizeof(char));
//     if (str) {
//         strcpy(str, "Hello from the memory pool!");
//     }

//     print_pool_status();  // 풀 상태 출력

//     // 메모리 해제
//     pool_free(arr1);
//     pool_free(str);

//     print_pool_status();  // 풀 상태 출력

//     // 게임 종료 시 메모리 풀 정리
//     pool_cleanup();

//     print_pool_status();  // 풀 상태 출력

//     return 0;
// }
