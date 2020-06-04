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

/**
 * @brief CAN_ERROR
 *
 * CAN error frame received or transmitted on a CAN channel.
 */
struct VECTOR_BLF_EXPORT CanErrorFrame final : ObjectHeader {
    CanErrorFrame();

    void read(RawFile & is) override;
    void write(RawFile & os) override;
    DWORD calculateObjectSize() const override;

    /**
     * @brief application channel
     *
     * Channel the frame was sent or received.
     */
    WORD channel {};

    /**
     * @brief CAN error frame length
     *
     * Length of error frame - can be left 0.
     */
    WORD length {};

    /** reserved */
    DWORD reservedCanErrorFrame {};
};

}
}
