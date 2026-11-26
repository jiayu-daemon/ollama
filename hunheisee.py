"""红黑树（Red-Black Tree）完整实现。

性质：
1. 每个节点是红色或黑色。
2. 根节点是黑色。
3. 所有叶子（NIL 哨兵）是黑色。
4. 红色节点的两个子节点都是黑色（不能有连续的红节点）。
5. 从任一节点到其所有后代叶子的路径上，黑色节点数量相同。

支持：insert / delete / search / 中序遍历 / 区间查询 / 结构校验。
"""

from __future__ import annotations

from typing import Any, Iterator, List, Optional, Tuple

RED = True
BLACK = False


class Node:
    """红黑树节点。key 为 None 的节点是 NIL 哨兵。"""

    __slots__ = ("key", "value", "color", "left", "right", "parent")

    def __init__(self, key: Any = None, value: Any = None, color: bool = RED) -> None:
        self.key = key
        self.value = value
        self.color = color
        self.left: "Node" = self
        self.right: "Node" = self
        self.parent: "Node" = self

    def __repr__(self) -> str:
        if self.key is None:
            return "NIL"
        return f"{self.key}{'R' if self.color == RED else 'B'}"


class RedBlackTree:
    """以红黑树实现的映射（key 可比较，key 唯一）。"""

    def __init__(self) -> None:
        self._nil = Node(color=BLACK)
        self._root: Node = self._nil
        self._size = 0

    # ------------------------------------------------------------------ 基础
    def __len__(self) -> int:
        return self._size

    def __contains__(self, key: Any) -> bool:
        return self._find_node(key) is not self._nil

    def __getitem__(self, key: Any) -> Any:
        node = self._find_node(key)
        if node is self._nil:
            raise KeyError(key)
        return node.value

    def __setitem__(self, key: Any, value: Any) -> None:
        self.insert(key, value)

    def __delitem__(self, key: Any) -> None:
        if not self.delete(key):
            raise KeyError(key)

    def __iter__(self) -> Iterator[Any]:
        return (k for k, _ in self.items())

    def is_empty(self) -> bool:
        return self._size == 0

    # ------------------------------------------------------------------ 旋转
    def _left_rotate(self, x: Node) -> None:
        y = x.right
        x.right = y.left
        if y.left is not self._nil:
            y.left.parent = x
        y.parent = x.parent
        if x.parent is self._nil:
            self._root = y
        elif x is x.parent.left:
            x.parent.left = y
        else:
            x.parent.right = y
        y.left = x
        x.parent = y

    def _right_rotate(self, x: Node) -> None:
        y = x.left
        x.left = y.right
        if y.right is not self._nil:
            y.right.parent = x
        y.parent = x.parent
        if x.parent is self._nil:
            self._root = y
        elif x is x.parent.right:
            x.parent.right = y
        else:
            x.parent.left = y
        y.right = x
        x.parent = y

    # ------------------------------------------------------------------ 插入
    def insert(self, key: Any, value: Any = None) -> None:
        """插入或更新键值对。"""
        z = Node(key, value)
        self._insert_node(z)

    def _insert_node(self, z: Node) -> None:
        parent = self._nil
        cur = self._root
        while cur is not self._nil:
            parent = cur
            if z.key < cur.key:
                cur = cur.left
            elif z.key > cur.key:
                cur = cur.right
            else:  # key 已存在：更新值后返回
                cur.value = z.value
                return
        z.parent = parent
        if parent is self._nil:
            self._root = z
        elif z.key < parent.key:
            parent.left = z
        else:
            parent.right = z
        z.left = z.right = self._nil
        z.color = RED
        self._size += 1
        self._insert_fixup(z)

    def _insert_fixup(self, z: Node) -> None:
        while z.parent.color == RED:
            parent = z.parent
            grand = parent.parent
            if parent is grand.left:
                uncle = grand.right
                if uncle.color == RED:  # 情况 1：叔叔为红 -> 变色上移
                    parent.color = BLACK
                    uncle.color = BLACK
                    grand.color = RED
                    z = grand
                else:
                    if z is parent.right:  # 情况 2：内侧 -> 转成情况 3
                        z = parent
                        self._left_rotate(z)
                        parent = z.parent
                        grand = parent.parent
                    # 情况 3：外侧 -> 变色 + 旋转
                    parent.color = BLACK
                    grand.color = RED
                    self._right_rotate(grand)
            else:
                uncle = grand.left
                if uncle.color == RED:
                    parent.color = BLACK
                    uncle.color = BLACK
                    grand.color = RED
                    z = grand
                else:
                    if z is parent.left:
                        z = parent
                        self._right_rotate(z)
                        parent = z.parent
                        grand = parent.parent
                    parent.color = BLACK
                    grand.color = RED
                    self._left_rotate(grand)
        self._root.color = BLACK

    # ------------------------------------------------------------------ 删除
    def delete(self, key: Any) -> bool:
        """删除键，成功返回 True，键不存在返回 False。"""
        z = self._find_node(key)
        if z is self._nil:
            return False
        self._delete_node(z)
        self._size -= 1
        return True

    def _transplant(self, u: Node, v: Node) -> None:
        if u.parent is self._nil:
            self._root = v
        elif u is u.parent.left:
            u.parent.left = v
        else:
            u.parent.right = v
        v.parent = u.parent

    def _delete_node(self, z: Node) -> None:
        y = z
        y_color = y.color
        if z.left is self._nil:
            x = z.right
            self._transplant(z, z.right)
        elif z.right is self._nil:
            x = z.left
            self._transplant(z, z.left)
        else:
            y = self._minimum(z.right)
            y_color = y.color
            x = y.right
            if y.parent is z:
                x.parent = y
            else:
                self._transplant(y, y.right)
                y.right = z.right
                y.right.parent = y
            self._transplant(z, y)
            y.left = z.left
            y.left.parent = y
            y.color = z.color
        if y_color == BLACK:
            self._delete_fixup(x)

    def _delete_fixup(self, x: Node) -> None:
        while x is not self._root and x.color == BLACK:
            parent = x.parent
            if x is parent.left:
                sibling = parent.right
                if sibling.color == RED:  # 情况 1：兄弟为红
                    sibling.color = BLACK
                    parent.color = RED
                    self._left_rotate(parent)
                    sibling = parent.right
                if sibling.left.color == BLACK and sibling.right.color == BLACK:
                    sibling.color = RED  # 情况 2：兄弟双子皆黑
                    x = parent
                else:
                    if sibling.right.color == BLACK:  # 情况 3：内侄黑、外侄红
                        sibling.left.color = BLACK
                        sibling.color = RED
                        self._right_rotate(sibling)
                        sibling = parent.right
                    # 情况 4：外侄红
                    sibling.color = parent.color
                    parent.color = BLACK
                    sibling.right.color = BLACK
                    self._left_rotate(parent)
                    x = self._root
            else:
                sibling = parent.left
                if sibling.color == RED:
                    sibling.color = BLACK
                    parent.color = RED
                    self._right_rotate(parent)
                    sibling = parent.left
                if sibling.right.color == BLACK and sibling.left.color == BLACK:
                    sibling.color = RED
                    x = parent
                else:
                    if sibling.left.color == BLACK:
                        sibling.right.color = BLACK
                        sibling.color = RED
                        self._left_rotate(sibling)
                        sibling = parent.left
                    sibling.color = parent.color
                    parent.color = BLACK
                    sibling.left.color = BLACK
                    self._right_rotate(parent)
                    x = self._root
        x.color = BLACK

    # ------------------------------------------------------------------ 查询
    def _find_node(self, key: Any) -> Node:
        cur = self._root
        while cur is not self._nil:
            if key < cur.key:
                cur = cur.left
            elif key > cur.key:
                cur = cur.right
            else:
                return cur
        return self._nil

    def search(self, key: Any, default: Any = None) -> Any:
        node = self._find_node(key)
        return default if node is self._nil else node.value

    def _minimum(self, node: Node) -> Node:
        while node.left is not self._nil:
            node = node.left
        return node

    def _maximum(self, node: Node) -> Node:
        while node.right is not self._nil:
            node = node.right
        return node

    def min_key(self) -> Any:
        if self._root is self._nil:
            raise ValueError("树为空")
        return self._minimum(self._root).key

    def max_key(self) -> Any:
        if self._root is self._nil:
            raise ValueError("树为空")
        return self._maximum(self._root).key

    def predecessor(self, key: Any) -> Optional[Any]:
        """小于 key 的最大键，不存在返回 None。"""
        cur, result = self._root, None
        while cur is not self._nil:
            if key <= cur.key:
                cur = cur.left
            else:
                result = cur.key
                cur = cur.right
        return result

    def successor(self, key: Any) -> Optional[Any]:
        """大于 key 的最小键，不存在返回 None。"""
        cur, result = self._root, None
        while cur is not self._nil:
            if key >= cur.key:
                cur = cur.right
            else:
                result = cur.key
                cur = cur.left
        return result

    def items(self) -> List[Tuple[Any, Any]]:
        """按键升序返回 (key, value) 列表。"""
        out: List[Tuple[Any, Any]] = []
        stack, cur = [], self._root
        while stack or cur is not self._nil:
            while cur is not self._nil:
                stack.append(cur)
                cur = cur.left
            cur = stack.pop()
            out.append((cur.key, cur.value))
            cur = cur.right
        return out

    def keys(self) -> List[Any]:
        return [k for k, _ in self.items()]

    def range_query(self, low: Any, high: Any) -> List[Tuple[Any, Any]]:
        """返回 low <= key <= high 的所有键值对（升序）。"""
        out: List[Tuple[Any, Any]] = []
        stack, cur = [], self._root
        while stack or cur is not self._nil:
            while cur is not self._nil:
                stack.append(cur)
                cur = cur.left
            cur = stack.pop()
            if low <= cur.key <= high:
                out.append((cur.key, cur.value))
            elif cur.key > high:
                break
            cur = cur.right
        return out

    # ------------------------------------------------------------------ 校验
    def height(self) -> int:
        def _h(node: Node) -> int:
            if node is self._nil:
                return 0
            return 1 + max(_h(node.left), _h(node.right))

        return _h(self._root)

    def validate(self) -> int:
        """校验五条红黑性质，返回黑高；不合法时抛 AssertionError。"""
        nil = self._nil
        if nil.color != BLACK:
            raise AssertionError("NIL 哨兵必须是黑色")
        if self._root.color != BLACK and self._root is not nil:
            raise AssertionError("根节点必须是黑色")
        if self._root.parent is not nil:
            raise AssertionError("根节点的 parent 必须是 NIL")

        def _check(node: Node) -> int:
            if node is nil:
                return 1
            if node.color == RED:
                if node.left.color == RED or node.right.color == RED:
                    raise AssertionError(f"红色节点 {node.key} 有红色子节点")
            if node.left is not nil and node.left.parent is not node:
                raise AssertionError(f"{node.left.key} 的父指针错误")
            if node.right is not nil and node.right.parent is not node:
                raise AssertionError(f"{node.right.key} 的父指针错误")
            left_bh = _check(node.left)
            right_bh = _check(node.right)
            if left_bh != right_bh:
                raise AssertionError(f"节点 {node.key} 两侧黑高不等：{left_bh} vs {right_bh}")
            return left_bh + (0 if node.color == RED else 1)

        return _check(self._root)

    def __repr__(self) -> str:
        return f"RedBlackTree(size={self._size}, height={self.height()})"


def _demo() -> None:
    tree = RedBlackTree()
    for k in [41, 38, 31, 12, 19, 8]:
        tree.insert(k, f"v{k}")
    print("插入后:", tree)
    print("key 列表:", tree.keys())
    print("校验通过，黑高 =", tree.validate())

    tree[50] = "v50"
    tree[38] = "new38"  # 更新
    print("更新 38 ->", tree[38])

    for k in [8, 12, 19, 31, 38, 41, 50]:
        print(f"  {k} 的前驱={tree.predecessor(k)}, 后继={tree.successor(k)}")

    print("区间 [19, 41]:", tree.range_query(19, 41))

    for k in [41, 38, 31]:
        tree.delete(k)
        tree.validate()
    print("删除 41/38/31 后:", tree, tree.keys())

    # 随机压力测试
    import random

    rnd = random.Random(2026)
    t = RedBlackTree()
    ref = {}
    for _ in range(3000):
        op = rnd.random()
        k = rnd.randint(0, 500)
        if op < 0.6:
            t[k] = k * 10
            ref[k] = k * 10
        elif op < 0.9:
            assert t.delete(k) == (k in ref)
            ref.pop(k, None)
        else:
            assert t.search(k) == ref.get(k)
        assert len(t) == len(ref)
        t.validate()
    assert t.keys() == sorted(ref)
    assert t.validate() >= 1
    print("随机测试通过:", t)


if __name__ == "__main__":
    _demo()
