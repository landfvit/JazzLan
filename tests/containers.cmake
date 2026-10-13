#container behavior is exercised through complete JazzLan programs
#unordered set assertions do not prescribe iteration order
add_jazz_test(container_literals [=[
print([]);
print(<>);
print({});
print(list());
print(set());
print(dict());
print((1));
print(<1>);
print(<1, 1.0, true>);
print([1, "Jazz", true, none, [2], <3>, {"key": 4}, <5>]);
print(1 < 2);
print(3 > 2);
print(<1, 2> == set([2, 1]));
]=] [=[[]
<>
{}
[]
<>
{}
1
<1>
<1>
[1, "Jazz", true, none, [2], <3>, {"key": 4}, <5>]
true
true
true
]=] "")

add_jazz_test(container_constructors [=[
print(list("Jazz"));
print(list([1, true, none]));
print(set([1, true, 1.0, 2]) == <1, 2>);
print(set("aba") == <"a", "b">);
print(set(<1, 2, 1>) == <2, 1>);
print(dict([["a", 1], ["b", 2], ["a", 3]]));
var mapping = {"a": 1, "b": 2};
print(list(mapping));
print(set(mapping) == <"a", "b">);
print(dict(mapping));
print(set(<1, 2>) == <2, 1>);
print(set(list(<1, 2>)) == <1, 2>);
]=] [=[["J", "a", "z", "z"]
[1, true, none]
true
true
true
{"a": 3, "b": 2}
["a", "b"]
true
{"a": 1, "b": 2}
true
true
]=] "")

add_jazz_test(container_constructor_shallow_copies [=[
var child = [1];
var original = [child];
var duplicate = list(original);
duplicate.append(2);
child.append(3);
print(original);
print(duplicate);
var mapping = {"child": child};
var other = dict(mapping);
other["extra"] = 4;
print(mapping);
print(other);
var elements = <1>;
var otherElements = set(elements);
otherElements.add(2);
print(elements == <1>);
print(otherElements == <1, 2>);
]=] [=[[[1, 3]]
[[1, 3], 2]
{"child": [1, 3]}
{"child": [1, 3], "extra": 4}
true
true
]=] "")

add_jazz_test(container_annotations [=[
var values: list = [1];
const pair: list = [1, "two"];
var elements: set = <1, 2>;
var mapping: dict = {"a": 1};
function identity(value: list): list { return value; }
function asSet(value): set { return set(value); }
function asDict(value): dict { return dict(value); }
print(identity(values));
print(asSet(values) == <1>);
print(asDict(mapping));
print(pair);
print(elements == <2, 1>);
]=] [=[[1]
true
{"a": 1}
[1, "two"]
true
]=] "")

add_jazz_test(container_indexing [=[
var values = [10, 20, 30];
print(values[0]);
print(values[-1]);
print(values[-3]);
print(values[true]);
print(values[false]);
print(["a", "b"][-1]);
print("Jazz"[-1]);
print({"a": [1, 2]}["a"][1]);
]=] [=[10
30
10
20
10
b
z
2
]=] "")

add_jazz_test(container_index_assignment [=[
var values = [1, 2, [3]];
values[-3] = "one";
values[true] = none;
values[-1][0] = false;
print(values);
var mapping = {"a": 1};
mapping["b"] = [2];
mapping["a"] = "changed";
mapping["b"][0] = 3;
print(mapping);
]=] [=[["one", none, [false]]
{"a": "changed", "b": [3]}
]=] "")

add_jazz_test(container_list_slices [=[
var values = [0, 1, 2, 3, 4];
print(values[:]);
print(values[1:4]);
print(values[:3]);
print(values[3:]);
print(values[::2]);
print(values[::-1]);
print(values[4:0:-2]);
print(values[-4:-1]);
print(values[-100:100]);
print(values[3:1]);
print(values[:-1:-1]);
var copy = values[:];
copy[0] = 99;
print(values[0]);
]=] [=[[0, 1, 2, 3, 4]
[1, 2, 3]
[0, 1, 2]
[3, 4]
[0, 2, 4]
[4, 3, 2, 1, 0]
[4, 2]
[1, 2, 3]
[0, 1, 2, 3, 4]
[]
[]
0
]=] "")

add_jazz_test(container_string_slices [=[
print("Jazz"[::-1]);
print("abcdef"[1:5:2]);
print("abcdef"[-3:]);
print("abcdef"[:2]);
]=] [=[zzaJ
bd
def
ab
]=] "")

add_jazz_test(container_length_membership [=[
print(len([]));
print(len(<1, 2>));
print(len(<1, 1.0, true>));
print(len({"a": 1, "b": 2}));
print(len("Jazz"));
print(true in [1]);
print(2 not in <1>);
print(1.0 in <true>);
print("a" in {"a": 1});
print(1 in {"a": 1});
print("az" in "Jazz");
print("z" not in "Jazz");
print([1] in [[1], [2]]);
]=] [=[0
2
1
2
4
true
true
true
true
false
true
false
true
]=] "")

add_jazz_test(container_list_append_extend_insert [=[
var values = [2];
print(values.append(3));
print(values.extend([4, 5]));
print(values.insert(0, 1));
print(values.insert(-100, 0));
print(values.insert(100, 6));
print(values.insert(-1, "before-last"));
print(values);
var self = [1, 2];
self.extend(self);
print(self);
]=] [=[none
none
none
none
none
none
[0, 1, 2, 3, 4, 5, "before-last", 6]
[1, 2, 1, 2]
]=] "")

add_jazz_test(container_list_pop_remove [=[
var values = [1, 2, true, 3];
print(values.pop());
print(values.pop(-2));
print(values.remove(1.0));
print(values);
print(values.pop(false));
print(values);
]=] [=[3
2
none
[true]
true
[]
]=] "")

add_jazz_test(container_list_copy_clear [=[
var child = [1];
var original = [child];
var copy = original.copy();
copy.append(2);
child.append(3);
print(original);
print(copy);
print(copy.clear());
print(copy);
print(original);
]=] [=[[[1, 3]]
[[1, 3], 2]
none
[]
[[1, 3]]
]=] "")

add_jazz_test(container_list_count_index [=[
var values = [1, "a", true, 1.0, [1], [1]];
print(values.count(true));
print(values.count([1]));
print(values.count("missing"));
print(values.index(1.0));
print(values.index([1]));
print(values.index(true, 1));
print(values.index(1, -3, -2));
]=] [=[3
2
0
0
4
2
3
]=] "")

add_jazz_test(container_list_reverse_sort [=[
var values = [3, 1, 2];
print(values.reverse());
print(values);
print(values.sort());
print(values);
var words = ["z", "a", "Jazz"];
words.sort();
print(words);
var large = [9007199254740993, 9007199254740992, 1, 0, false, true];
large.sort();
print(large);
]=] [=[none
[2, 1, 3]
none
[1, 2, 3]
["Jazz", "a", "z"]
[0, false, 1, true, 9007199254740992, 9007199254740993]
]=] "")

add_jazz_test(container_const_list_read_methods [=[
const values = [1, true, 1.0, "a", [2]];
print(values.count(1));
print(values.count([2]));
print(values.index(true));
print(values.index(1, 1));
print(values.index(1, -3, -2));
print(values);
print([1, 2] < [1, 3]);
print([1] < [1, 0]);
]=] [=[3
1
0
1
2
[1, true, 1, "a", [2]]
true
true
]=] "")

add_jazz_test(container_set_mutators [=[
var values = <1>;
print(values.add(true));
print(values.add(2));
print(values.remove(1.0));
print(values.discard(99));
print(values == <2>);
print(values.pop());
print(len(values));
print(values.update([1, 2], <2, 3>));
print(values == <1, 2, 3>);
var copy = values.copy();
print(copy.clear());
print(len(copy));
print(values == <1, 2, 3>);
]=] [=[none
none
none
none
true
2
0
none
true
none
0
true
]=] "")

add_jazz_test(container_set_algebra_methods [=[
var values = <1, 2, 3>;
print(values.union([3, 4], <5>) == <1, 2, 3, 4, 5>);
print(values.intersection([2, 3, 4], <3, 5>) == <3>);
print(values.difference([2], <3>) == <1>);
print(values.symmetric_difference([3, 4]) == <1, 2, 4>);
print(values == <1, 2, 3>);
]=] [=[true
true
true
true
true
]=] "")

add_jazz_test(container_set_operators [=[
var left = <1, 2>;
var right = <2, 3>;
print((left | right) == <1, 2, 3>);
print((left & right) == <2>);
print((left - right) == <1>);
print((left ^ right) == <1, 3>);
print(<1> < left);
print(<1> <= left);
print(left <= <2, 1>);
print(left < <2, 1>);
print(left >= <1>);
print(left > <1>);
print(left < right);
print(<> < <1> < <1, 2>);
]=] [=[true
true
true
true
true
true
true
false
true
true
false
true
]=] "")

add_jazz_test(container_dict_numeric_keys [=[
var values = {true: "first", 1: "second", 1.0: "last", "x": 2, false: 3, 0: 4};
print(values);
print(values.keys());
print(values.values());
print(values.items());
print(values[1]);
print(values[0.0]);
values[true] = "updated";
print(values.keys());
print(len(values));
]=] [=[{true: "last", "x": 2, false: 4}
[true, "x", false]
["last", 2, 4]
[[true, "last"], ["x", 2], [false, 4]]
last
4
[true, "x", false]
3
]=] "")

add_jazz_test(container_dict_readers [=[
var values = {"a": none, "b": 2};
print(values.get("a", 99));
print(values.get("missing"));
print(values.get("missing", 99));
var keys = values.keys();
var items = values.items();
values["c"] = 3;
print(keys);
print(items);
print(values.values());
]=] [=[none
none
99
["a", "b"]
[["a", none], ["b", 2]]
[none, 2, 3]
]=] "")

add_jazz_test(container_dict_pop_copy_clear [=[
var child = [1];
var values = {"a": child, "b": 2, "c": 3};
var copy = values.copy();
child.append(4);
print(copy);
print(values.pop("missing", 99));
print(values.pop("b"));
values["b"] = 5;
print(values.keys());
print(values.popitem());
print(values.popitem());
print(values.clear());
print(values);
print(copy);
]=] [=[{"a": [1, 4], "b": 2, "c": 3}
99
2
["a", "c", "b"]
["b", 5]
["c", 3]
none
{}
{"a": [1, 4], "b": 2, "c": 3}
]=] "")

add_jazz_test(container_dict_update_setdefault [=[
var values = {"a": 1};
print(values.setdefault("a", 99));
print(values.setdefault("b"));
print(values.setdefault("c", []));
values["c"].append(3);
print(values.update({"a": 2, "d": 4}));
print(values.update([["b", false], ["e", 5]]));
print(values.update(values));
print(values);
]=] [=[1
none
[]
none
none
none
{"a": 2, "b": false, "c": [3], "d": 4, "e": 5}
]=] "")

add_jazz_test(container_alias_function_mutation [=[
function change(values: list, mapping: dict): none {
    values.append(2);
    mapping["changed"] = true;
    return none;
}
var values = [1];
var alias = values;
var mapping = {"values": values};
change(alias, mapping);
print(values);
print(mapping);
var child = [3];
const frozen = [child];
print(frozen);
var copy = child.copy();
copy.append(4);
print(copy);
print(child);
]=] [=[[1, 2]
{"values": [1, 2], "changed": true}
[[3]]
[3, 4]
[3]
]=] "")

add_jazz_test(container_scalar_keys [=[
var mapping = {none: "empty", true: "first", 1.0: "latest", "1": "text"};
print(len(mapping));
print(mapping[1]);
print(mapping[none]);
print(mapping.keys());
print(len(<none, 1, true>));
print(none in <none>);
]=] [=[3
latest
empty
[none, true, "1"]
2
true
]=] "")

add_jazz_test(container_structural_equality [=[
print([1, {"a": [2]}] == [true, {"a": [2.0]}]);
print([1, [2]] == [1.0, [2.0]]);
print([1, 2] == <1, 2>);
print(<1, 2> == <2.0, true>);
print({"a": 1, "b": [2]} == {"b": [2.0], "a": true});
print({1: "a"} == {true: "a"});
print({"a": 1} != {"a": 2});
print([1, 2] < [1, 3]);
print([1] < [1, 0]);
]=] [=[true
true
false
true
true
true
true
true
true
]=] "")

add_jazz_test(container_numeric_equality_precision [=[
print(1 == 1.0 == true);
print(0 == false);
print(0.0 == false);
print(none == false);
print(9007199254740993 == 9007199254740992.0);
print(9007199254740992 == 9007199254740992.0);
print(9223372036854775807 == 9223372036854775808.0);
print(len(<9007199254740993, 9007199254740992.0, 9007199254740992>));
var mapping = {9007199254740993: "odd", 9007199254740992.0: "even"};
print(mapping[9007199254740993]);
print(mapping[9007199254740992]);
]=] [=[true
true
true
false
false
true
false
2
odd
even
]=] "")

add_jazz_test(container_numeric_ordering_precision [=[
print(9007199254740993 > 9007199254740992.0);
print(9007199254740992.0 < 9007199254740993);
print(9223372036854775807 < 9223372036854775808.0);
print(-9007199254740993 < -9007199254740992.0);
print(false < true);
print(true <= 1.0);
print([9007199254740993] > [9007199254740992.0]);
]=] [=[true
true
true
true
true
true
true
]=] "")

add_jazz_test(container_comparison_chains [=[
var calls = [];
function mark(value) {
    calls.append(value);
    return value;
}
print(mark(1) < mark(2) < mark(3));
print(calls);
calls.clear();
print(mark(3) < mark(2) < mark(1));
print(calls);
print(3 < 2 < [][0]);
print(0 < 1 == true);
print(1 == true in [1]);
print(1 not in [] == []);
]=] [=[true
[1, 2, 3]
false
[3, 2]
false
true
true
true
]=] "")

#const freezes existing aliases and nested mutable children
add_jazz_test(container_frozen_list_alias [=[
var values = [1];
var alias = values;
const fixed = values;
alias.append(2);
]=] "" "Cannot modify frozen list")

add_jazz_test(container_frozen_list_index [=[
var values = [1];
const fixed = values;
values[0] = 2;
]=] "" "Cannot modify frozen list")

add_jazz_test(container_frozen_nested_child [=[
var child = [1];
const fixed = [{"child": [child]}];
child.clear();
]=] "" "Cannot modify frozen list")

add_jazz_test(container_frozen_dict_alias [=[
var values = {"a": 1};
var alias = values;
const fixed = [values];
alias["a"] = 2;
]=] "" "Cannot modify frozen dict")

add_jazz_test(container_frozen_set_function [=[
function change(value: set) { value.add(2); }
var values = <1>;
const fixed = {"values": [values]};
change(values);
]=] "" "Cannot modify frozen set")

add_jazz_test(container_frozen_cycle [=[
var values = [];
values.append(values);
const fixed = values;
values.append(1);
]=] "" "Cannot modify frozen list")

#freezing a mutable container does not make it hashable
foreach(containerType list set dict)
    add_jazz_test(container_unhashable_${containerType}
        "var value = ${containerType}(); set([value]);"
        "" "Unhashable type: ${containerType}"
    )
    add_jazz_test(container_unhashable_frozen_${containerType}
        "const value = ${containerType}(); var mapping = {value: 1};"
        "" "Unhashable type: ${containerType}"
    )
endforeach()

add_jazz_test(container_unhashable_nested_list
    "var value = [1, []]; set([value]);"
    "" "Unhashable type: list"
)

add_jazz_test(container_set_index_unsupported
    "var value = <1>; print(value[0]);"
    "" "Set does not support indexing"
)
add_jazz_test(container_index_out_of_range
    "print([1][-2]);"
    "" "Index out of range"
)
add_jazz_test(container_index_requires_integer
    "print([1][0.0]);"
    "" "Index must be int or bool"
)
add_jazz_test(container_slice_zero_step
    "print([1, 2][::0]);"
    "" "Slice step cannot be zero"
)
add_jazz_test(container_dict_missing_key
    "print({}[\"missing\"]);"
    "" "Key not found: missing"
)
add_jazz_test(container_list_remove_missing
    "var value = [1]; value.remove(2);"
    "" "Value not found"
)
add_jazz_test(container_dict_malformed_pair
    "dict([[1, 2, 3]]);"
    "" "Dictionary entries must contain two values"
)
add_jazz_test(container_numeric_equality_preserves_exact_annotations
    "function value(input: int) {} value(true);"
    "" "Argument type mismatch: value (parameter input, expected int, got bool)"
)
add_jazz_test(container_assignment_preserves_fixed_type
    "var value = [1]; value = <1>;"
    "" "Type mismatch: value"
)

#explicit none bounds match omitted bounds, including descending slices
add_jazz_test(container_slice_none_and_extreme_steps [=[
var values = [0, 1, 2, 3, 4];
var smallest = -(9223372036854775807) - 1;
print(-9223372036854775808);
print(values[none:none:none]);
print(values[none:none:-1]);
print(values[none:3:none]);
print([0, 1, 2][none:none:-1]);
print("Jazz"[none:none:none]);
print(values[::9223372036854775807]);
print(values[::-9223372036854775807]);
print(values[::smallest]);
print(values[none:none:-9223372036854775808]);
print(values[4:0:smallest]);
print(values[1:5:9223372036854775807]);
print(values[4:-9223372036854775808:-1]);
print(values[-9223372036854775808:9223372036854775807]);
print([][::smallest]);
]=] [=[-9223372036854775808
[0, 1, 2, 3, 4]
[4, 3, 2, 1, 0]
[0, 1, 2]
[2, 1, 0]
Jazz
[0]
[4]
[4]
[4]
[4]
[1]
[4, 3, 2, 1, 0]
[0, 1, 2, 3, 4]
[]
]=] "")

#recursive containers print and compare without infinite recursion
add_jazz_test(container_recursive_structure [=[
var left = [];
left.append(left);
var right = [];
right.append(right);
print(left);
print(left == left);
print(left == right);
left.append(1);
print(left);
print(left == right);
right.append(true);
print(left == right);
var first = {};
first["self"] = first;
var second = {};
second["self"] = second;
print(first);
print(first == second);
first["value"] = 1;
second["value"] = true;
print(first);
print(first == second);
second["value"] = 2;
print(first == second);
]=] [=[[[...]]
true
true
[[...], 1]
false
true
{"self": {...}}
true
{"self": {...}, "value": 1}
true
false
]=] "")

#string iteration, indexing and slicing operate on unicode codepoints
add_jazz_test(container_utf8_codepoints [=[
var text = "Příliš";
print(len(text));
print(text[1]);
print(text[-1]);
print(list(text));
print(set(text) == <"P", "ř", "í", "l", "i", "š">);
print(text[1:3]);
print(text[::-1]);
print(text[::2]);
print("ří" in text);
print(len(set(text)));
]=] [=[6
ř
š
["P", "ř", "í", "l", "i", "š"]
true
ří
šilířP
Píi
true
6
]=] "")

#unordered floating values must not invalidate sorting or lose elements
add_jazz_test(container_sort_nan_keeps_elements [=[
var large = 2.0;
var count = 0;
while (count < 16) {
    large = large * large;
    count = count + 1;
}
var unordered = large - large;
var values = [unordered, 3, 1, unordered, 2, large, -large];
print(values.sort());
print(len(values));
print(1 in values);
print(2 in values);
print(3 in values);
]=] [=[none
7
true
true
true
]=] "")

add_jazz_test(container_set_update_and_boolean_methods [=[
var values = <1, 2, 3>;
var alias = values;
print(values.intersection_update([2, 3, 4]));
print(alias == <2, 3>);
print(values.difference_update(<2>));
print(alias == <3>);
print(values.symmetric_difference_update([3, 4, 4]));
print(alias == <4>);
print(values.issubset([4, 5]));
print(values.issuperset(<4>));
print(values.isdisjoint([1, 2]));
print(values.isdisjoint([4]));
print(values.issubset({4: "value"}));
print(values.update());
print(values.intersection_update());
print(values.difference_update());
print(values == <4>);
print(values.difference_update(values));
print(alias == <>);
]=] [=[none
true
none
true
none
true
true
true
true
false
true
none
none
none
true
none
true
]=] "")

add_jazz_test(container_sequence_concatenation_repetition [=[
print([1] + ["two"]);
print(2 * [1, 2]);
print([1, "a"] * 2);
print([1] * 0);
print([1] * -2);
print([1] * true);
print(false * [1]);
print([] * 9223372036854775807);
var child = [1];
var original = [child];
var repeated = original * 2;
repeated[0].append(2);
repeated.append("extra");
print(original);
print(repeated);
]=] [=[[1, "two"]
[1, 2, 1, 2]
[1, "a", 1, "a"]
[]
[]
[1]
[]
[]
[[1, 2]]
[[1, 2], [1, 2], "extra"]
]=] "")

#every mutator rejects frozen objects, including methods that would be no-ops
foreach(method append extend insert pop remove clear reverse sort)
    set(arguments "")
    if(method STREQUAL "append")
        set(arguments "2")
    elseif(method STREQUAL "extend")
        set(arguments "[]")
    elseif(method STREQUAL "insert")
        set(arguments "0, 2")
    elseif(method STREQUAL "remove")
        set(arguments "1")
    endif()
    add_jazz_test(container_frozen_list_method_${method}
        "var values = [1]; const fixed = values; values.${method}(${arguments});"
        "" "Cannot modify frozen list"
    )
endforeach()

foreach(method add remove discard pop clear update intersection_update difference_update symmetric_difference_update)
    set(arguments "")
    if(method STREQUAL "add")
        set(arguments "2")
    elseif(method STREQUAL "remove")
        set(arguments "1")
    elseif(method STREQUAL "discard")
        set(arguments "99")
    elseif(method STREQUAL "update")
        set(arguments "<2>")
    elseif(method STREQUAL "intersection_update")
        set(arguments "<1>")
    elseif(method STREQUAL "difference_update" OR method STREQUAL "symmetric_difference_update")
        set(arguments "<>")
    endif()
    add_jazz_test(container_frozen_set_method_${method}
        "var values = <1>; const fixed = values; values.${method}(${arguments});"
        "" "Cannot modify frozen set"
    )
endforeach()

foreach(method pop popitem setdefault update clear)
    set(arguments "")
    if(method STREQUAL "pop")
        set(arguments "\"a\"")
    elseif(method STREQUAL "setdefault")
        set(arguments "\"a\", 1")
    elseif(method STREQUAL "update")
        set(arguments "{}")
    endif()
    add_jazz_test(container_frozen_dict_method_${method}
        "var values = {\"a\": 1}; const fixed = values; values.${method}(${arguments});"
        "" "Cannot modify frozen dict"
    )
endforeach()

#keeps the user-facing container example executable
file(READ "${PROJECT_SOURCE_DIR}/examples/containers.jazz" containerExampleSource)
add_jazz_test(containers_example "${containerExampleSource}" [=[
[1, "Jazz", none, true]
true
["Jazz", none]
[10, 20]
<1>
2
true
true
true
{"name": "Jazz", "scores": [3, 5, 8]}
true
[3, 5, 8]
[3, 5, 8, 10]
[3, 5, 8]
]=] "")

#sets cannot be indexed or sliced, and tuple is no longer a language type
add_jazz_test(container_set_slice_unsupported
    "print(<1, 2>[:]);"
    "" "Value does not support slicing"
)
add_jazz_test(container_removed_tuple_type
    "var value: tuple;"
    "" "Unknown type: tuple"
)
add_jazz_test(container_removed_tuple_constructor
    "tuple();"
    "" "Undefined function: tuple"
)
add_jazz_test(container_removed_parenthesized_empty
    "print(());"
    "" "Expected expression"
)
add_jazz_test(container_removed_parenthesized_comma
    "print((1, 2));"
    "" "Expected ')'"
)

add_jazz_test(container_removed_parenthesized_singleton
    "print((1,));"
    "" "Expected ')'"
)
