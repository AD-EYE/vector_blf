/*
 * Copyright (C) 2013-2020 Tobias Lorenz.
 * Contact: tobias.lorenz@gmx.net
 *
 * This file is part of Tobias Lorenz's Toolkit.
 *
 * Commercial License Usage
 * Licensees holding valid commercial licenses may use this file in
 * accordance with the commercial license agreement provided with the
 * Software or, alternatively, in accordance with the terms contained in
 * a written agreement between you and Tobias Lorenz.
 *
 * GNU General Public License 3.0 Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl.html.
 */

#include <Vector/BLF/RawUncompressedFile.h>

#include <algorithm>
#undef NDEBUG
#include <cassert>
#include <iostream>

namespace Vector {
namespace BLF {

RawUncompressedFile::~RawUncompressedFile() {
    close();
}

void RawUncompressedFile::open(const char * filename, std::ios_base::openmode mode) {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    /* open file */
    m_structuredCompressedFile.open(filename, mode);
    if (!m_structuredCompressedFile.is_open()) {
        return;
    }
    m_openMode = mode;

    /* start index/read thread */
    if (m_openMode & std::ios_base::in) {
        indexThread(); // @todo start index thread
        // @todo start read thread
    }

    /* start write thread */
    if (m_openMode & std::ios_base::out) {
        // @todo start write thread
    }
}

bool RawUncompressedFile::is_open() const {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_structuredCompressedFile.is_open();
}

void RawUncompressedFile::close() {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_structuredCompressedFile.is_open()) {
        return;
    }

    // @todo abort threads and wait to join

    /* close actions */
    if (m_openMode & std::ios_base::out) {
        /* finish log container */
        shrinkLogContainer();
        writeLogContainer();

        /* set FileStatistics::uncompressedFileSize */
        m_structuredCompressedFile.setUncompressedFileSize(m_statisticsSize);
    }

    m_structuredCompressedFile.close();
}

RawUncompressedFile::streamsize RawUncompressedFile::read(char * s, const RawUncompressedFile::streamsize n) {
    assert(s || !n); // if n>0, then s!=nullptr

    /* mutex lock */
    std::unique_lock<std::mutex> lock(m_mutex);

    /* read data */
    streamsize totalSize = 0;
    while (totalSize < n) {
        /* find starting log container */
        std::vector<LogContainerRef>::iterator logContainerRef =
            std::find_if(m_logContainerRefs.begin(), m_logContainerRefs.end(), [&](const LogContainerRef & logContainerRef) {
                return (logContainerRef.filePosition <= m_posg) &&
                       (m_posg < logContainerRef.filePosition + logContainerRef.fileSize);
            });

        /* not found, so abort read here */
        if (logContainerRef == m_logContainerRefs.end()) {
            return totalSize;
        }

        /* load data if needed */
        if (logContainerRef->uncompressedFile.empty()) {
            /* read log container */
            StructuredCompressedFile::streampos containerIndex = logContainerRef - m_logContainerRefs.begin();
            m_structuredCompressedFile.seekg(containerIndex);
            LogContainer * logContainer = nullptr;
            if (m_structuredCompressedFile.read(&logContainer)) {
                logContainer->uncompress(logContainerRef->uncompressedFile); // @todo do this in readThread
                delete logContainer;
            } else {
                assert(!logContainer); // no delete necessary
                return totalSize;
            }
        }

        /* copy data */
        streamoff offset = m_posg - logContainerRef->filePosition;
        streamsize size = std::min(n - totalSize, logContainerRef->fileSize - offset);
        assert(size); // logContainerRef would be wrong otherwise
        assert(logContainerRef->uncompressedFile.cbegin() + offset + size <= logContainerRef->uncompressedFile.cend()); // check source size
        std::copy(logContainerRef->uncompressedFile.cbegin() + offset,
                  logContainerRef->uncompressedFile.cbegin() + offset + size,
                  s);

        /* update counters */
        m_posg += size;
        s += size;
        totalSize += size;
    }

    return totalSize;
}

RawUncompressedFile::streampos RawUncompressedFile::tellg() {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_structuredCompressedFile.is_open()) {
        return -1;
    }
    return m_posg;
}

void RawUncompressedFile::seekg(const RawUncompressedFile::streampos pos) {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    /* new get position */
    m_posg = pos;
}

void RawUncompressedFile::seekg(const RawUncompressedFile::streamoff off, const std::ios_base::seekdir way) {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    /* calculate new position */
    streampos ref;
    switch(way) {
    case std::ios_base::beg:
        ref = 0;
        break;
    case std::ios_base::cur:
        ref = m_posg;
        break;
    case std::ios_base::end:
        ref = m_posp; // posp is always at the end
        break;
    default:
        assert(false); // other cases should not happen
    }
    m_posg = ref + off;
}

RawUncompressedFile::streamsize RawUncompressedFile::write(const char * s, const RawUncompressedFile::streamsize n) {
    assert(s || !n); // if n>0, then s!=nullptr

    /* mutex lock */
    std::unique_lock<std::mutex> lock(m_mutex);

    /* write data */
    streamsize totalSize = 0;
    while (totalSize < n) {
        /* find starting log container */
        std::vector<LogContainerRef>::iterator logContainerRef =
            std::find_if(m_logContainerRefs.begin(), m_logContainerRefs.end(), [&](const LogContainerRef & logContainerRef) {
                return (logContainerRef.filePosition <= m_posp) &&
                       (m_posp < logContainerRef.filePosition + logContainerRef.fileSize);
            });

        /* not found, so append new log container */
        if (logContainerRef == m_logContainerRefs.end()) {
            /* prepare log container reference */
            LogContainerRef newLogContainerRef;

            /* write the current log container */
            if (!m_logContainerRefs.empty()) {
                newLogContainerRef.filePosition = m_logContainerRefs.back().filePosition + m_logContainerRefs.back().fileSize;

                /* compress and write log container */
                writeLogContainer();
            }

            /* append log container reference */
            newLogContainerRef.fileSize = m_defaultLogContainerSize;
            newLogContainerRef.uncompressedFile.resize(m_defaultLogContainerSize);
            assert(newLogContainerRef.uncompressedFile.size() == m_defaultLogContainerSize);
            m_logContainerRefs.push_back(newLogContainerRef);
            logContainerRef = std::prev(m_logContainerRefs.end());
        }
        assert(logContainerRef != m_logContainerRefs.end());
        if (logContainerRef->uncompressedFile.empty()) {
            std::cout << "LogContainerRef is empty: position=0x" << std::hex << logContainerRef->filePosition << " size=0x" << std::hex << logContainerRef->fileSize << std::endl;
            assert(!logContainerRef->uncompressedFile.empty());
        }
        assert(logContainerRef->fileSize == std::streamsize(logContainerRef->uncompressedFile.size()));

        /* copy data */
        streamoff offset = m_posp - logContainerRef->filePosition;
        streamsize size = std::min(n - totalSize, logContainerRef->fileSize - offset);
        assert(size); // logContainerRef would be wrong otherwise
        assert(logContainerRef->uncompressedFile.begin() + offset + size <= logContainerRef->uncompressedFile.end()); // check destination size
        std::copy(s,
                  s + size,
                  logContainerRef->uncompressedFile.begin() + offset);

        /* update counters */
        m_posp += size;
        s += size;
        totalSize += size;
    }

    return totalSize;
}

RawUncompressedFile::streampos RawUncompressedFile::tellp() {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_structuredCompressedFile.is_open()) {
        return -1;
    }
    return m_posp;
}

void RawUncompressedFile::seekp(const RawUncompressedFile::streampos /*pos*/) {
    /* should not be used */
    assert(false);
}

void RawUncompressedFile::seekp(const RawUncompressedFile::streamoff off, const std::ios_base::seekdir way) {
    assert((off >= 0) && (way == std::ios_base::cur)); // only to be used to skip padding bytes

    std::vector<char> zero(off);
    write(zero.data(), zero.size()); // write does the lock
}

RawUncompressedFile::streamsize RawUncompressedFile::size() const {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_size;
}

RawUncompressedFile::streamsize RawUncompressedFile::statisticsSize() const {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_statisticsSize;
}

DWORD RawUncompressedFile::defaultLogContainerSize() const {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_defaultLogContainerSize;
}

void RawUncompressedFile::setDefaultLogContainerSize(DWORD defaultLogContainerSize) {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    /* set default log container size */
    m_defaultLogContainerSize = defaultLogContainerSize;
}

int RawUncompressedFile::compressionMethod() const {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_compressionMethod;
}

void RawUncompressedFile::setCompressionMethod(const int compressionMethod) {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    m_compressionMethod = compressionMethod;
}

int RawUncompressedFile::compressionLevel() const {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_compressionLevel;
}

void RawUncompressedFile::setCompressionLevel(const int compressionLevel) {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    m_compressionLevel = compressionLevel;
}

void RawUncompressedFile::finishLogContainer() {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    shrinkLogContainer();
}

/* StructuredCompressedFile pass-thru methods */

StructuredCompressedFile::streamsize RawUncompressedFile::structuredCompressedFileSize() const {
    return m_structuredCompressedFile.size();
}

/* RawCompressedFile pass-thru methods */

RawCompressedFile::streamsize RawUncompressedFile::rawCompressedFileSize() const {
    return m_structuredCompressedFile.rawCompressedFileSize();
}

FileStatistics RawUncompressedFile::statistics() const {
    return m_structuredCompressedFile.statistics();
}

void RawUncompressedFile::setApplication(const BYTE id, const BYTE major, const BYTE minor, const BYTE build) {
    m_structuredCompressedFile.setApplication(id, major, minor, build);
}

void RawUncompressedFile::setApi(const BYTE major, const BYTE minor, const BYTE build, const BYTE patch) {
    m_structuredCompressedFile.setApi(major, minor, build, patch);
}

void RawUncompressedFile::setObjectCount(const DWORD objectCount) {
    m_structuredCompressedFile.setObjectCount(objectCount);
}

void RawUncompressedFile::setObjectsRead(const DWORD objectsRead) {
    m_structuredCompressedFile.setObjectsRead(objectsRead);
}

void RawUncompressedFile::setMeasurementStartTime(const SYSTEMTIME measurementStartTime) {
    m_structuredCompressedFile.setMeasurementStartTime(measurementStartTime);
}

void RawUncompressedFile::setLastObjectTime(const SYSTEMTIME lastObjectTime) {
    m_structuredCompressedFile.setLastObjectTime(lastObjectTime);
}

/* private methods */

void RawUncompressedFile::shrinkLogContainer() {
    if (m_logContainerRefs.empty()) {
        return;
    }

    /* shrink_to_fit last log container */
    streamsize size = m_posp - m_logContainerRefs.back().filePosition;
    assert(size >= 0);
    if (size == 0) {
        m_logContainerRefs.pop_back();
    } else {
        m_logContainerRefs.back().uncompressedFile.resize(size);
        m_logContainerRefs.back().fileSize = size;
    }
}

void RawUncompressedFile::writeLogContainer() {
    if (m_logContainerRefs.empty()) {
        return;
    }
    if (m_logContainerRefs.back().uncompressedFile.empty()) {
        return;
    }

    /* compress and write log container */
    LogContainer * logContainer = new LogContainer;
    logContainer->compress(m_logContainerRefs.back().uncompressedFile, m_compressionMethod, m_compressionLevel);
    m_structuredCompressedFile.write(logContainer);

    /* update statistics */
    m_statisticsSize += logContainer->internalHeaderSize();
    m_statisticsSize += logContainer->uncompressedFileSize;
}

/* threads */

void RawUncompressedFile::indexThread() {
    // already locked by calling method open

    /* create index of all log containers */
    streampos filePosition = 0;
    m_statisticsSize = 0;
    for(;;) {
        LogContainer * logContainer = nullptr;
        if (!m_structuredCompressedFile.read(&logContainer)) {
            break;
        }

        /* add log container reference */
        LogContainerRef logContainerRef;
        logContainerRef.filePosition = filePosition;
        logContainerRef.fileSize = logContainer->uncompressedFileSize;
        m_logContainerRefs.push_back(logContainerRef);

        /* update statistics */
        filePosition += logContainer->uncompressedFileSize;
        m_statisticsSize += logContainer->internalHeaderSize();
        m_statisticsSize += logContainer->uncompressedFileSize;

        delete logContainer;
    }

    /* set file size */
    m_size = filePosition;

    /* seek back to first log container */
    m_structuredCompressedFile.seekg(0);
}

void RawUncompressedFile::readThread() {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    // @todo RawUncompressedFile::readThread
}

void RawUncompressedFile::writeThread() {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    // @todo RawUncompressedFile::writeThread
}

}
}
