#define BOOST_TEST_MODULE FileStatistics
#if !defined(WIN32)
#define BOOST_TEST_DYN_LINK
#endif
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include <Vector/BLF.h>

/** test throw on wrong file signature */
BOOST_AUTO_TEST_CASE(CorrectFileSignature) {
    /* open file and write in fileStatistics */
    {
        Vector::BLF::RawCompressedFile file;
        file.open(CMAKE_CURRENT_BINARY_DIR "/test_FileStatistics.blf", std::ios_base::out);
        Vector::BLF::FileStatistics fileStatistics1;
        fileStatistics1.write(file);
    }

    /* read fileStatistics */
    {
        Vector::BLF::RawCompressedFile file;
        file.open(CMAKE_CURRENT_BINARY_DIR "/test_FileStatistics.blf", std::ios_base::in);
        Vector::BLF::FileStatistics fileStatistics2;
        fileStatistics2.read(file);
    }
}

/** test throw on wrong file signature */
BOOST_AUTO_TEST_CASE(WrongFileSignature) {
    /* open file and write in fileStatistics */
    {
        Vector::BLF::RawCompressedFile file;
        file.open(CMAKE_CURRENT_BINARY_DIR "/test_FileStatistics.blf", std::ios_base::out);
        Vector::BLF::FileStatistics fileStatistics1;
        fileStatistics1.signature--;
        fileStatistics1.write(file);
    }

    /* read fileStatistics */
    {
        Vector::BLF::RawCompressedFile file;
        file.open(CMAKE_CURRENT_BINARY_DIR "/test_FileStatistics.blf", std::ios_base::in);
        Vector::BLF::FileStatistics fileStatistics2;
        BOOST_CHECK_THROW(fileStatistics2.read(file), Vector::BLF::Exception);
    }
}
