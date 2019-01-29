// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from callback.djinni

#ifndef DJINNI_GENERATED_CURRENCYLISTCALLBACK_HPP
#define DJINNI_GENERATED_CURRENCYLISTCALLBACK_HPP

#include "../utils/optional.hpp"
#include <vector>

namespace ledger { namespace core { namespace api {

struct Currency;
struct Error;

/**
 *Callback triggered by main completed task,
 *returns optional result as list of template type T
 */
class CurrencyListCallback {
public:
    virtual ~CurrencyListCallback() {}

    /**
     * Method triggered when main task complete
     * @params result optional of type list<T>, non null if main task failed
     * @params error optional of type Error, non null if main task succeeded
     */
    virtual void onCallback(const std::experimental::optional<std::vector<Currency>> & result, const std::experimental::optional<Error> & error) = 0;
};

} } }  // namespace ledger::core::api
#endif //DJINNI_GENERATED_CURRENCYLISTCALLBACK_HPP