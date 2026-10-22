#pragma once

#include <Salix/core/IPlugin.h>
#include <functional>
#include <string>
#include <vector>

class MockIPlugin : public Salix::IPlugin {
public:
    void on_register_types() override {
        ++register_calls;
        call_order.push_back("register");
    }

    void on_startup(Salix::InitContext& context) override {
        ++startup_calls;
        last_context = &context;
        call_order.push_back("startup");
        if (startup_handler) {
            startup_handler(context);
        }
    }

    void on_shutdown() override {
        ++shutdown_calls;
        call_order.push_back("shutdown");
    }

    int register_calls = 0;
    int startup_calls = 0;
    int shutdown_calls = 0;
    Salix::InitContext* last_context = nullptr;
    std::vector<std::string> call_order;
    std::function<void(Salix::InitContext&)> startup_handler;
};
