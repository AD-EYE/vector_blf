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

#pragma once

#include <Vector/BLF/platform.h>

#include <vector>

#include <Vector/BLF/RawFile.h>
#include <Vector/BLF/StructuredCompressedFile.h>
#include <Vector/BLF/Unknown115.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * Raw Uncompressed File
 *
 * This class is thread-safe.
 */
class VECTOR_BLF_EXPORT RawUncompressedFile :
    public RawFile
{
public:
    RawUncompressedFile(StructuredCompressedFile & structuredCompressedFile);
    virtual ~RawUncompressedFile();

    using streamoff = std::streamoff;
    using streamsize = std::streamsize;
    using streampos = std::streampos;

    void open(const char * filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) override;
    bool is_open() const override;
    void close() override;
    std::streamsize read(char * s, streamsize n) override;
    streampos tellg() override;
    void seekg(const streampos pos) override;
    void seekg(const streamoff off, const std::ios_base::seekdir way) override;
    std::streamsize write(const char * s, streamsize n) override;
    streampos tellp() override;
    void seekp(const streampos pos) override;
    void seekp(const streamoff off, const std::ios_base::seekdir way) override;

    /**
     * Get file size
     *
     * @return file size
     */
    virtual streamsize size() const;
    /** end-of-file object */
    Unknown115 unknown115 {};

    /**
     * Get default log container size.
     *
     * @return default log container size
     */
    virtual DWORD defaultLogContainerSize() const;

    /**
     * Set default log container size.
     *
     * @param[in] defaultLogContainerSize default log container size
     */
    virtual void setDefaultLogContainerSize(DWORD defaultLogContainerSize);

private:
    /** log container reference */
    struct LogContainerRef {
        /** file position */
        streampos filePosition {0};

        /**
         * file size
         *
         * @secreflist
         * @refitem LogContainer::uncompressedfileSize
         * @endsecreflist
         */
        streamsize fileSize {0};

        /** uncompressed log container */
        std::vector<char> uncompressedFile {};
    };

    /** mutex */
    mutable std::mutex m_mutex {};

    /** structured compress file */
    StructuredCompressedFile & m_structuredCompressedFile;

    /** open mode */
    std::ios_base::openmode m_openMode {};

    /** get position */
    streampos m_posg {0};

    /** put position */
    streampos m_posp {0};

    /** file size */
    streamsize m_size {0};

    /** log container references (index is StructuredCompressedFile::streampos) */
    std::vector<LogContainerRef> m_logContainerRefs {};

    /** default log container size */
    DWORD m_defaultLogContainerSize {0x20000};

    /**
     * index thread
     *
     * When opening file for reading, the thread scans the file to find all
     * LogContainers.
     */
    void indexThread();

    /**
     * read thread
     *
     * This reads ahead the next LogContainers.
     */
    void readThread();

    /**
     * write thread
     *
     * This writes LogContainers.
     */
    void writeThread();
};

}
}
