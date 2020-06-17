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

RawUncompressedFile::RawUncompressedFile(StructuredCompressedFile & structuredCompressedFile) :
    m_structuredCompressedFile(structuredCompressedFile)
{
}

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
        indexThread(); // @todo make this a thread
        // @todo start normal read thread
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

    // @todo abort read thread
    // @todo wait till read/write threads finished

    /* write unknown115 */
    if (m_openMode & std::ios_base::out) {
        // @todo write unknown115
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
                       (logContainerRef.filePosition + logContainerRef.fileSize > m_posg);
            });
        /* not found */
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
                logContainer->uncompress(logContainerRef->uncompressedFile);
                // @todo this should be done by the read ahead thread
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
                       (logContainerRef.filePosition + logContainerRef.fileSize > m_posp);
            });
        /* not found, so append new log container */
        if (logContainerRef == m_logContainerRefs.end()) {
            /* append new log container */
            LogContainerRef newLogContainerRef;
            if (m_logContainerRefs.empty()) {
                newLogContainerRef.filePosition = 0;
            } else {
                newLogContainerRef.filePosition = m_logContainerRefs.back().fileSize + m_logContainerRefs.back().filePosition;
            }
            newLogContainerRef.fileSize = m_defaultLogContainerSize;
            newLogContainerRef.uncompressedFile.resize(m_defaultLogContainerSize);
            m_logContainerRefs.push_back(newLogContainerRef);
            logContainerRef = m_logContainerRefs.end() - 1;
        }

        /* copy data */
        streamoff offset = m_posp - logContainerRef->filePosition;
        streamsize size = std::min(n - totalSize, logContainerRef->fileSize - offset);
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

FileStatistics RawUncompressedFile::statistics() const {
    // no lock needed as just pass-thru
    return m_structuredCompressedFile.statistics();
}

void RawUncompressedFile::setStatistics(const Vector::BLF::FileStatistics & statistics) {
    // no lock needed as just pass-thru
    m_structuredCompressedFile.setStatistics(statistics);
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

Unknown115 RawUncompressedFile::unknown115() const {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_unknown115;
}

void RawUncompressedFile::setUnknown115(const Unknown115 & unknown115) {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    m_unknown115 = unknown115;
}

void RawUncompressedFile::indexThread() {
    // already locked by calling method open

    /* create index of all log containers */
    streampos filePosition = 0;
    m_statisticsSize = m_structuredCompressedFile.statistics().statisticsSize;
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

    // @todo read thread
}

void RawUncompressedFile::writeThread() {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    // @todo write thread
}

}
}