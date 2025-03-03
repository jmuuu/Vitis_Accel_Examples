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
/**
 * @file zlib_dm_wr.cpp
 * @brief Source for data writer kernel for streaming data to zlib decompression
 * streaming kernel.
 *
 * This file is part of Vitis Data Compression Library.
 */

#include "zlib_dm_multibyte_wr.hpp"

const int kGMemBurstSize = 16;

template <uint16_t STREAMDWIDTH>
void streamDataDm2kSync(hls::stream<ap_uint<STREAMDWIDTH> >& in,
                        hls::stream<ap_axiu<STREAMDWIDTH, 0, 0, 0> >& inStream_dm,
                        uint32_t inputSize) {
    // read data from input hls to input stream for decompression kernel
    uint32_t itrLim = 1 + (inputSize - 1) / (STREAMDWIDTH / 8);
streamDataDm2kSync:
    for (uint32_t i = 0; i < itrLim; i++) {
#pragma HLS PIPELINE II = 1
        ap_uint<STREAMDWIDTH> temp = in.read();
        ap_axiu<STREAMDWIDTH, 0, 0, 0> dataIn;
        dataIn.data = temp; // kernel to kernel data transfer
        inStream_dm.write(dataIn);
    }
}

void zlib_dm_wr(uintMemWidth_t* in, uint32_t input_size, hls::stream<ap_axiu<16, 0, 0, 0> >& inAxiStream) {
    hls::stream<uintMemWidth_t> inHlsStream("inputStream");
    hls::stream<ap_uint<16> > outdownstream("outDownStream");
#pragma HLS STREAM variable = inHlsStream depth = 512
#pragma HLS STREAM variable = outdownstream depth = 32
#pragma HLS RESOURCE variable = outdownstream core = FIFO_SRL

#pragma HLS dataflow
    xf::compression::details::mm2sSimple<MULTIPLE_BYTES * 8, 64>(in, inHlsStream, input_size);
    xf::compression::details::streamDownsizer<uint32_t, MULTIPLE_BYTES * 8, 16>(inHlsStream, outdownstream, input_size);
    streamDataDm2kSync<16>(outdownstream, inAxiStream, input_size);
}

extern "C" {
void xilZlibDmWriter(uintMemWidth_t* in, uint32_t inputSize, hls::stream<ap_axiu<16, 0, 0, 0> >& instreamk) {
#pragma HLS INTERFACE m_axi port = in offset = slave bundle = gmem0 max_read_burst_length = \
    64 max_write_burst_length = 2 num_read_outstanding = 16 num_write_outstanding = 1
#pragma HLS interface axis port = instreamk
#pragma HLS INTERFACE s_axilite port = in bundle = control
#pragma HLS INTERFACE s_axilite port = inputSize bundle = control
#pragma HLS INTERFACE s_axilite port = return bundle = control

    zlib_dm_wr(in, inputSize, instreamk);
}
}
