/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __JSVC_DSO_H__
#define __JSVC_DSO_H__

#include <stdbool.h>

#include <intl_cfunc.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A library handle represents a unique pointer to its location in memory.
 */
#ifdef DSO_DYLD
#include <mach-o/dyld.h>
#endif
typedef void *dso_handle;

bool INTL_CFUNC(dso_init)(void);
dso_handle INTL_CFUNC(dso_link)(const char *pth);
bool  INTL_CFUNC(dso_unlink)(dso_handle lib);
void *INTL_CFUNC(dso_symbol)(dso_handle lib, const char *nam);
char *INTL_CFUNC(dso_error)(void);

#ifdef __cplusplus
};
#endif

#endif /* __JSVC_DSO_H__ */

