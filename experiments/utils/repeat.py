def has_duplicates(lst):
    return len(lst) != len(set(lst))

my_list = []
print("有重复元素" if has_duplicates(my_list) else "没有重复元素")