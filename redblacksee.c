/* redblacksee.c —— 红黑树（Red-Black Tree）完整 C 实现
 *
 * 五条红黑性质：
 *   1. 每个节点是红色或黑色；
 *   2. 根节点是黑色；
 *   3. 所有叶子（NIL 哨兵）是黑色；
 *   4. 红色节点的两个子节点都是黑色（不允许连续红节点）；
 *   5. 从任一节点到其所有后代叶子的路径上，黑节点数量相同。
 *
 * 提供：创建/销毁、插入/更新、删除、查找、最小/最大、前驱/后继、
 *       中序遍历、区间查询、结构校验。
 *
 * 编译：gcc -std=c11 -O2 -Wall -Wextra redblacksee.c -o redblacksee
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { RB_RED = 0, RB_BLACK = 1 } Color;

typedef struct RBNode {
    int key;
    int value;
    Color color;
    struct RBNode *left;
    struct RBNode *right;
    struct RBNode *parent;
} RBNode;

typedef struct {
    RBNode *root;
    RBNode *nil; /* 哨兵：代表所有外部叶子 */
    size_t size;
} RBTree;

/* ------------------------------------------------------------------ 基础 */

static void *xmalloc(size_t n)
{
    void *p = malloc(n);
    if (p == NULL) {
        fprintf(stderr, "内存分配失败\n");
        exit(EXIT_FAILURE);
    }
    return p;
}

RBTree *rb_create(void)
{
    RBTree *t = (RBTree *)xmalloc(sizeof(RBTree));
    t->nil = (RBNode *)xmalloc(sizeof(RBNode));
    t->nil->key = 0;
    t->nil->value = 0;
    t->nil->color = RB_BLACK;
    t->nil->left = t->nil->right = t->nil->parent = t->nil;
    t->root = t->nil;
    t->size = 0;
    return t;
}

static void rb_free_subtree(RBTree *t, RBNode *x)
{
    if (x == t->nil)
        return;
    rb_free_subtree(t, x->left);
    rb_free_subtree(t, x->right);
    free(x);
}

void rb_destroy(RBTree *t)
{
    if (t == NULL)
        return;
    rb_free_subtree(t, t->root);
    free(t->nil);
    free(t);
}

size_t rb_size(const RBTree *t) { return t->size; }

int rb_empty(const RBTree *t) { return t->root == t->nil; }

/* ------------------------------------------------------------------ 旋转 */

static void rb_left_rotate(RBTree *t, RBNode *x)
{
    RBNode *y = x->right;
    x->right = y->left;
    if (y->left != t->nil)
        y->left->parent = x;
    y->parent = x->parent;
    if (x->parent == t->nil)
        t->root = y;
    else if (x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;
    y->left = x;
    x->parent = y;
}

static void rb_right_rotate(RBTree *t, RBNode *x)
{
    RBNode *y = x->left;
    x->left = y->right;
    if (y->right != t->nil)
        y->right->parent = x;
    y->parent = x->parent;
    if (x->parent == t->nil)
        t->root = y;
    else if (x == x->parent->right)
        x->parent->right = y;
    else
        x->parent->left = y;
    y->right = x;
    x->parent = y;
}

/* ------------------------------------------------------------------ 插入 */

static void rb_insert_fixup(RBTree *t, RBNode *z)
{
    while (z->parent->color == RB_RED) {
        RBNode *parent = z->parent;
        RBNode *grand = parent->parent;
        if (parent == grand->left) {
            RBNode *uncle = grand->right;
            if (uncle->color == RB_RED) { /* 情况 1：叔叔为红 */
                parent->color = RB_BLACK;
                uncle->color = RB_BLACK;
                grand->color = RB_RED;
                z = grand;
            } else {
                if (z == parent->right) { /* 情况 2：内侧，转成情况 3 */
                    z = parent;
                    rb_left_rotate(t, z);
                    parent = z->parent;
                    grand = parent->parent;
                }
                /* 情况 3：外侧 */
                parent->color = RB_BLACK;
                grand->color = RB_RED;
                rb_right_rotate(t, grand);
            }
        } else {
            RBNode *uncle = grand->left;
            if (uncle->color == RB_RED) {
                parent->color = RB_BLACK;
                uncle->color = RB_BLACK;
                grand->color = RB_RED;
                z = grand;
            } else {
                if (z == parent->left) {
                    z = parent;
                    rb_right_rotate(t, z);
                    parent = z->parent;
                    grand = parent->parent;
                }
                parent->color = RB_BLACK;
                grand->color = RB_RED;
                rb_left_rotate(t, grand);
            }
        }
    }
    t->root->color = RB_BLACK;
}

static RBNode *rb_find_node(const RBTree *t, int key)
{
    RBNode *cur = t->root;
    while (cur != t->nil) {
        if (key < cur->key)
            cur = cur->left;
        else if (key > cur->key)
            cur = cur->right;
        else
            return cur;
    }
    return t->nil;
}

/* 插入；key 已存在则更新 value。返回 1 表示新插入，0 表示更新。 */
int rb_insert(RBTree *t, int key, int value)
{
    RBNode *parent = t->nil;
    RBNode *cur = t->root;

    while (cur != t->nil) {
        parent = cur;
        if (key < cur->key)
            cur = cur->left;
        else if (key > cur->key)
            cur = cur->right;
        else {
            cur->value = value;
            return 0;
        }
    }

    RBNode *z = (RBNode *)xmalloc(sizeof(RBNode));
    z->key = key;
    z->value = value;
    z->color = RB_RED;
    z->left = z->right = t->nil;
    z->parent = parent;

    if (parent == t->nil)
        t->root = z;
    else if (key < parent->key)
        parent->left = z;
    else
        parent->right = z;

    t->size++;
    rb_insert_fixup(t, z);
    return 1;
}

/* ------------------------------------------------------------------ 删除 */

static void rb_transplant(RBTree *t, RBNode *u, RBNode *v)
{
    if (u->parent == t->nil)
        t->root = v;
    else if (u == u->parent->left)
        u->parent->left = v;
    else
        u->parent->right = v;
    v->parent = u->parent;
}

static RBNode *rb_min_node(RBTree *t, RBNode *x)
{
    while (x->left != t->nil)
        x = x->left;
    return x;
}

static void rb_delete_fixup(RBTree *t, RBNode *x)
{
    while (x != t->root && x->color == RB_BLACK) {
        RBNode *parent = x->parent;
        if (x == parent->left) {
            RBNode *w = parent->right;
            if (w->color == RB_RED) { /* 情况 1：兄弟为红 */
                w->color = RB_BLACK;
                parent->color = RB_RED;
                rb_left_rotate(t, parent);
                w = parent->right;
            }
            if (w->left->color == RB_BLACK && w->right->color == RB_BLACK) {
                w->color = RB_RED; /* 情况 2：兄弟双子皆黑 */
                x = parent;
            } else {
                if (w->right->color == RB_BLACK) { /* 情况 3：内侄红、外侄黑 */
                    w->left->color = RB_BLACK;
                    w->color = RB_RED;
                    rb_right_rotate(t, w);
                    w = parent->right;
                }
                /* 情况 4：外侄红 */
                w->color = parent->color;
                parent->color = RB_BLACK;
                w->right->color = RB_BLACK;
                rb_left_rotate(t, parent);
                x = t->root;
            }
        } else {
            RBNode *w = parent->left;
            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                parent->color = RB_RED;
                rb_right_rotate(t, parent);
                w = parent->left;
            }
            if (w->right->color == RB_BLACK && w->left->color == RB_BLACK) {
                w->color = RB_RED;
                x = parent;
            } else {
                if (w->left->color == RB_BLACK) {
                    w->right->color = RB_BLACK;
                    w->color = RB_RED;
                    rb_left_rotate(t, w);
                    w = parent->left;
                }
                w->color = parent->color;
                parent->color = RB_BLACK;
                w->left->color = RB_BLACK;
                rb_right_rotate(t, parent);
                x = t->root;
            }
        }
    }
    x->color = RB_BLACK;
}

/* 删除 key，成功返回 1，不存在返回 0。 */
int rb_delete(RBTree *t, int key)
{
    RBNode *z = rb_find_node(t, key);
    if (z == t->nil)
        return 0;

    RBNode *y = z;
    Color y_color = y->color;
    RBNode *x;

    if (z->left == t->nil) {
        x = z->right;
        rb_transplant(t, z, z->right);
    } else if (z->right == t->nil) {
        x = z->left;
        rb_transplant(t, z, z->left);
    } else {
        y = rb_min_node(t, z->right);
        y_color = y->color;
        x = y->right;
        if (y->parent == z) {
            x->parent = y;
        } else {
            rb_transplant(t, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        rb_transplant(t, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    free(z);
    t->size--;
    if (y_color == RB_BLACK)
        rb_delete_fixup(t, x);
    return 1;
}

/* ------------------------------------------------------------------ 查询 */

/* 查找 key，找到返回 1 并通过 out 输出 value。 */
int rb_search(const RBTree *t, int key, int *out)
{
    RBNode *n = rb_find_node(t, key);
    if (n == t->nil)
        return 0;
    if (out != NULL)
        *out = n->value;
    return 1;
}

int rb_contains(const RBTree *t, int key) { return rb_find_node(t, key) != t->nil; }

int rb_min(const RBTree *t, int *key, int *value)
{
    RBNode *n = t->root;
    while (n->left != t->nil)
        n = n->left;
    if (n == t->nil)
        return 0;
    if (key)
        *key = n->key;
    if (value)
        *value = n->value;
    return 1;
}

int rb_max(const RBTree *t, int *key, int *value)
{
    RBNode *n = t->root;
    while (n->right != t->nil)
        n = n->right;
    if (n == t->nil)
        return 0;
    if (key)
        *key = n->key;
    if (value)
        *value = n->value;
    return 1;
}

/* 小于 key 的最大键；不存在返回 0。 */
int rb_predecessor(const RBTree *t, int key, int *out_key, int *out_value)
{
    RBNode *cur = t->root;
    RBNode *best = t->nil;
    int found = 0;
    while (cur != t->nil) {
        if (key <= cur->key) {
            cur = cur->left;
        } else {
            best = cur;
            found = 1;
            cur = cur->right;
        }
    }
    if (found && out_key)
        *out_key = best->key;
    if (found && out_value)
        *out_value = best->value;
    return found;
}

/* 大于 key 的最小键；不存在返回 0。 */
int rb_successor(const RBTree *t, int key, int *out_key, int *out_value)
{
    RBNode *cur = t->root;
    RBNode *best = t->nil;
    int found = 0;
    while (cur != t->nil) {
        if (key >= cur->key) {
            cur = cur->right;
        } else {
            best = cur;
            found = 1;
            cur = cur->left;
        }
    }
    if (found && out_key)
        *out_key = best->key;
    if (found && out_value)
        *out_value = best->value;
    return found;
}

/* 升序收集到数组，返回实际元素个数（数组容量不足时截断）。 */
static int rb_collect(const RBTree *t, RBNode *x, int *keys, int *values, int n, int cap)
{
    if (x == t->nil || n >= cap)
        return n;
    n = rb_collect(t, x->left, keys, values, n, cap);
    if (n < cap) {
        if (keys)
            keys[n] = x->key;
        if (values)
            values[n] = x->value;
        n++;
    }
    return rb_collect(t, x->right, keys, values, n, cap);
}

int rb_to_array(const RBTree *t, int *keys, int *values, int cap)
{
    return rb_collect(t, t->root, keys, values, 0, cap);
}

/* 将 [low, high] 内的键值对升序写入数组，返回个数。 */
static int rb_collect_range(const RBTree *t, RBNode *x, int low, int high, int *keys,
                            int *values, int n, int cap)
{
    if (x == t->nil || n >= cap)
        return n;
    if (low < x->key)
        n = rb_collect_range(t, x->left, low, high, keys, values, n, cap);
    if (n >= cap)
        return n;
    if (low <= x->key && x->key <= high) {
        if (keys)
            keys[n] = x->key;
        if (values)
            values[n] = x->value;
        n++;
    }
    if (x->key <= high)
        n = rb_collect_range(t, x->right, low, high, keys, values, n, cap);
    return n;
}

int rb_range_query(const RBTree *t, int low, int high, int *keys, int *values, int cap)
{
    return rb_collect_range(t, t->root, low, high, keys, values, 0, cap);
}

void rb_print_inorder(const RBTree *t)
{
    int cap = (int)t->size;
    if (cap == 0) {
        printf("(空)\n");
        return;
    }
    int *keys = (int *)xmalloc(sizeof(int) * (size_t)cap);
    int *vals = (int *)xmalloc(sizeof(int) * (size_t)cap);
    int n = rb_to_array(t, keys, vals, cap);
    printf("[");
    for (int i = 0; i < n; i++)
        printf("%s%d:%d", i ? ", " : "", keys[i], vals[i]);
    printf("]\n");
    free(keys);
    free(vals);
}

/* ------------------------------------------------------------------ 校验 */

static int rb_height_rec(const RBTree *t, RBNode *x)
{
    if (x == t->nil)
        return 0;
    int l = rb_height_rec(t, x->left);
    int r = rb_height_rec(t, x->right);
    return 1 + (l > r ? l : r);
}

int rb_height(const RBTree *t) { return rb_height_rec(t, t->root); }

/* 返回黑高；结构不合法时打印原因并返回 -1。 */
static int rb_validate_rec(const RBTree *t, RBNode *x)
{
    if (x == t->nil)
        return 1;

    if (x->color == RB_RED && (x->left->color == RB_RED || x->right->color == RB_RED)) {
        fprintf(stderr, "校验失败：红色节点 %d 有红色子节点\n", x->key);
        return -1;
    }
    if (x->left != t->nil && x->left->parent != x) {
        fprintf(stderr, "校验失败：%d 的左孩子父指针错误\n", x->left->key);
        return -1;
    }
    if (x->right != t->nil && x->right->parent != x) {
        fprintf(stderr, "校验失败：%d 的右孩子父指针错误\n", x->right->key);
        return -1;
    }

    int lbh = rb_validate_rec(t, x->left);
    if (lbh < 0)
        return -1;
    int rbh = rb_validate_rec(t, x->right);
    if (rbh < 0)
        return -1;
    if (lbh != rbh) {
        fprintf(stderr, "校验失败：节点 %d 两侧黑高不等（%d vs %d）\n", x->key, lbh, rbh);
        return -1;
    }
    return lbh + (x->color == RB_RED ? 0 : 1);
}

int rb_validate(const RBTree *t)
{
    if (t->nil->color != RB_BLACK) {
        fprintf(stderr, "校验失败：NIL 哨兵不是黑色\n");
        return -1;
    }
    if (t->root != t->nil && t->root->color != RB_BLACK) {
        fprintf(stderr, "校验失败：根节点不是黑色\n");
        return -1;
    }
    if (t->root->parent != t->nil) {
        fprintf(stderr, "校验失败：根节点的 parent 不是 NIL\n");
        return -1;
    }
    return rb_validate_rec(t, t->root);
}

/* ------------------------------------------------------------------ 演示 */

static void demo_basic(void)
{
    RBTree *t = rb_create();
    int keys[] = {41, 38, 31, 12, 19, 8};

    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++)
        rb_insert(t, keys[i], keys[i] * 10);

    printf("插入 41,38,31,12,19,8 后：\n");
    printf("  元素个数=%zu 高度=%d\n", rb_size(t), rb_height(t));
    printf("  升序输出：");
    rb_print_inorder(t);
    printf("  黑高=%d（校验通过）\n", rb_validate(t));

    rb_insert(t, 50, 500);
    rb_insert(t, 38, 999); /* 更新 */
    int v = 0;
    rb_search(t, 38, &v);
    printf("更新 38 的值为 %d，树中现有 %zu 个元素\n", v, rb_size(t));

    printf("前驱/后继：\n");
    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
        int pk = 0, sk = 0;
        int has_p = rb_predecessor(t, keys[i], &pk, NULL);
        int has_s = rb_successor(t, keys[i], &sk, NULL);
        printf("  %-3d 前驱=", keys[i]);
        if (has_p)
            printf("%-4d", pk);
        else
            printf("无  ");
        printf(" 后继=");
        if (has_s)
            printf("%d", sk);
        else
            printf("无");
        printf("\n");
    }

    int rk[16], rv[16];
    int n = rb_range_query(t, 19, 41, rk, rv, 16);
    printf("区间 [19,41] 共 %d 个：", n);
    for (int i = 0; i < n; i++)
        printf("%d:%d ", rk[i], rv[i]);
    printf("\n");

    int del[] = {41, 38, 31};
    for (int i = 0; i < 3; i++) {
        rb_delete(t, del[i]);
        if (rb_validate(t) < 0) {
            fprintf(stderr, "删除 %d 后校验失败\n", del[i]);
            exit(EXIT_FAILURE);
        }
    }
    printf("删除 41/38/31 后：");
    rb_print_inorder(t);
    printf("  高度=%d 黑高=%d\n", rb_height(t), rb_validate(t));

    rb_destroy(t);
}

static void demo_stress(void)
{
    enum { MAX_KEY = 500, OPS = 3000 };
    RBTree *t = rb_create();
    int ref[MAX_KEY + 1];
    int ref_count = 0;

    for (int i = 0; i <= MAX_KEY; i++)
        ref[i] = INT_MIN;
    srand(2026);

    for (int i = 0; i < OPS; i++) {
        int op = rand() % 100;
        int k = rand() % (MAX_KEY + 1);

        if (op < 60) {
            if (ref[k] == INT_MIN)
                ref_count++;
            ref[k] = k * 10;
            rb_insert(t, k, ref[k]);
        } else if (op < 90) {
            int expected = (ref[k] != INT_MIN);
            if (rb_delete(t, k) != expected) {
                fprintf(stderr, "删除返回值与预期不符：key=%d\n", k);
                exit(EXIT_FAILURE);
            }
            if (expected) {
                ref[k] = INT_MIN;
                ref_count--;
            }
        } else {
            int got = 0;
            int found = rb_search(t, k, &got);
            int expected = (ref[k] != INT_MIN);
            if (found != expected || (found && got != ref[k])) {
                fprintf(stderr, "查询结果与预期不符：key=%d\n", k);
                exit(EXIT_FAILURE);
            }
        }

        if ((int)rb_size(t) != ref_count) {
            fprintf(stderr, "元素个数不一致：%zu vs %d\n", rb_size(t), ref_count);
            exit(EXIT_FAILURE);
        }
        if (rb_validate(t) < 0) {
            fprintf(stderr, "第 %d 次操作后红黑性质被破坏\n", i);
            exit(EXIT_FAILURE);
        }
    }

    int *keys = (int *)xmalloc(sizeof(int) * (size_t)(ref_count > 0 ? ref_count : 1));
    int n = rb_to_array(t, keys, NULL, ref_count);
    int j = 0;
    for (int k = 0; k <= MAX_KEY; k++) {
        if (ref[k] != INT_MIN) {
            if (j >= n || keys[j] != k) {
                fprintf(stderr, "升序遍历与预期不符\n");
                exit(EXIT_FAILURE);
            }
            j++;
        }
    }
    free(keys);

    printf("随机压力测试通过：%d 次操作后 size=%zu, height=%d, 黑高=%d\n", OPS, rb_size(t),
           rb_height(t), rb_validate(t));
    rb_destroy(t);
}

int main(void)
{
    demo_basic();
    printf("\n");
    demo_stress();
    return 0;
}
