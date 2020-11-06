#include <gtest/gtest.h>
#include "BaseFixture.h"

class DispatcherStopTest : public BaseFixture
{
};

TEST_F(DispatcherStopTest, InifiniteRunningTest)
{
    auto receiver = make_receiver([&](const std::shared_ptr<api::Event> &event) {
        throw make_exception(api::ErrorCode::UNKNOWN, "Wow, this test would never end without this fix !");
        // Happens really often when there are networks errors for instance.
    });

    getTestExecutionContext()->delay(
    LambdaRunnable::make([&]() {
        receiver->onEvent(nullptr);
    }),1);

    dispatcher->waitUntilStopped();
}