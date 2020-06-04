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

#include <Vector/BLF/ObjectHeader.h>
#include <Vector/BLF/RawFile.h>
#include <Vector/BLF/VectorTypes.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/** line A/B information in AfdxStatus class */
struct VECTOR_BLF_EXPORT AfdxLineStatus final {
    AfdxLineStatus() noexcept = default;
    virtual ~AfdxLineStatus() noexcept = default;
    AfdxLineStatus(const AfdxLineStatus &) = default;
    AfdxLineStatus & operator=(const AfdxLineStatus &) = default;
    AfdxLineStatus(AfdxLineStatus &&) = default;
    AfdxLineStatus & operator=(AfdxLineStatus &&) = default;

    /** @copydoc ObjectHeader::read */
    virtual void read(RawFile & is);

    /** @copydoc ObjectHeader::write */
    virtual void write(RawFile & os);

    /** @copydoc ObjectHeader::calculateObjectSize */
    virtual DWORD calculateObjectSize() const;

    /**
     * Flags
     *   - Bit 0 - Link Status
     *   - Bit 1 - Bitrate
     *   - Bit 2 - Ethernet Phy
     *   - Bit 3 - Duplex
     */
    WORD flags {};

    /**
     * Link Status
     *   - 0 - Unknown
     *   - 1 - Link down
     *   - 2 - Link up
     *   - 3 - Negotiate
     *   - 4 - Link error
     */
    BYTE linkStatus {};

    /**
     * Eternet Phy
     *   - 0 - Unknown
     *   - 1 - IEEE 802.3
     *   - 2 - BroadR-Reach
     */
    BYTE ethernetPhy {};

    /**
     * Duplex
     *   - 0 - Unknown
     *   - 1 - Half Duplex
     *   - 2 - Full Duplex
     */
    BYTE duplex {};

    /**
     * MDI
     *   - 0 - Unknown
     *   - 1 - Direct
     *   - 2 - Crossover
     */
    BYTE mdi {};

    /**
     * Connector
     *   - 0 - Unknown
     *   - 1 - RJ45
     *   - 2 - D-Sub
     */
    BYTE connector {};

    /**
     * Clock Mode
     *   - 0 - Unknown
     *   - 1 - Master
     *   - 2 - Slave
     */
    BYTE clockMode {};

    /**
     * Pairs
     *   - 0 - Unknown
     *   - 1 - BR 1-pair
     *   - 2 - BR 2-pair
     *   - 3 - BR 4-pair
     */
    BYTE pairs {};

    /** reserved */
    BYTE reservedAfdxLineStatus1 {};

    /** reserved */
    WORD reservedAfdxLineStatus2 {};

    /**
     * Bitrate in [kbit/sec]
     */
    ULONG bitrate {};
};

}
}
