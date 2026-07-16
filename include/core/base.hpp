#pragma once

#include <cassert>
#include <concepts>

template <typename ComponentType>
struct BaseView {
    explicit BaseView(ComponentType& comp) : comp_{comp} {}

    template <typename DerivedView>
        requires std::derived_from<std::remove_cvref_t<DerivedView>, BaseView<ComponentType>>
    explicit BaseView(DerivedView& derived) : comp_{derived.comp_} {}

    virtual ~BaseView() = default;

    BaseView(const BaseView&)                    = default;
    auto operator=(const BaseView&) -> BaseView& = default;
    BaseView(BaseView&&)                         = default;
    auto operator=(BaseView&&) -> BaseView&      = default;

  protected:
    ComponentType& comp_;
};

template <typename T>
class [[nodiscard]] SafePtr {
  public:
    SafePtr(T* ptr) : ptr_{ptr} {}

    auto hasValue() const -> bool { return ptr_ != nullptr; }

    auto operator*() const -> const T& {
        assert(ptr_ != nullptr && "Attempted to access a null pointer via SafePtr");
        return *ptr_;
    }
    auto operator*() -> T& {
        assert(ptr_ != nullptr && "Attempted to access a null pointer via SafePtr");
        return *ptr_;
    }
    auto operator->() const -> const T* {
        assert(ptr_ != nullptr && "Attempted to access a null pointer via SafePtr");
        return ptr_;
    }
    auto operator->() -> T* {
        assert(ptr_ != nullptr && "Attempted to access a null pointer via SafePtr");
        return ptr_;
    }

  private:
    T* ptr_ = nullptr;
};