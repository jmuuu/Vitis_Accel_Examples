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

#ifndef XF_BLAS_DDR_HPP
#define XF_BLAS_DDR_HPP

#include "assert.h"
#include "types.hpp"

namespace xf {

namespace blas {

template <typename t_FloatType,
          unsigned int t_DdrWidth,    // In t_FloatType
          unsigned int t_DdrWidthBits // In bits; both must be defined and be consistent
          >
class DdrUtil {
   private:
   public:
    typedef WideType<t_FloatType, t_DdrWidth, t_DdrWidthBits / t_DdrWidth> DdrWideType;

   private:
   public:
};

} // namespace
} // namespace
#endif
