/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (c) 2019 Oracle and/or its affiliates. All rights reserved.
 *
 *  GRUB is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GRUB is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GRUB.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Main secure launch definitions header file.
 */

#ifndef GRUB_SLAUNCH_H
#define GRUB_SLAUNCH_H 1

#include <grub/types.h>
#include <grub/i386/linux.h>

#define GRUB_SL_BOOTPARAMS_OFFSET	0x12c

struct grub_slaunch_info
{
  grub_uint32_t sl_version;
  grub_uint32_t sl_entry;   /* Field updated by boot build tool */
  grub_uint32_t sl_mle_hdr; /* Field updated by boot build tool */
  grub_uint32_t sl_flags;
  grub_uint32_t sl_dev_map;
} GRUB_PACKED;

struct grub_slaunch_params
{
  struct linux_kernel_params *params;
  grub_addr_t real_mode_target;
  grub_addr_t prot_mode_target;
};

struct grub_slaunch_module
{
  struct grub_slaunch_module *next;
  grub_uint8_t *addr;
  grub_addr_t target;
  grub_size_t size;
};

struct grub_slaunch_module *grub_slaunch_get_modules (void);

grub_err_t grub_slaunch_boot_txt (struct grub_slaunch_params *slparams);
grub_err_t grub_slaunch_boot_skinit (struct grub_slaunch_params *slparams);

void grub_linux_slaunch_set (grub_err_t (*sfunc) (struct grub_slaunch_params*));

#endif
