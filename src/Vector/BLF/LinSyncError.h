/*
 * Copyright (C) 2013 Tobias Lorenz.
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

#include "platform.h"

#include "VectorTypes.h"
#include "ObjectHeader.h"

#include "vector_blf_export.h"

namespace Vector {
namespace BLF {

/**
 * @brief LIN_SYNC_ERROR
 *
 * Synchronization errors occur if the LIN hardware cannot synchronize with an external
 * Master. This might happen if the baud rate actually used by the Master deviates by more than 15 %
 * from the baud rate specified by the LIN hardware. In this case the baud rate value should be
 * modified. This error event may also occur if the Master transmits an invalid or corrupted Sync
 * field.
 */
class VECTOR_BLF_EXPORT LinSyncError : public ObjectHeader
{
public:
    LinSyncError();

    /**
     * @brief application channel
     *
     * Channel number where the event notified.
     */
    WORD channel;

    /**
     * Reserved, has to be set to 0.
     */
    WORD dummy;

    /**
     * Time intervals [in us] detected between the
     * falling signal edges of the Sync field
     */
    WORD timeDiff[4];
};

}
}
