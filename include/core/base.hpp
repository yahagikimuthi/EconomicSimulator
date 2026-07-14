#pragma once

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