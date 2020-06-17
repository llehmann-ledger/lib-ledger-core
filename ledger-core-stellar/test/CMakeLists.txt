project(ledger-core-stellar-tests)

add_executable(ledger-core-stellar-tests
    main.cpp
    address_tests.cpp
    create_stellar_account_tests.cpp
    get_memo_tests.cpp
    horizon_explorer_tests
    StellarFixture.cpp
    synchronization_tests.cpp
    transaction_tests.cpp
    xdr_tests.cpp
)

target_link_libraries(ledger-core-stellar-tests ledger-core-bundle)
target_link_libraries(ledger-core-stellar-tests ledger-test)
target_link_libraries(ledger-core-stellar-tests ledger-core-integration-test)
target_link_libraries(ledger-core-stellar-tests ledger-qt-host)
target_link_libraries(ledger-core-stellar-tests ledger-core-stellar-obj)
target_link_libraries(ledger-core-stellar-tests gtest gtest_main)

