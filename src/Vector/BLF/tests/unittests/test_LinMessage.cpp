#define BOOST_TEST_MODULE LinMessage
#if !defined(WIN32)
#define BOOST_TEST_DYN_LINK
#endif
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include <Vector/BLF.h>

/* LIN_MESSAGE = 11 */
BOOST_AUTO_TEST_CASE(LinMessage)
{
    Vector::BLF::File file;
    file.open(CMAKE_CURRENT_SOURCE_DIR "/events_from_binlog/test_LinMessage.blf");
    BOOST_REQUIRE(file.is_open());

    Vector::BLF::ObjectHeaderBase * ohb = file.read();
    BOOST_REQUIRE(ohb != nullptr);
    BOOST_REQUIRE(ohb->objectType == Vector::BLF::ObjectType::LIN_MESSAGE);
    Vector::BLF::LinMessage * obj = static_cast<Vector::BLF::LinMessage *>(ohb);

    /* ObjectHeaderBase */
    BOOST_CHECK_EQUAL(obj->signature, Vector::BLF::ObjectSignature);
    BOOST_CHECK_EQUAL(obj->headerSize, obj->calculateHeaderSize());
    BOOST_CHECK_EQUAL(obj->headerVersion, 1);
    BOOST_CHECK_EQUAL(obj->objectSize, obj->calculateObjectSize());
    BOOST_CHECK(obj->objectType == Vector::BLF::ObjectType::LIN_MESSAGE);

    /* ObjectHeader */
    BOOST_CHECK_EQUAL(obj->objectFlags, Vector::BLF::ObjectHeader::ObjectFlags::TimeOneNans);
    BOOST_CHECK_EQUAL(obj->clientIndex, 0x1111);
    BOOST_CHECK_EQUAL(obj->objectVersion, 0);
    BOOST_CHECK_EQUAL(obj->objectTimeStamp, 0x2222222222222222);

    /* LinMessage */
    BOOST_CHECK_EQUAL(obj->channel, 0x1111);
    BOOST_CHECK_EQUAL(obj->id, 0x22);
    BOOST_CHECK_EQUAL(obj->dlc, 0x33);
    BOOST_CHECK_EQUAL(obj->data[0], 0x44);
    BOOST_CHECK_EQUAL(obj->data[1], 0x55);
    BOOST_CHECK_EQUAL(obj->data[2], 0x66);
    BOOST_CHECK_EQUAL(obj->data[3], 0x77);
    BOOST_CHECK_EQUAL(obj->data[4], 0x88);
    BOOST_CHECK_EQUAL(obj->data[5], 0x99);
    BOOST_CHECK_EQUAL(obj->data[6], 0xAA);
    BOOST_CHECK_EQUAL(obj->data[7], 0xBB);
    BOOST_CHECK_EQUAL(obj->fsmId, 0xCC);
    BOOST_CHECK_EQUAL(obj->fsmState, 0xDD);
    BOOST_CHECK_EQUAL(obj->headerTime, 0xEE);
    BOOST_CHECK_EQUAL(obj->fullTime, 0xFF);
    BOOST_CHECK_EQUAL(obj->crc, 0x1111);
    BOOST_CHECK_EQUAL(obj->dir, 0x22);
    BOOST_CHECK_EQUAL(obj->reservedLinMessage1, 0x33);
    BOOST_CHECK_EQUAL(obj->reservedLinMessage2, 0);

    delete ohb;

    /* read next */
    ohb = file.read();
    BOOST_REQUIRE(ohb != nullptr);
    BOOST_REQUIRE(ohb->objectType == Vector::BLF::ObjectType::LIN_MESSAGE);

    delete ohb;

    file.close();
}