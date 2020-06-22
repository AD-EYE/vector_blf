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

#include <memory>
#include <mutex>
#include <vector>

#include <Vector/BLF/RawUncompressedFile.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * Structured Uncompressed File
 *
 * The StructuredUncompressedFile is the 4. layer in the file architecture.
 *
 * It provides structure-wise access to the objects in the RawUncompressedFile.
 *
 * This class is thread-safe.
 */
class VECTOR_BLF_EXPORT StructuredUncompressedFile {
public:
    ~StructuredUncompressedFile();

    /** stream offset */
    using streamoff = int32_t;

    /** stream size */
    using streamsize = uint32_t;

    /** stream position */
    using streampos = uint32_t;

    /** @copydoc RawFile::open() */
    virtual void open(const char * filename, std::ios_base::openmode mode = std::ios_base::in);

    /** @copydoc RawFile::is_open() */
    virtual bool is_open() const;

    /** @copydoc RawFile::close() */
    virtual void close();

    /**
     * Read object.
     *
     * This operation blocks until the object is available.
     *
     * @param[out] objectHeaderBase object
     * @return Number of objects read (0 or 1)
     */
    virtual streamsize read(ObjectHeaderBase ** objectHeaderBase);

    /** @copydoc RawFile::tellg() */
    virtual streampos tellg() const;

    /** @copydoc RawFile::seekg(std::streampos) */
    virtual void seekg(const streampos pos);

    /** @copydoc RawFile::seekg(std::streamoff, std::ios_base::seekdir) */
    virtual void seekg(const streamoff off, const std::ios_base::seekdir way);

    /**
     * Write object.
     *
     * @param[in] objectHeaderBase object
     * @return Number of objects written (0 or 1)
     */
    virtual streamsize write(ObjectHeaderBase * objectHeaderBase);

    /** @copydoc RawFile::tellp() */
    virtual streampos tellp() const;

    /** @copydoc RawFile::size() */
    virtual streamsize size() const;

    /* RawUncompressedFile pass-thru methods */

    /** @copydoc RawUncompressedFile::size() */
    virtual RawUncompressedFile::streamsize rawUncompressedFileSize() const;

    /** @copydoc RawUncompressedFile::statisticsSize() */
    virtual RawUncompressedFile::streamsize rawUncompressedFileStatisticsSize() const;

    /** @copydoc RawUncompressedFile::defaultLogContainerSize() */
    virtual DWORD defaultLogContainerSize() const;

    /** @copydoc RawUncompressedFile::setDefaultLogContainerSize() */
    virtual void setDefaultLogContainerSize(DWORD defaultLogContainerSize);

    /** @copydoc RawUncompressedFile::compressionMethod() */
    virtual int compressionMethod() const;

    /** @copydoc RawUncompressedFile::setCompressionMethod() */
    virtual void setCompressionMethod(const int compressionMethod = 2);

    /** @copydoc RawUncompressedFile::compressionLevel() */
    virtual int compressionLevel() const;

    /** @copydoc RawUncompressedFile::setCompressionLevel() */
    virtual void setCompressionLevel(const int compressionLevel = 6);

    /* StructuredCompressedFile pass-thru methods */

    /** @copydoc StructuredCompressedFile::size() */
    virtual StructuredCompressedFile::streamsize structuredCompressedFileSize() const;

    /* RawCompressedFile pass-thru methods */

    /** @copydoc RawCompressedFile::size() */
    virtual RawCompressedFile::streamsize rawCompressedFileSize() const;

    /** @copydoc RawCompressedFile::statistics() */
    virtual FileStatistics statistics() const;

    /** @copydoc RawUncompressedFile::setApplication() */
    virtual void setApplication(const BYTE id, const BYTE major = 0, const BYTE minor = 0, const BYTE build = 0);

    /** @copydoc RawUncompressedFile::setApi() */
    virtual void setApi(const BYTE major, const BYTE minor, const BYTE build, const BYTE patch);

    /** @copydoc RawUncompressedFile::setObjectsRead() */
    virtual void setObjectsRead(const DWORD objectsRead);

    /** @copydoc RawUncompressedFile::setMeasurementStartTime() */
    virtual void setMeasurementStartTime(const SYSTEMTIME measurementStartTime);

    /** @copydoc RawUncompressedFile::setLastObjectTime() */
    virtual void setLastObjectTime(const SYSTEMTIME lastObjectTime);

private:
    /** object reference */
    struct ObjectRef {
        /** file position */
        RawUncompressedFile::streampos filePosition {0};

        /**
         * object size
         *
         * @see ObjectHeaderBase::objectSize
         */
        DWORD objectSize {0};

        /**
         * object type
         *
         * @see ObjectHeaderBase::objectType
         */
        ObjectType objectType {ObjectType::UNKNOWN};

        // @todo cache object here
    };

    /** mutex */
    mutable std::mutex m_mutex {};

    /** raw uncompressed file */
    RawUncompressedFile m_rawUncompressedFile {};

    /** open mode */
    std::ios_base::openmode m_openMode {};

    /** get position */
    streampos m_posg{0};

    /** object references (index is streampos) */
    std::vector<ObjectRef> m_objectRefs {};

    /* threads */

    /**
     * index thread
     *
     * When opening file for reading, the thread scans the file to find all
     * objects.
     */
    void indexThread();

    /**
     * read thread
     *
     * This reads ahead the next objects.
     */
    void readThread();

    /**
     * write thread
     *
     * This writes objects.
     */
    void writeThread();
};

}
}
