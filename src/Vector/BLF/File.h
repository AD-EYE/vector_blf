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

#include <Vector/BLF/StructuredUncompressedFile.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * File
 *
 * This is just a top-level class that abstracts the internal architecture.
 * It also provides compatibility for some deprecated functions.
 */
class VECTOR_BLF_EXPORT File final {
public:
    virtual ~File();

    /**
     * open file
     *
     * @param[in] filename file name
     * @param[in] mode open mode, either in (read) or out (write)
     */
    virtual void open(const std::string & filename, const std::ios_base::openmode mode = std::ios_base::in);

    /**
     * Read object from file.
     *
     * Ownership is taken over from the library to the user.
     * The user has to take care to delete the object.
     *
     * @return read object or nullptr
     *
     * @deprecated Use new "streamsize read(ObjectHeaderBase **)" method instead.
     */
    virtual ObjectHeaderBase * read();

    /* StructuredUncompressedFile pass-thru methods */

    /** @copydoc StructuredUncompressedFile::open() */
    virtual void open(const char * filename, const std::ios_base::openmode mode = std::ios_base::in);

    /** @copydoc StructuredUncompressedFile::is_open() */
    virtual bool is_open() const;

    /** @copydoc StructuredUncompressedFile::close() */
    virtual void close();

    /** @copydoc StructuredUncompressedFile::read() */
    virtual StructuredUncompressedFile::streamsize read(ObjectHeaderBase ** objectHeaderBase);

    /** @copydoc StructuredUncompressedFile::tellg() */
    virtual StructuredUncompressedFile::streampos tellg();

    /** @copydoc StructuredUncompressedFile::seekg(StructuredUncompressedFile::streampos) */
    virtual void seekg(const StructuredUncompressedFile::streampos pos);

    /** @copydoc StructuredUncompressedFile::seekg(StructuredUncompressedFile::streamoff, std::ios_base::seekdir) */
    virtual void seekg(const StructuredUncompressedFile::streamoff off, const std::ios_base::seekdir way);

    /** @copydoc StructuredUncompressedFile::write() */
    virtual bool write(ObjectHeaderBase * objectHeaderBase);

    /** @copydoc StructuredUncompressedFile::tellp() */
    virtual StructuredUncompressedFile::streampos tellp();

    /** @copydoc StructuredUncompressedFile::size() */
    virtual StructuredUncompressedFile::streamsize size() const;

    /** @copydoc StructuredUncompressedFile::setApplication() */
    virtual void setApplication(const BYTE id, const BYTE major = 0, const BYTE minor = 0, const BYTE build = 0);

    /** @copydoc StructuredUncompressedFile::setApi() */
    virtual void setApi(const BYTE major, const BYTE minor, const BYTE build, const BYTE patch);

    /** @copydoc StructuredUncompressedFile::setObjectsRead() */
    virtual void setObjectsRead(const DWORD objectsRead);

    /** @copydoc StructuredUncompressedFile::setMeasurementStartTime() */
    virtual void setMeasurementStartTime(const SYSTEMTIME measurementStartTime);

    /** @copydoc StructuredUncompressedFile::setLastObjectTime() */
    virtual void setLastObjectTime(const SYSTEMTIME lastObjectTime);

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

private:
    /** open mode */
    std::ios_base::openmode m_openMode {};

    /** structured uncompressed file */
    StructuredUncompressedFile m_structuredUncompressedFile {};
};

}
}
