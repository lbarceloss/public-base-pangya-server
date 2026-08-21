
#pragma once
#ifndef _STDA_COMPRESS_H
#define _STDA_COMPRESS_H

#include "minilzo.h"

#define HEAP_ALLOC(var,size) \
    lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]

namespace stdA {
    class compress {
        public:
            compress();
            ~compress();

            void compress_data(unsigned char *_uncompress, size_t _size_uncompress, unsigned char *_compress, size_t *_size_compress);

            void decompress_data(unsigned char *_compress,
								 size_t _size_compress,
								 unsigned char *_uncompress,
								 size_t *_size_uncompress,
								 size_t _size_decompress		 );

            int getLastError();

        protected:
            HEAP_ALLOC(m_wrkmem, LZO1X_1_MEM_COMPRESS);
            int m_error;
    };
}

#endif
