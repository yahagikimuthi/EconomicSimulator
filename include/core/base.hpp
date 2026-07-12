#pragma once

template <typename ComponentType>
struct BaseView {
    explicit BaseView(ComponentType& comp) : comp_{comp} {}

    virtual ~BaseView() = default;

    BaseView(const BaseView&)                    = default;
    auto operator=(const BaseView&) -> BaseView& = default;
    BaseView(BaseView&&)                         = default;
    auto operator=(BaseView&&) -> BaseView&      = default;

  protected:
    ComponentType& comp_;
};