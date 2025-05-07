def find_unique_elements(list1, list2):
    # 找出 list1 中有但 list2 中没有的元素
    unique_in_list1 = list(set(list1) - set(list2))
    # 找出 list2 中有但 list1 中没有的元素
    unique_in_list2 = list(set(list2) - set(list1))
    return unique_in_list1, unique_in_list2

list1 = []
list2 = []

unique_in_list1, unique_in_list2 = find_unique_elements(list1, list2)

print("在 list1 中有但在 list2 中没有的元素:", unique_in_list1)
print("在 list2 中有但在 list1 中没有的元素:", unique_in_list2)