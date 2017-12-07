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

#include <array>

#include "AbstractFile.h"
#include "ObjectHeader.h"
#include "VectorTypes.h"

#include "vector_blf_export.h"

namespace Vector {
namespace BLF {

/**
 * @brief AFDX_ERROR_EVENT
 *
 * AFDX general error event, available since 8.5
 */
class VECTOR_BLF_EXPORT AfdxErrorEvent final : public ObjectHeader
{
public:
    AfdxErrorEvent();

    virtual void read(AbstractFile & is) override;
    virtual void write(AbstractFile & os) override;
    virtual DWORD calculateObjectSize() const override;

    /**
     * @brief application channel
     */
    WORD channel;

    /**
     * @brief Error Level
     *   - 0=error
     *   - 1=warning
     *   - 2=info
     */
    WORD errorLevel;

    /** source identifier */
    ULONG sourceIdentifier;

    /** error text */
    std::array<char, 512> errorText;

    /** error attributes */
    std::array<char, 512> errorAttributes;
};

}
}