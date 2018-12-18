/*
 * Copyright (c) 2015, Mellanox Semiconductor, Ltd. and Contributors. All
 * rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of Mellanox nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __BLUEFIELD_BOOT_H__
#define __BLUEFIELD_BOOT_H__

#define BFB_IMGHDR_MAGIC	0x13026642  /* "Bf^B^S" */

#define BFB_IMGHDR_MAJOR	1
#define BFB_IMGHDR_MINOR	1

/*
 * Boot stream format
 *
 * A BlueField boot stream is composed of one or more images, each preceded
 * by an image header.  An image might be instructions and data to be
 * executed or consumed by the processor, or metadata to be used by the
 * boot process itself; examples of the latter include digital signatures
 * or keys for secure boot operations.
 *
 * This header must be compiled in LP64 mode, as is reasonable since it is
 * only used by low-level 64-bit boot software; this is enforced by the use
 * of a ":64" suffix on the "unsigned long following_images" field, which
 * will cause a compiler error if not.
 */

union boot_image_header {
	struct {
		/* Magic number; must be BFB_IMGHDR_MAGIC. */
		unsigned long magic:32;
		/*
		 * Major version number.  If this number is increased,
		 * it indicates an incompatible change in the header format
		 * or image contents, to the extent that software written
		 * with a previous major version in mind should refuse to
		 * process the new header.
		 */
		unsigned long major:4;
		/*
		 * Minor version number.  If this number is increased, it
		 * indicates an upward-compatible change in the header
		 * format or image contents, to the extent that software
		 * written with a previous minor version in mind can still
		 * process the new header.
		 */
		unsigned long minor:4;
		/*
		 * Reserved for future expansion.  Should be ignored by
		 * readers and set to zero by writers.
		 */
		unsigned long reserved:12;
		/*
		 * Length of this header, in 8-byte words.  Software
		 * processing the header should use this value to determine
		 * the header's length; if this is longer than expected
		 * based on the major/minor version numbers, ignore and
		 * discard any extra words.
		 */
		unsigned long hdr_len:4;
		/*
		 * ID or type of the image.  This is the same value used
		 * internally by ARM Trusted Firmware; e.g., BL2_ID, or 2,
		 * for the BL2 image; BL33_CERT_ID, or 15, for the BL3-3
		 * certificate; and so forth.
		 */
		unsigned long image_id:8;
		/*
                 * Length of the image in bytes.  Note that if the byte
                 * count is not an integral multiple of 8, the image will
                 * be padded with zeros to make it occupy an integral
                 * number of 8-byte words in the stream, but this value is
                 * the unpadded size.
		 */
		unsigned long image_len:32;
		/*
		 * CRC-32 of the image, including any added padding bytes.
		 */
		unsigned long image_crc:32;
		/*
		 * Bitmap of the IDs of any images following this one in
		 * the stream, not including the image described by this
		 * header.  Thus, if a stream included images of types
		 * 0, 1, and 2, in that order, the following_images values
		 * for the three headers would be 0x6, 0x4, and 0x0
		 * respectively.
		 */
		unsigned long following_images:64;
	} data;
	unsigned long words[3];
};


#endif /* __BFB_BOOT_H__ */
