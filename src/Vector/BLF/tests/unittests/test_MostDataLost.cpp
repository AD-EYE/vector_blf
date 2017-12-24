#define BOOST_TEST_MODULE MostDataLost
#if !defined(WIN32)
#define BOOST_TEST_DYN_LINK
#endif
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include <Vector/BLF.h>

/* MOST_DATALOST = 38 */
BOOST_AUTO_TEST_CASE(MostDataLost_1)
{
    Vector::BLF::File file;
    file.open(CMAKE_CURRENT_SOURCE_DIR "/events_from_binlog/test_MostDataLost.blf");
    BOOST_REQUIRE(file.is_open());

    Vector::BLF::ObjectHeaderBase * ohb = file.read();
    BOOST_REQUIRE(ohb != nullptr);
    BOOST_REQUIRE(ohb->objectType == Vector::BLF::ObjectType::MOST_DATALOST);
    Vector::BLF::MostDataLost * obj = static_cast<Vector::BLF::MostDataLost *>(ohb);

    /* ObjectHeaderBase */
    BOOST_CHECK_EQUAL(obj->signature, Vector::BLF::ObjectSignature);
    BOOST_CHECK_EQUAL(obj->headerSize, obj->calculateHeaderSize());
    BOOST_CHECK_EQUAL(obj->headerVersion, 1);
    BOOST_CHECK_EQUAL(obj->objectSize, obj->calculateObjectSize());
    BOOST_CHECK(obj->objectType == Vector::BLF::ObjectType::MOST_DATALOST);

    /* ObjectHeader2 */
    BOOST_CHECK_EQUAL(obj->objectFlags, Vector::BLF::ObjectHeader2::ObjectFlags::TimeOneNans);
    BOOST_CHECK_EQUAL(obj->timeStampStatus, 0x22);
    BOOST_CHECK_EQUAL(obj->reservedObjectHeader, 0x33);
    BOOST_CHECK_EQUAL(obj->objectVersion, 0);
    BOOST_CHECK_EQUAL(obj->objectTimeStamp, 0x4444444444444444);
    BOOST_CHECK_EQUAL(obj->originalTimeStamp, 0x5555555555555555);

    /* MostDataLost */
    BOOST_CHECK_EQUAL(obj->channel, 0x1111);
    BOOST_CHECK_EQUAL(obj->reservedMostDataLost, 0x2222);
    BOOST_CHECK_EQUAL(obj->info, 0x33333333);
    BOOST_CHECK_EQUAL(obj->lostMsgsCtrl, 0x44444444);
    BOOST_CHECK_EQUAL(obj->lostMsgsAsync, 0x55555555);
    BOOST_CHECK_EQUAL(obj->lastGoodTimeStampNs, 0x6666666666666666);
    BOOST_CHECK_EQUAL(obj->nextGoodTimeStampNs, 0x7777777777777777);

    delete ohb;

    /* read next */
    ohb = file.read();
    BOOST_REQUIRE(ohb != nullptr);
    BOOST_REQUIRE(ohb->objectType == Vector::BLF::ObjectType::MOST_DATALOST);

    delete ohb;

    /* read last */
    ohb = file.read();
    BOOST_REQUIRE(ohb != nullptr);
    BOOST_REQUIRE(ohb->objectType == Vector::BLF::ObjectType::Unknown115);

    delete ohb;

    BOOST_CHECK(file.eof());
    file.close();
}

BOOST_AUTO_TEST_CASE(MostDataLost_2)
{
    Vector::BLF::File file;
    file.open(CMAKE_CURRENT_SOURCE_DIR "/events_from_converter/test_MostDataLost.blf");
    BOOST_REQUIRE(file.is_open());

    Vector::BLF::ObjectHeaderBase * ohb = file.read();
    BOOST_REQUIRE(ohb != nullptr);
    BOOST_REQUIRE(ohb->objectType == Vector::BLF::ObjectType::MOST_DATALOST);
    Vector::BLF::MostDataLost * obj = static_cast<Vector::BLF::MostDataLost *>(ohb);

    /* ObjectHeaderBase */
    BOOST_CHECK_EQUAL(obj->signature, Vector::BLF::ObjectSignature);
    BOOST_CHECK_EQUAL(obj->headerSize, obj->calculateHeaderSize());
    BOOST_CHECK_EQUAL(obj->headerVersion, 1); // Vector bug: This should be 2 for ObjectHeader2
    BOOST_CHECK_EQUAL(obj->objectSize, obj->calculateObjectSize());
    BOOST_CHECK(obj->objectType == Vector::BLF::ObjectType::MOST_DATALOST);

    /* ObjectHeader2 */
    BOOST_CHECK_EQUAL(obj->objectFlags, Vector::BLF::ObjectHeader2::ObjectFlags::TimeOneNans);
    BOOST_CHECK_EQUAL(obj->timeStampStatus, 0);
    // reserved
    BOOST_CHECK_EQUAL(obj->objectVersion, 0);
    BOOST_CHECK_EQUAL(obj->objectTimeStamp, 1303700000); // ns
    BOOST_CHECK_EQUAL(obj->originalTimeStamp, 0);

    /* MostDataLost */
    BOOST_CHECK_EQUAL(obj->channel, 1);
    // reserved
    BOOST_CHECK_EQUAL(obj->info, 0x05);
    BOOST_CHECK_EQUAL(obj->lostMsgsCtrl, 0x003F);
    BOOST_CHECK_EQUAL(obj->lostMsgsAsync, 0x000D);
    BOOST_CHECK_EQUAL(obj->lastGoodTimeStampNs, 101303690000); // ns
    BOOST_CHECK_EQUAL(obj->nextGoodTimeStampNs, 2223525920000); // ns

    delete ohb;

    BOOST_CHECK(file.eof());
    file.close();
}
