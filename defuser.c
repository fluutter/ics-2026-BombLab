#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}



int solve_phase_1(uint64_t challenge, char *out, size_t cap)
{
    uint32_t values[MAX_FIELDS] = {0};

    uint32_t low = (uint32_t)challenge;
    uint32_t mid16 = (uint32_t)(challenge >> 16);
    uint32_t high = (uint32_t)(challenge >> 32);

    uint32_t step2 =
        (high + (low << 2)) ^ UINT32_C(0x2a3b);

    uint32_t step0 =
        (mid16 + UINT32_C(0xde73)) ^ low;

    values[0] =
        ((step0 * UINT32_C(5)) + step2) ^ UINT32_C(0x70cd);

    return write_answer(out, cap, values, 1);
}



int solve_phase_2(uint64_t challenge, char *out, size_t cap)
{
    uint32_t values[MAX_FIELDS] = {0};

    values[0] =
        (((uint32_t)(challenge >> 9) * UINT32_C(2)) & UINT32_C(0x1e))
        + UINT32_C(0xb);

    for (uint32_t i = 1; i < 6; ++i) {
        uint32_t chunk =
            (uint32_t)((challenge >> (5 * i)) & UINT64_C(0x3f));

        values[i] = (chunk ^ i) + (values[i - 1] * UINT32_C(2));
    }

    return write_answer(out, cap, values, 6);
}



static uint32_t p3_dispatch(uint32_t index, uint32_t x)
{
    switch (index) {
    case 0: /* 跳转表 0 -> p3_case_7 */
        return x * UINT32_C(3) + UINT32_C(0xe84c);

    case 1: /* 跳转表 1 -> p3_case_3 */
        return x * UINT32_C(5) + UINT32_C(0x0fe5);

    case 2: /* 跳转表 2 -> p3_case_5 */
        return (x ^ UINT32_C(0x8c50)) + (x >> 1);

    case 3: /* 跳转表 3 -> p3_case_2 */
        return (x * UINT32_C(4)) ^ UINT32_C(0xd228);

    case 4: /* 跳转表 4 -> p3_case_0 */
        return (x ^ UINT32_C(0x8b00)) + (x >> 1);

    case 5: /* 跳转表 5 -> p3_case_1 */
        return x * UINT32_C(5) + UINT32_C(0x192ee);

    case 6: /* 跳转表 6 -> p3_case_4 */
        return (x * UINT32_C(2)) ^ UINT32_C(0x1099);

    case 7: /* 跳转表 7 -> p3_case_6 */
        return x * UINT32_C(5) + UINT32_C(0x38789);

    default:
        return 0;
    }
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap)
{
    uint32_t values[MAX_FIELDS] = {0};

    uint32_t index = (uint32_t)(challenge & UINT64_C(0x7));

    uint32_t argument =
        (uint32_t)(challenge >> 3) ^
        (uint32_t)(challenge >> 32);

    values[0] = index;
    values[1] = p3_dispatch(index, argument);

    return write_answer(out, cap, values, 2);
}



static uint32_t p4_walk(uint32_t target,
                        uint32_t low,
                        uint32_t high,
                        uint32_t code)
{
    uint32_t mid = low + ((high - low) >> 1);

    if (mid == target) {
        return (mid + UINT32_C(0xa8)) ^ code;
    }

    uint32_t branch_value = mid + code;
    uint32_t result;

    if (target < mid) {
        result = p4_walk(target, low, mid - 1, code);
        result = (result * UINT32_C(3)) ^ branch_value;
        return result ^ UINT32_C(0x16);
    }

    result = p4_walk(target, mid + 1, high, code);
    result = (result * UINT32_C(3)) ^ branch_value;
    return result ^ UINT32_C(0x1f);
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap)
{
    uint32_t values[MAX_FIELDS] = {0};

    uint32_t target = (uint32_t)(challenge & UINT64_C(0x7f));
    uint32_t code =
        (uint32_t)(challenge >> 7) ^
        (uint32_t)(challenge >> 32);

    values[0] = p4_walk(target, 0, 127, code);

    return write_answer(out, cap, values, 1);
}



typedef struct {
    uint32_t field0;
    uint32_t field1;
    size_t next;
} p5_node;

/*
 * 节点顺序及 next 链：
 * 0: 0x405b70 -> 1
 * 1: 0x405bc0 -> 2
 * 2: 0x405bf0 -> 3
 * 3: 0x405bb0 -> 4
 * 4: 0x405bd0 -> 5
 * 5: 0x405be0 -> 6
 * 6: 0x405ba0 -> 7
 * 7: 0x405b60 -> 8
 * 8: 0x405b80 -> 9
 * 9: 0x405c10（本题最多只会访问到这里）
 */
static const p5_node p5_nodes[] = {
    { UINT32_C(0x3596), UINT32_C(0x0226), 1 },
    { UINT32_C(0x6d10), UINT32_C(0x0817), 2 },
    { UINT32_C(0xa9f3), UINT32_C(0x0936), 3 },
    { UINT32_C(0x373b), UINT32_C(0x0b42), 4 },
    { UINT32_C(0x8c86), UINT32_C(0x001d), 5 },
    { UINT32_C(0x514a), UINT32_C(0x0332), 6 },
    { UINT32_C(0x1c23), UINT32_C(0x052e), 7 },
    { UINT32_C(0xeea5), UINT32_C(0x0152), 8 },
    { UINT32_C(0xe832), UINT32_C(0x0720), 9 },
    { UINT32_C(0x7b1c), UINT32_C(0x0a0b), 9 }
};

static const size_t p5_heads[8] = {
    0, 1, 2, 3, 4, 5, 6, 7
};

int solve_phase_5(uint64_t challenge, char *out, size_t cap)
{
    uint32_t values[MAX_FIELDS] = {0};

    size_t node_index =
        p5_heads[(size_t)(challenge & UINT64_C(0x7))];

    uint32_t result =
        (uint32_t)(challenge >> 6) ^
        (uint32_t)(challenge >> 32) ^
        p5_nodes[node_index].field0 ^
        UINT32_C(0x4739);

    uint32_t last_value = p5_nodes[node_index].field1;
    result += last_value;

    uint32_t steps = (uint32_t)((challenge >> 3) & UINT64_C(0x3));

    for (uint32_t i = 0; i < steps; ++i) {
        node_index = p5_nodes[node_index].next;

        result ^= p5_nodes[node_index].field0;
        last_value = p5_nodes[node_index].field1;
        result += last_value;
    }

    values[0] = result;
    values[1] = last_value;

    return write_answer(out, cap, values, 2);
}


typedef uint32_t (*p6_op_fn)(uint32_t);


static uint32_t p6_op_0(uint32_t x)
{
    uint32_t rotated = (x << 3) | (x >> 29);
    return rotated ^ UINT32_C(0x0c30);
}


static uint32_t p6_op_1(uint32_t x)
{
    uint32_t edx = x << 2;
    uint32_t eax = UINT32_C(0x6e2c) + edx;

    edx += x;
    eax ^= edx;

    return eax;
}


static uint32_t p6_op_2(uint32_t x)
{
    uint32_t eax = x ^ UINT32_C(0x707b);
    uint32_t shifted = x << 7;

    return eax + shifted;
}


static uint32_t p6_op_3(uint32_t x)
{
    uint32_t eax = (x >> 2) ^ x;
    return eax + UINT32_C(0x1ff7);
}


static uint32_t p6_op_4(uint32_t x)
{
    uint32_t eax = x + UINT32_C(0xfdaf);
    return eax ^ (x >> 2);
}


static uint32_t p6_op_5(uint32_t x)
{
    return (x >> 7) | (x << 25);
}

/*
 * GDB 中 p6_ops 的顺序：
 * 0: p6_op_4
 * 1: p6_op_3
 * 2: p6_op_2
 * 3: p6_op_1
 * 4: p6_op_2
 * 5: p6_op_5
 * 6: p6_op_0
 * 7: p6_op_4
 */
static const p6_op_fn p6_ops[8] = {
    p6_op_4,
    p6_op_3,
    p6_op_2,
    p6_op_1,
    p6_op_2,
    p6_op_5,
    p6_op_0,
    p6_op_4
};

int solve_phase_6(uint64_t challenge, char *out, size_t cap)
{
    uint32_t values[MAX_FIELDS] = {0};

    uint32_t first_arg =
        (uint32_t)(challenge >> 6) ^
        (uint32_t)(challenge >> 32);

    uint32_t first_index =
        (uint32_t)(challenge & UINT64_C(0x7));

    uint32_t result = p6_ops[first_index](first_arg);

    result ^= UINT32_C(0xf803);

    uint32_t second_index =
        (uint32_t)((challenge & UINT64_C(0x38)) >> 3);

    result = p6_ops[second_index](result);

    result += UINT32_C(0x599);

    values[0] = result;
    return write_answer(out, cap, values, 1);
}




int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    (void)challenge;
    (void)context;
    (void)out;
    (void)cap;
    // TODO（选做）: 根据 secret_gate、secret_copy 和 secret_phase 的汇编构造完整的第七行，写入 out 后返回 0。
    // 第七行不一定只有整数，可以直接用 snprintf 写入；还没完成时保持返回 1，测试会显示 not attempted。
    return 1;
}
