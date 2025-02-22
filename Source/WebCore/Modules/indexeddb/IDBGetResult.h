/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef IDBGetResult_h
#define IDBGetResult_h

#if ENABLE(INDEXED_DATABASE)

#include "IDBKey.h"
#include "IDBKeyData.h"
#include "IDBKeyPath.h"
#include "IDBValue.h"
#include "SharedBuffer.h"

namespace WebCore {

class IDBGetResult {
public:
    IDBGetResult()
        : m_isDefined(false)
    {
    }

    IDBGetResult(Ref<SharedBuffer>&& buffer, const IDBKeyData& currentPrimaryKey)
        : m_primaryKeyData(currentPrimaryKey)
    {
        dataFromBuffer(buffer.get());
    }

    IDBGetResult(const ThreadSafeDataBuffer& buffer)
        : m_value(buffer)
    {
    }

    IDBGetResult(PassRefPtr<IDBKey> key)
        : m_keyData(key.get())
    {
    }

    IDBGetResult(const IDBKeyData& keyData)
        : m_keyData(keyData)
    {
    }

    IDBGetResult(PassRefPtr<SharedBuffer> buffer, PassRefPtr<IDBKey> key, const IDBKeyPath& path)
        : m_keyData(key.get())
        , m_keyPath(path)
    {
        if (buffer)
            dataFromBuffer(*buffer);
    }

    IDBGetResult(const IDBKeyData& keyData, const IDBKeyData& primaryKeyData)
        : m_keyData(keyData)
        , m_primaryKeyData(primaryKeyData)
    {
    }

    IDBGetResult(const IDBKeyData& keyData, const IDBKeyData& primaryKeyData, const ThreadSafeDataBuffer& valueBuffer)
        : m_value(valueBuffer)
        , m_keyData(keyData)
        , m_primaryKeyData(primaryKeyData)
    {
    }

    IDBGetResult isolatedCopy() const;

    const IDBValue& value() const { return m_value; }
    const IDBKeyData& keyData() const { return m_keyData; }
    const IDBKeyData& primaryKeyData() const { return m_primaryKeyData; }
    const IDBKeyPath& keyPath() const { return m_keyPath; }
    bool isDefined() const { return m_isDefined; }

    template<class Encoder> void encode(Encoder&) const;
    template<class Decoder> static bool decode(Decoder&, IDBGetResult&);

private:
    WEBCORE_EXPORT void dataFromBuffer(SharedBuffer&);

    IDBValue m_value;
    IDBKeyData m_keyData;
    IDBKeyData m_primaryKeyData;
    IDBKeyPath m_keyPath;
    bool m_isDefined { true };
};

template<class Encoder>
void IDBGetResult::encode(Encoder& encoder) const
{
    encoder << m_keyData << m_primaryKeyData << m_keyPath << m_isDefined << m_value;
}

template<class Decoder>
bool IDBGetResult::decode(Decoder& decoder, IDBGetResult& result)
{
    if (!decoder.decode(result.m_keyData))
        return false;

    if (!decoder.decode(result.m_primaryKeyData))
        return false;

    if (!decoder.decode(result.m_keyPath))
        return false;

    if (!decoder.decode(result.m_isDefined))
        return false;

    if (!decoder.decode(result.m_value))
        return false;

    return true;
}

} // namespace WebCore

#endif // ENABLE(INDEXED_DATABASE)
#endif // IDBGetResult_h
