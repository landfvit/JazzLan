#in-place transforms keep the shared source container
add_jazz_test(transform_list_in_place [=[
function double(value: int): int { return value * 2; }
var values = [1, 2, 3];
values [| double(#) |];
print(values);
values [| # + 1 |];
print(values);
]=] [=[[2, 4, 6]
[3, 5, 7]
]=] "")

add_jazz_test(transform_list_new_target [=[
var values = [1, 2, 3];
values [| 2 * # |] doubled;
doubled.append(8);
print(values);
print(doubled);
]=] [=[[1, 2, 3]
[2, 4, 6, 8]
]=] "")

#an expression can ignore the current element and takes only explicit arguments
add_jazz_test(transform_constant_expression [=[
function constant(): int { return 7; }
var values = [1, 2];
values [| constant() |];
print(values);
values [| none |] emptyValues;
print(emptyValues);
]=] [=[[7, 7]
[none, none]
]=] "")

add_jazz_test(transform_full_expressions [=[
function mixed(value) {
    if (value == 1) { return "one"; }
    if (value == 2) { return true; }
    return [value];
}
var numbers = [1, 2, 3];
numbers [| mixed(#) |];
print(numbers);
var rows = [[1, 2], [3]];
rows [| {"first": #[0], "last": #[-1], "copy": #.copy(), "head": #[:1]} |] records;
print(records);
rows [| #[0] + len(#) |] totals;
print(totals);
]=] [=[["one", true, [3]]
[{"first": 1, "last": 2, "copy": [1, 2], "head": [1]}, {"first": 3, "last": 3, "copy": [3], "head": [3]}]
[3, 4]
]=] "")

#empty inputs preserve their kind and never evaluate the transform expression
add_jazz_test(transform_empty_sources [=[
var values = [];
values [| missing() |];
values [| missing() |] mappedList;
var elements = <>;
elements <| missing() |>;
elements <| missing() |> mappedSet;
var mapping = {};
mapping {| missing() |};
mapping {| missing() |} mappedDict;
print(values);
print(mappedList);
print(mappedSet);
print(mappedDict);
]=] [=[[]
[]
<>
{}
]=] "")

add_jazz_test(transform_source_evaluated_once [=[
var calls = [];
var values = [1, 2];
function source(): list {
    calls.append(1);
    return values;
}
source()[|#+1|];
print(values);
source()[|#*2|]mapped;
print(values);
print(mapped);
print(len(calls));
]=] [=[[2, 3]
[2, 3]
[4, 6]
2
]=] "")

#nested transforms restore the outer placeholder after the callback returns
add_jazz_test(transform_nested_placeholder_context [=[
function nested(value: int): int {
    var inner = [value, value + 1];
    inner [| 10 + # |] mapped;
    return mapped[0] + mapped[1];
}
var values = [1, 2];
values [| nested(#) + # |] mapped;
print(mapped);
print(values);
]=] [=[[24, 27]
[1, 2]
]=] "")

add_jazz_test(transform_in_place_aliases [=[
var values = [1, 2];
var alias = values;
var mapping = {"values": values};
alias [| # * 3 |];
print(values);
print(mapping);
mapping["values"][|#-1|];
print(alias);
]=] [=[[3, 6]
{"values": [3, 6]}
[2, 5]
]=] "")

#the new outer container retains references returned by the expression
add_jazz_test(transform_new_target_shared_children [=[
var child = [1];
var values = [child];
values [| # |] mapped;
mapped.append([3]);
mapped[0].append(2);
print(values);
print(mapped);
]=] [=[[[1, 2]]
[[1, 2], [3]]
]=] "")

add_jazz_test(transform_target_shadows_outer_scope [=[
var values = [1, 2];
var result = [99];
if (true) {
    values [| # * 2 |] result;
    print(result);
    values [| # + 10 |] values;
    print(values);
}
print(values);
print(result);
]=] [=[[2, 4]
[11, 12]
[1, 2]
[99]
]=] "")

#callbacks can mutate the source while iteration uses its original snapshot
add_jazz_test(transform_callback_source_mutation [=[
var values = [1, 2];
function visit(value: int): int {
    values.append(value + 10);
    return value * 2;
}
values [| visit(#) |] mapped;
print(values);
print(mapped);
]=] [=[[1, 2, 11, 12]
[2, 4]
]=] "")

add_jazz_test(transform_set_preserves_kind_and_deduplicates [=[
var values = <1, 2, 3>;
var alias = values;
values<|#-#|>mapped;
print(values == <1, 2, 3>);
print(mapped == <0>);
mapped.add(4);
print(mapped == <0, 4>);
values<|#-#|>;
print(values == <0>);
print(alias == <0>);
]=] [=[true
true
true
true
true
]=] "")

add_jazz_test(transform_dict_values_preserve_keys [=[
var values = {true: 1, "b": 2};
var alias = values;
values{|#*10|}mapped;
print(values);
print(mapped);
mapped["c"] = 30;
values{|[#,#+1]|};
print(values);
print(alias);
print(mapped);
]=] [=[{true: 1, "b": 2}
{true: 10, "b": 20}
{true: [1, 2], "b": [2, 3]}
{true: [1, 2], "b": [2, 3]}
{true: 10, "b": 20, "c": 30}
]=] "")

#frozen inputs can produce new mutable containers of the same kind
add_jazz_test(transform_frozen_new_targets [=[
const values = [1, 2];
values [| # * 2 |] mappedList;
mappedList.append(6);
print(values);
print(mappedList);
const elements = <1, 2>;
elements <| # + 10 |> mappedSet;
mappedSet.add(13);
print(elements == <1, 2>);
print(mappedSet == <11, 12, 13>);
const mapping = {"a": 1};
mapping {| # + 1 |} mappedDict;
mappedDict["b"] = 3;
print(mapping);
print(mappedDict);
]=] [=[[1, 2]
[2, 4, 6]
true
true
{"a": 1}
{"a": 2, "b": 3}
]=] "")

#no automatic element replacement is visible before all callbacks succeed
foreach(mode in_place new_target)
    set(target "")
    if(mode STREQUAL "new_target")
        set(target "mapped")
    endif()
    set(source [=[
var values = [1, 2, 3];
function visit(value: int): int {
    print(values);
    if (value == 3) { return [][0]; }
    return value * 10;
}
]=])
    string(APPEND source "values [| visit(#) |] ${target};")
    add_jazz_test(transform_failure_atomic_${mode}
        "${source}" "[1, 2, 3]\n[1, 2, 3]\n[1, 2, 3]\n"
        "Index out of range"
    )
endforeach()

add_jazz_test(transform_callback_side_effects_before_failure [=[
var values = [1, 2];
function visit(value: int): int {
    values.append(value + 10);
    if (value == 2) {
        print(values);
        return [][0];
    }
    return value * 2;
}
values [| visit(#) |] mapped;
]=] "[1, 2, 11, 12]\n" "Index out of range")

add_jazz_test(transform_no_implicit_argument
    "function double(value: int): int { return value * 2; } var values = [1]; values [| double() |];"
    "" "Invalid argument count: double"
)
add_jazz_test(transform_void_callback
    "function empty(): void {} var values = [1]; values [| empty() |];"
    "" "Function returned no value: empty"
)
add_jazz_test(transform_exact_callback_annotation
    "function double(value: float): float { return value * 2.0; } var values = [1]; values [| double(#) |];"
    "" "Argument type mismatch: double (parameter value, expected float, got int)"
)
add_jazz_test(transform_requires_container
    "42 [| # |] mapped;"
    "" "Transform requires a container"
)
add_jazz_test(transform_placeholder_outside
    "print(#);"
    "" "Placeholder '#' is only allowed inside a transform"
)
add_jazz_test(transform_duplicate_target
    "var values = [1]; var mapped = [99]; values [| # * 2 |] mapped;"
    "" "Variable already defined: mapped"
)

foreach(mode in_place new_target)
    set(target "")
    if(mode STREQUAL "new_target")
        set(target "mapped")
    endif()
    add_jazz_test(transform_set_unhashable_${mode}
        "var values = <1>; values <| [#] |> ${target};"
        "" "Unhashable type: list"
    )
endforeach()

#delimiter pairs belong to the source container type
set(transformOpen_list "[|")
set(transformClose_list "|]")
set(transformLiteral_list "[1]")
set(transformOpen_set "<|")
set(transformClose_set "|>")
set(transformLiteral_set "<1>")
set(transformOpen_dict "{|")
set(transformClose_dict "|}")
set(transformLiteral_dict "{\"a\": 1}")

#mutation is rejected before even the first callback can execute
foreach(containerType list set dict)
    add_jazz_test(transform_frozen_in_place_${containerType}
        "function visit(value) { print(\"called\"); return value; } const values = ${transformLiteral_${containerType}}; values ${transformOpen_${containerType}} visit(#) ${transformClose_${containerType}};"
        "" "Cannot modify frozen ${containerType}"
    )
endforeach()

add_jazz_test(transform_frozen_child_remains_frozen
    "const values = [[1]]; values [| # |] mapped; mapped[0].append(2);"
    "" "Cannot modify frozen list"
)

#callbacks may freeze the source after the initial mutability check
add_jazz_test(transform_callback_freezes_source [=[
var values = [1, 2];
function freeze(value: int): int {
    const fixed = values;
    print(value);
    return value * 2;
}
values [| freeze(#) |];
]=] "1\n2\n" "Cannot modify frozen list")

#transform delimiters remain distinct from set operators and comparisons
add_jazz_test(transform_adjacent_tokens_and_precedence [=[
var values=[<1>,<2>];
values[|#|<3>|]mapped;
print(mapped == [<1,3>,<2,3>]);
print(values == [<1>,<2>]);
var numbers=[0,1,2];
numbers[|#>1|]greater;
print(greater);
numbers[|1+#*2|]arithmetic;
print(arithmetic);
numbers[|# in <1,2>|]selected;
print(selected);
var nested=[[1,2]];
nested[0][|#*2|];
print(nested);
nested[0].copy()[|#+1|]copied;
print(copied);
print(nested);
]=] [=[true
true
[false, false, true]
[1, 3, 5]
[false, true, true]
[[2, 4]]
[3, 5]
[[2, 4]]
]=] "")

add_jazz_test(transform_missing_closing_delimiter
    "var values = [1]; values [| # + 1;"
    "" "Expected '|]' after transform expression"
)
add_jazz_test(transform_empty_expression
    "var values = [1]; values [| |];"
    "" "Expected expression"
)
add_jazz_test(transform_target_rejects_var_keyword
    "var values = [1]; values [| # |] var mapped;"
    "" "Expected ';'"
)

#all mismatched source and operator pairs fail before the callback is evaluated
foreach(sourceType list set dict)
    foreach(operatorType list set dict)
        if(NOT sourceType STREQUAL operatorType)
            set(literal "${transformLiteral_${sourceType}}")
            if(sourceType STREQUAL "list" AND operatorType STREQUAL "set")
                set(literal "[]")
            endif()
            add_jazz_test(transform_operator_${operatorType}_rejects_${sourceType}
                "function visit(value) { print(\"called\"); return value; } var values = ${literal}; values ${transformOpen_${operatorType}} visit(#) ${transformClose_${operatorType}} mapped;"
                "" "Transform expects ${operatorType}, got ${sourceType}"
            )
        endif()
    endforeach()
endforeach()

#each opener requires its own matching closer
set(transformWrongClose_list "|>")
set(transformWrongClose_set "|}")
set(transformWrongClose_dict "|]")
foreach(containerType list set dict)
    add_jazz_test(transform_mismatched_closing_${containerType}
        "var values = ${transformLiteral_${containerType}}; values ${transformOpen_${containerType}} # ${transformWrongClose_${containerType}} mapped;"
        "" "Expected '${transformClose_${containerType}}' after transform expression"
    )
endforeach()

#keeps the user-facing transform example executable
file(READ "${PROJECT_SOURCE_DIR}/examples/transforms.jazz" transformExampleSource)
add_jazz_test(transforms_example "${transformExampleSource}" [=[
[1, 2, 3, 4]
[2, 4, 6, 8]
[2, 4, 6, 8]
[3, 5]
[4, 6]
<0>
{"Jazz": 3, "Lu": 5}
{"Jazz": 30, "Lu": 50}
[[1], [2]]
[[1, 3], [2]]
]=] "")

#the removed parenthesis transform operator is no longer accepted
add_jazz_test(transform_removed_parenthesis_operator
    "var values = <1>; values (| # |);"
    "" "Expected expression"
)
