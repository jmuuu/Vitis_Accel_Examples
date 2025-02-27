/**
* Copyright (C) 2019-2021 Xilinx, Inc
*
* Licensed under the Apache License, Version 2.0 (the "License"). You may
* not use this file except in compliance with the License. A copy of the
* License is located at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
* WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
* License for the specific language governing permissions and limitations
* under the License.
*/

#include "zlib_parallelbyte_decompress_full.hpp"

const int c_historySize = LZ_MAX_OFFSET_LIMIT;

extern "C" {
void xilDecompressFull(uint32_t input_size,
                       hls::stream<ap_axiu<16, 0, 0, 0> >& inaxistreamd,
                       hls::stream<ap_axiu<MULTIPLE_BYTES * 8, 0, 0, 0> >& outaxistreamd,
                       hls::stream<ap_axiu<64, 0, 0, 0> >& sizestreamd) {
#pragma HLS INTERFACE s_axilite port = input_size bundle = control
#pragma HLS interface axis port = inaxistreamd
#pragma HLS interface axis port = outaxistreamd
#pragma HLS interface axis port = sizestreamd
#pragma HLS INTERFACE s_axilite port = return bundle = control

    // Call for decompression
    xf::compression::inflateMultiByte<DECODER_TYPE, MULTIPLE_BYTES, HIGH_FMAX_II, c_historySize>(
        inaxistreamd, outaxistreamd, sizestreamd, input_size);
}
}
