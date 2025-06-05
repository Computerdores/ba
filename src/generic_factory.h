#pragma once
#include <tuple>

template <typename T, typename... Args>
class GenericFactory {
  public:
    using Object = T;

    explicit GenericFactory(Args &&...args) : _args(std::forward<Args>(args)...) {}

    Object *build() {
        return std::apply([](auto &&...args) { return new Object(std::forward<Args>(args)...); }, _args);
    }

  private:
    std::tuple<Args...> _args;
};
