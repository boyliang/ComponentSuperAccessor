/*
 * Copyright (c) 2008, 2009, Google Inc. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "ICOImageDecoder.h"

#include <algorithm>

#include "BMPImageReader.h"
#include "PNGImageDecoder.h"

namespace WebCore {

// Number of bits in .ICO/.CUR used to store the directory and its entries,
// respectively (doesn't match sizeof values for member structs since we omit
// some fields).
static const size_t sizeOfDirectory = 6;
static const size_t sizeOfDirEntry = 16;

ICOImageDecoder::ICOImageDecoder()
    : ImageDecoder()
    , m_allDataReceived(false)
    , m_decodedOffset(0)
{
}

ICOImageDecoder::~ICOImageDecoder()
{
    deleteAllValues(m_bmpReaders);
    deleteAllValues(m_pngDecoders);
}

void ICOImageDecoder::setData(SharedBuffer* data, bool allDataReceived)
{
    if (failed())
        return;

    ImageDecoder::setData(data, allDataReceived);
    m_allDataReceived = allDataReceived;

    for (BMPReaders::iterator i(m_bmpReaders.begin());
         i != m_bmpReaders.end(); ++i) {
        if (*i)
            (*i)->setData(data);
    }
    for (size_t i = 0; i < m_pngDecoders.size(); ++i)
        setDataForPNGDecoderAtIndex(i);
}

bool ICOImageDecoder::isSizeAvailable()
{
    if (!ImageDecoder::isSizeAvailable())
        decodeWithCheckForDataEnded(0, true);

    return ImageDecoder::isSizeAvailable();
}

IntSize ICOImageDecoder::size() const
{
    return m_frameSize.isEmpty() ? ImageDecoder::size() : m_frameSize;
}

IntSize ICOImageDecoder::frameSizeAtIndex(size_t index) const
{
    return (index && (index < m_dirEntries.size())) ?
        m_dirEntries[index].m_size : size();
}

bool ICOImageDecoder::setSize(unsigned width, unsigned height)
{
    if (m_frameSize.isEmpty())
        return ImageDecoder::setSize(width, height);

    // The size calculated inside the BMPImageReader had better match the one in
    // the icon directory.
    if (IntSize(width, height) != m_frameSize)
        setFailed();
    return !failed();
}

size_t ICOImageDecoder::frameCount()
{
    decodeWithCheckForDataEnded(0, true);
    if (m_frameBufferCache.isEmpty())
        m_frameBufferCache.resize(m_dirEntries.size());
    // CAUTION: We must not resize m_frameBufferCache again after this, as
    // decodeAtIndex() may give a BMPImageReader a pointer to one of the
    // entries.
    return m_frameBufferCache.size();
}

RGBA32Buffer* ICOImageDecoder::frameBufferAtIndex(size_t index)
{
    // Ensure |index| is valid.
    if (index >= frameCount())
        return 0;

    // Determine the image type, and if this is a BMP, decode.
    decodeWithCheckForDataEnded(index, false);

    // PNGs decode into their own framebuffers, so only use our internal cache
    // for non-PNGs (BMP or unknown).
    if (!m_pngDecoders[index])
        return &m_frameBufferCache[index];

    // Fail if the size the PNGImageDecoder calculated does not match the size
    // in the directory.
    if (m_pngDecoders[index]->isSizeAvailable()) {
        const IntSize pngSize(m_pngDecoders[index]->size());
        const IconDirectoryEntry& dirEntry = m_dirEntries[index];
        if (pngSize != dirEntry.m_size) {
            setFailed();
            m_pngDecoders[index]->setFailed();
        }
    }

    return m_pngDecoders[index]->frameBufferAtIndex(0);
}

// static
bool ICOImageDecoder::compareEntries(const IconDirectoryEntry& a,
                                     const IconDirectoryEntry& b)
{
    // Larger icons are better.
    const int aEntryArea = a.m_size.width() * a.m_size.height();
    const int bEntryArea = b.m_size.width() * b.m_size.height();
    if (aEntryArea != bEntryArea)
        return (aEntryArea > bEntryArea);

    // Higher bit-depth icons are better.
    return (a.m_bitCount > b.m_bitCount);
}

void ICOImageDecoder::setDataForPNGDecoderAtIndex(size_t index)
{
    if (!m_pngDecoders[index])
        return;

    const IconDirectoryEntry& dirEntry = m_dirEntries[index];
    // Copy out PNG data to a separate vector and send to the PNG decoder.
    // FIXME: Save this copy by making the PNG decoder able to take an
    // optional offset.
    RefPtr<SharedBuffer> pngData(
        SharedBuffer::create(&m_data->data()[dirEntry.m_imageOffset],
                             m_data->size() - dirEntry.m_imageOffset));
    m_pngDecoders[index]->setData(pngData.get(), m_allDataReceived);
}

void ICOImageDecoder::decodeWithCheckForDataEnded(size_t index, bool onlySize)
{
    if (failed())
        return;

    // If we couldn't decode the image but we've received all the data, decoding
    // has failed.
    if ((!decodeDirectory() || (!onlySize && !decodeAtIndex(index)))
        && m_allDataReceived)
        setFailed();
}

bool ICOImageDecoder::decodeDirectory()
{
    // Read and process directory.
    if ((m_decodedOffset < sizeOfDirectory) && !processDirectory())
        return false;

    // Read and process directory entries.
    return (m_decodedOffset >=
            (sizeOfDirectory + (m_dirEntries.size() * sizeOfDirEntry)))
        || processDirectoryEntries();
}

bool ICOImageDecoder::decodeAtIndex(size_t index)
{
    ASSERT(index < m_dirEntries.size());
    const IconDirectoryEntry& dirEntry = m_dirEntries[index];
    if (!m_bmpReaders[index] && !m_pngDecoders[index]) {
        // Image type unknown.
        const ImageType imageType = imageTypeAtIndex(index);
        if (imageType == BMP) {
            // We need to have already sized m_frameBufferCache before this, and
            // we must not resize it again later (see caution in frameCount()).
            ASSERT(m_frameBufferCache.size() == m_dirEntries.size());
            m_bmpReaders[index] =
                new BMPImageReader(this, dirEntry.m_imageOffset, 0, true);
            m_bmpReaders[index]->setData(m_data.get());
            m_bmpReaders[index]->setBuffer(&m_frameBufferCache[index]);
        } else if (imageType == PNG) {
            m_pngDecoders[index] = new PNGImageDecoder();
            setDataForPNGDecoderAtIndex(index);
        } else {
            // Not enough data to determine image type yet.
            return false;
        }
    }

    if (m_bmpReaders[index]) {
        m_frameSize = dirEntry.m_size;
        bool result = m_bmpReaders[index]->decodeBMP(false);
        m_frameSize = IntSize();
        return result;
    }

    // For PNGs, we're now done; further decoding will happen when calling
    // frameBufferAtIndex() on the PNG decoder.
    return true;
}

bool ICOImageDecoder::processDirectory()
{
    // Read directory.
    ASSERT(!m_decodedOffset);
    if (m_data->size() < sizeOfDirectory)
        return false;
    const uint16_t fileType = readUint16(2);
    const uint16_t idCount = readUint16(4);
    m_decodedOffset = sizeOfDirectory;

    // See if this is an icon filetype we understand, and make sure we have at
    // least one entry in the directory.
    enum {
        ICON = 1,
        CURSOR = 2,
    };
    if (((fileType != ICON) && (fileType != CURSOR)) || (idCount == 0)) {
        setFailed();
        return false;
    }

    // Enlarge member vectors to hold all the entries.  We must initialize the
    // BMP and PNG decoding vectors to 0 so that all entries can be safely
    // deleted in our destructor.  If we don't do this, they'll contain garbage
    // values, and deleting those will corrupt memory.
    m_dirEntries.resize(idCount);
    m_bmpReaders.fill(0, idCount);
    m_pngDecoders.fill(0, idCount);
    return true;
}

bool ICOImageDecoder::processDirectoryEntries()
{
    // Read directory entries.
    ASSERT(m_decodedOffset == sizeOfDirectory);
    if ((m_decodedOffset > m_data->size())
        || ((m_data->size() - m_decodedOffset) <
            (m_dirEntries.size() * sizeOfDirEntry)))
        return false;
    for (IconDirectoryEntries::iterator i(m_dirEntries.begin());
         i != m_dirEntries.end(); ++i)
        *i = readDirectoryEntry();  // Updates m_decodedOffset.

    // Make sure the specified image offsets are past the end of the directory
    // entries.
    for (IconDirectoryEntries::iterator i(m_dirEntries.begin());
         i != m_dirEntries.end(); ++i) {
        if (i->m_imageOffset < m_decodedOffset) {
            setFailed();
            return false;
        }
    }

    // Arrange frames in decreasing quality order.
    std::sort(m_dirEntries.begin(), m_dirEntries.end(), compareEntries);

    // The image size is the size of the largest entry.
    const IconDirectoryEntry& dirEntry = m_dirEntries.first();
    setSize(dirEntry.m_size.width(), dirEntry.m_size.height());
    return true;
}

ICOImageDecoder::IconDirectoryEntry ICOImageDecoder::readDirectoryEntry()
{
    // Read icon data.
    // The casts to uint8_t in the next few lines are because that's the on-disk
    // type of the width and height values.  Storing them in ints (instead of
    // matching uint8_ts) is so we can record dimensions of size 256 (which is
    // what a zero byte really means).
    int width = static_cast<uint8_t>(m_data->data()[m_decodedOffset]);
    if (width == 0)
        width = 256;
    int height = static_cast<uint8_t>(m_data->data()[m_decodedOffset + 1]);
    if (height == 0)
        height = 256;
    IconDirectoryEntry entry;
    entry.m_size = IntSize(width, height);
    entry.m_bitCount = readUint16(6);
    entry.m_imageOffset = readUint32(12);

    // Some icons don't have a bit depth, only a color count.  Convert the
    // color count to the minimum necessary bit depth.  It doesn't matter if
    // this isn't quite what the bitmap info header says later, as we only use
    // this value to determine which icon entry is best.
    if (!entry.m_bitCount) {
        uint8_t colorCount = m_data->data()[m_decodedOffset + 2];
        if (colorCount) {
            for (--colorCount; colorCount; colorCount >>= 1)
                ++entry.m_bitCount;
        }
    }

    m_decodedOffset += sizeOfDirEntry;
    return entry;
}

ICOImageDecoder::ImageType ICOImageDecoder::imageTypeAtIndex(size_t index)
{
    // Check if this entry is a BMP or a PNG; we need 4 bytes to check the magic
    // number.
    ASSERT(index < m_dirEntries.size());
    const uint32_t imageOffset = m_dirEntries[index].m_imageOffset;
    if ((imageOffset > m_data->size()) || ((m_data->size() - imageOffset) < 4))
        return Unknown;
    return strncmp(&m_data->data()[imageOffset], "\x89PNG", 4) ? BMP : PNG;
}

}
