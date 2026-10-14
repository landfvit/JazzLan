#each iteration gets a fresh variable and body scope
add_jazz_test(for_heterogeneous_fresh_scope [=[
for (var item in [1, "Jazz", true, none, [2]]) {
    var local = item;
    print(local);
}
]=] "1\nJazz\ntrue\nnone\n[2]\n" "")

add_jazz_test(for_typed_variable_and_shadowing [=[
var item = 99;
var values = [1, 2];
var results = [];
for (var item: int in values) {
    function current(): int { return item; }
    item = item + 10;
    results.append(current());
    if (true) {
        var item = "inner";
        print(item);
    }
}
for (var values: int in values) { print(values); }
print(results);
print(values);
print(item);
]=] "inner\ninner\n1\n2\n[11, 12]\n[1, 2]\n99\n" "")

add_jazz_test(for_iterable_kinds [=[
var elements = <>;
for (var item in <3, 1, 3, 2>) { elements.add(item); }
print(elements == <1, 2, 3>);
var keys = [];
for (var key: string in {"Jazz": 1, "Lu": 2}) { keys.append(key); }
print(keys);
var letters = [];
for (var letter: string in "Příliš") { letters.append(letter); }
print(letters);
const frozen = [1, 2];
var copied = [];
for (var item in frozen) { copied.append(item); }
print(copied);
]=] [=[true
["Jazz", "Lu"]
["P", "ř", "í", "l", "i", "š"]
[1, 2]
]=] "")

#the source is evaluated once and its shallow snapshot survives source growth
add_jazz_test(for_source_snapshot_and_shared_children [=[
var calls = [];
var values = [[1], [2]];
function source(): list {
    calls.append(1);
    return values;
}
var seen = [];
for (var item: list in source()) {
    seen.append(item[0]);
    item.append(9);
    values.append([99]);
}
print(len(calls));
print(seen);
print(values);
]=] "1\n[1, 2]\n[[1, 9], [2, 9], [99], [99]]\n" "")

add_jazz_test(for_set_dict_snapshots_survive_clear [=[
var elements = <1, 2, 3>;
var seen = <>;
for (var item in elements) {
    elements.clear();
    seen.add(item);
}
print(seen == <1, 2, 3>);
print(len(elements));
var mapping = {"a": 1, "b": 2};
var keys = [];
for (var key in mapping) {
    mapping.clear();
    keys.append(key);
}
print(keys);
print(mapping);
]=] [=[true
0
["a", "b"]
{}
]=] "")

#empty iterables perform no element type checks or body execution
add_jazz_test(for_empty_typed_iterables [=[
for (var item: string in []) { print("unreachable"); }
for (var item: float in range(0)) { print("unreachable"); }
for (var item: int in "") { print("unreachable"); }
for (var item: bool in {}) { print("unreachable"); }
for (var item: none in <>) { print("unreachable"); }
print("done");
]=] "done\n" "")

add_jazz_test(loop_break_continue [=[
var fromFor = [];
for (var item in range(7)) {
    if (item == 2) { continue; }
    if (item == 5) { break; }
    fromFor.append(item);
}
print(fromFor);
var count = 0;
var fromWhile = [];
while (true) {
    count = count + 1;
    if (count == 2) { continue; }
    if (count == 5) { break; }
    fromWhile.append(count);
}
print(fromWhile);
print(count);
]=] "[0, 1, 3, 4]\n[1, 3, 4]\n5\n" "")

#break and continue affect only the nearest loop
add_jazz_test(loop_nested_control [=[
var values = [];
for (var outer in range(3)) {
    var inner = 0;
    while (true) {
        inner = inner + 1;
        if (inner == 1) { continue; }
        values.append(outer * 10 + inner);
        break;
    }
    values.append(outer);
}
print(values);
var pairs = [];
for (var outer in range(2)) {
    for (var inner in range(3)) {
        if (inner == 0) { continue; }
        pairs.append([outer, inner]);
        break;
    }
}
print(pairs);
]=] "[2, 0, 12, 1, 22, 2]\n[[0, 1], [1, 1]]\n" "")

#returns cross loops and called functions retain their own loop control
add_jazz_test(loop_function_control_and_return [=[
function find(): int {
    for (var item in range(5)) {
        while (true) {
            if (item == 2) { return item; }
            break;
        }
    }
    return 99;
}
function finish(): void {
    while (true) {
        for (var item in [1]) { fetch; }
    }
}
print(find());
finish();
for (var item in [3, 4]) {
    function local(): int {
        for (var inner in [0, 1]) {
            if (inner == 0) { continue; }
            break;
        }
        return item;
    }
    print(local());
    if (item == 3) { continue; }
    break;
}
print("done");
]=] "2\n3\n4\ndone\n" "")

#range returns an ordinary mutable list and excludes its stop value
add_jazz_test(range_forms_and_empty_results [=[
print(range(4));
print(range(2, 5));
print(range(1, 7, 2));
print(range(5, 0, -2));
print(range(-3, 3, 2));
print(range(0));
print(range(-2));
print(range(3, 3));
print(range(3, 1));
print(range(1, 3, -1));
var values: list = range(2);
values.append(9);
print(values);
]=] [=[[0, 1, 2, 3]
[2, 3, 4]
[1, 3, 5]
[5, 3, 1]
[-3, -1, 1]
[]
[]
[]
[]
[]
[0, 1, 9]
]=] "")

#boundary steps stay within int64 without overflowing the next candidate
add_jazz_test(range_int64_boundaries [=[
print(range(9223372036854775805, 9223372036854775807));
print(range(9223372036854775806, 9223372036854775807, 2));
print(range(-9223372036854775808, -9223372036854775806));
print(range(-9223372036854775806, -9223372036854775808, -1));
print(range(0, -9223372036854775808, -9223372036854775808));
print(range(9223372036854775807, -9223372036854775808, -9223372036854775808));
print(range(-9223372036854775808, 9223372036854775807, 9223372036854775807));
]=] [=[[9223372036854775805, 9223372036854775806]
[9223372036854775806]
[-9223372036854775808, -9223372036854775807]
[-9223372036854775806, -9223372036854775807]
[0]
[9223372036854775807, -1]
[-9223372036854775808, -1, 9223372036854775806]
]=] "")

add_jazz_test(range_user_function_shadowing [=[
function range(value: int): list { return [99, value]; }
var values = [];
for (var item in range(3)) { values.append(item); }
print(values);
print(range(4));
]=] "[99, 3]\n[99, 4]\n" "")

add_jazz_test(for_noniterable
    "for (var item in 1) {}"
    "" "Value is not iterable: int"
)
add_jazz_test(for_exact_annotation
    "for (var item: float in [1.0, 2]) { print(item); }"
    "1\n" "Type mismatch: item"
)
add_jazz_test(for_iteration_variable_fixed_type
    "for (var item in [1]) { item = \"changed\"; }"
    "" "Type mismatch: item"
)
add_jazz_test(for_variable_does_not_escape
    "for (var item in [1]) {} print(item);"
    "" "Undefined variable: item"
)
add_jazz_test(for_body_cannot_redeclare_iteration_variable
    "for (var item in [1]) { var item = 2; }"
    "" "Variable already defined: item"
)

foreach(keyword break continue)
    if(keyword STREQUAL "break")
        set(controlName "Break")
    else()
        set(controlName "Continue")
    endif()
    add_jazz_test(${keyword}_outside_loop
        "if (false) { ${keyword}; }"
        "" "${controlName} is only allowed inside a loop"
    )
    add_jazz_test(${keyword}_cannot_escape_nested_function
        "while (false) { function value() { ${keyword}; } }"
        "" "${controlName} is only allowed inside a loop"
    )
endforeach()

add_jazz_test(for_requires_var
    "for (item in [1]) {}"
    "" "Expected 'var' in for loop"
)
add_jazz_test(for_rejects_const_declaration
    "for (const item in [1]) {}"
    "" "Expected 'var' in for loop"
)
add_jazz_test(for_requires_in
    "for (var item [1]) {}"
    "" "Expected 'in' after loop variable"
)

add_jazz_test(range_requires_argument
    "range();" "" "Invalid argument count: range"
)
add_jazz_test(range_rejects_four_arguments
    "range(1, 2, 3, 4);" "" "Invalid argument count: range"
)
add_jazz_test(range_rejects_bool
    "range(true);" "" "range arguments must be int"
)
add_jazz_test(range_rejects_float
    "range(0, 3, 1.0);" "" "range arguments must be int"
)
add_jazz_test(range_rejects_zero_step
    "range(0, 3, 0);" "" "range step cannot be zero"
)
add_jazz_test(range_rejects_impossible_size
    "range(-9223372036854775808, 9223372036854775807);"
    "" "range is too large"
)

#debug output includes the loop variable, optional type and control statements
set(loopAstOutput "\n")
string(APPEND loopAstOutput [=[Program
├── ForStatement
│   ├── name: item
│   ├── type: int
│   ├── iterable:
│   │   └── CallExpression(range)
│   │       └── arguments:
│   │           └── NumberExpression(0)
│   └── body:
│       ├── ContinueStatement
│       └── BreakStatement
└── WhileStatement
    ├── condition:
    │   └── BoolExpression(false)
    └── body:
        ├── BreakStatement
        └── ContinueStatement
]=])
add_jazz_test(loops_ast [=[
for (var item: int in range(0)) {
    continue;
    break;
}
while (false) {
    break;
    continue;
}
]=] "${loopAstOutput}" "" --ast)

#keeps the user-facing loop example executable
file(READ "${PROJECT_SOURCE_DIR}/examples/loops.jazz" loopsExampleSource)
add_jazz_test(loops_example "${loopsExampleSource}" [=[
[1, 2, 3, 4, 5]
[1, 4, 16, 25]
0
1
2
[5, 3, 1]
Jazz
Lu
]=] "")
