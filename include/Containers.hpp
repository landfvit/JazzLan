#pragma once

#include <algorithm>
#include <functional>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

#include "Value.hpp"

//tracks containers so recursive references can be released at shutdown
class ContainerHeap {
private:
    using WeakContainer = std::variant<
        std::weak_ptr<ListValue>,
        std::weak_ptr<SetValue>,
        std::weak_ptr<DictValue>
    >;

    std::vector<WeakContainer> objects;
    size_t allocations = 0;

public:
    ContainerHeap() = default;
    ContainerHeap(const ContainerHeap&) = delete;
    ContainerHeap& operator=(const ContainerHeap&) = delete;
    ~ContainerHeap();

    //creates a mutable container without copying its children
    template<typename T>
    std::shared_ptr<T> create() {
        if (++allocations % 256 == 0) {
            std::erase_if(objects, [](const WeakContainer& object) {
                return std::visit([](const auto& pointer) {
                    return pointer.expired();
                }, object);
            });
        }

        auto object = std::make_shared<T>();
        objects.emplace_back(object);
        return object;
    }
};

//constructs a container or range, or evaluates len
Value callContainerFunction(
    const std::string& name,
    const std::vector<Value>& arguments,
    ContainerHeap& heap
);

//executes a method on a container
Value callContainerMethod(
    const Value& object,
    const std::string& name,
    const std::vector<Value>& arguments,
    ContainerHeap& heap
);

//returns elements in iteration order
std::vector<Value> iterableValues(const Value& value);
//returns the number of elements without copying a container
size_t containerSize(const Value& value);
//checks membership using the same equality as dictionary keys
bool containsValue(const Value& container, const Value& value);
//reads or replaces an item
Value getItem(const Value& object, const Value& index);
void setItem(const Value& object, const Value& index, const Value& value);
//creates a shallow slice
Value sliceValue(
    const Value& object,
    const std::optional<Value>& start,
    const std::optional<Value>& stop,
    const std::optional<Value>& step,
    ContainerHeap& heap
);
//handles container operators, or returns no result for scalar operators
std::optional<Value> containerOperation(
    const Value& left,
    const std::string& op,
    const Value& right,
    ContainerHeap& heap
);

//transforms a snapshot into a new container of the same type
Value transformContainer(
    const Value& source,
    const std::function<Value(const Value&)>& transform,
    ContainerHeap& heap
);
//replaces shared contents after all transformed values are ready
void replaceContainerContents(const Value& source, const Value& result);
