// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _SSL_LIB_H_
#define _SSL_LIB_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "ssl_types.h"

 void _ssl_set_alpn_list(const SSL *ssl);

#ifdef __cplusplus
}
#endif

#endif
