/**********************************************************************************************************************
 * Copyright (c) Prophesee S.A.                                                                                       *
 *                                                                                                                    *
 * Licensed under the Apache License, Version 2.0 (the "License");                                                    *
 * you may not use this file except in compliance with the License.                                                   *
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0                                 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed   *
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.                      *
 * See the License for the specific language governing permissions and limitations under the License.                 *
 **********************************************************************************************************************/

#ifndef __PSEE_ISSD_H
#define __PSEE_ISSD_H

#include <stdint.h>
#include <stddef.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

enum op_id {
    READ,
    WRITE,
    DELAY
};

struct read_op {
    uint16_t addr;
    uint32_t data;
    uint32_t mask;
};

struct write_op {
    uint16_t addr;
    uint32_t data;
};

struct delay_op {
    uint32_t us;
};

struct reg_op {
    enum op_id op;
    union {
        struct read_op read;
        struct write_op write;
        struct delay_op delay;
    } args;
};

struct sequence {
    const struct reg_op *ops;
    size_t len;
};

struct issd {
    const struct sequence init;
    const struct sequence start;
    const struct sequence stop;
    const struct sequence destroy;
};
#endif
