#include <dbwrapper.h>
#include <staking/staker_db.h>
#include <test/util/setup_common.h>
#include <boost/test/unit_test.hpp>


void script_fixture(CScript& script) {
    CKey key;
    CPubKey pubkey;
    key.MakeNewKey(true);
    pubkey = key.GetPubKey();
    script.clear();
    script << ToByteVector(pubkey) << OP_CHECKSIG;
}

BOOST_FIXTURE_TEST_SUITE(stakerdb_tests, BasicTestingSetup)

void check_entry_equals(CStakerDbEntry input, CStakerDbEntry output) {
    BOOST_CHECK(input.getAmount() == output.getAmount());
    BOOST_CHECK(input.getReward() == output.getReward());
    BOOST_CHECK(input.isComplete() == output.isComplete());
    BOOST_CHECK(input.getCompleteBlock() == output.getCompleteBlock());
    BOOST_CHECK(input.getNumOutput() == output.getNumOutput());
    BOOST_CHECK(input.getScript() == output.getScript());
    BOOST_CHECK(input.getKey() == output.getKey());
}

BOOST_AUTO_TEST_CASE(staker_db_entry_serialization) {
    CScript script;
    script_fixture(script);
    CStakerDbEntry input(InsecureRand256(), 10, 15, 20, 25, 30, script);
    CStakerDbEntry output;

    fs::path ph = GetDataDir() / "test_staker_db";
    CDBWrapper dbw(ph, (1 << 20), true, false, false);

    BOOST_CHECK(dbw.Write(input.getKey(), input));
    BOOST_CHECK(dbw.Read(input.getKey(), output));
    output.setKey(input.getKey());
    check_entry_equals(input, output);
}

BOOST_AUTO_TEST_CASE(staker_db_crud_operation) {
    CScript script;
    script_fixture(script);
    uint256 key = InsecureRand256();
    CStakerDbEntry input(key, 10, 15, 20, 25, 30, script);
    CStakerDbEntry output;
    CStakerDB db;

    // test agains cache crud
    BOOST_CHECK(db.addStakerEntry(input));
    output = db.getStakerDbEntry(key);
    check_entry_equals(input, output);
    BOOST_CHECK(db.removeStakerEntry(key));
    output = db.getStakerDbEntry(key);
    BOOST_CHECK(input.getKey() != output.getKey());

    // test against levedb crud
    BOOST_CHECK(db.addStakerEntry(input));
    db.flushDB();
    output = db.getStakerDbEntry(key);
    check_entry_equals(input, output);
    BOOST_CHECK(db.removeStakerEntry(key));
    output = db.getStakerDbEntry(key);
    BOOST_CHECK(input.getKey() != output.getKey());

}

BOOST_AUTO_TEST_CASE(address_mapping) {
    CStakerDB db;
    std::set<uint256> list_of_ids;
    std::string address = "wallet_address";
    uint256 txid1, txid2 = InsecureRand256(), InsecureRand256();
    db.addAddressToMap(address, txid1);
    db.addAddressToMap(address, txid1);
    list_of_ids = db.getStakerIdsForAddress(address);
    BOOST_CHECK(list_of_ids.size() == 1);

    db.addAddressToMap(address, txid2);
    list_of_ids = db.getStakerIdsForAddress(address);
    BOOST_CHECK(list_of_ids.size() == 2);

    list_of_ids = db.getStakerIdsForAddress("not existing address");
    BOOST_CHECK(list_of_ids.size() == 0);
}

BOOST_AUTO_TEST_SUITE_END()
