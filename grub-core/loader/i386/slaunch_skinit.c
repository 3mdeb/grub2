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
 */

#include <grub/loader.h>
#include <grub/memory.h>
#include <grub/normal.h>
#include <grub/err.h>
#include <grub/misc.h>
#include <grub/types.h>
#include <grub/dl.h>
#include <grub/slaunch.h>
#include <grub/i386/relocator.h>

static void *
allocate_zeropage(void)
{
  grub_err_t err;
  void *addr = NULL;

  struct grub_relocator *relocator = grub_relocator_new ();
  if (!relocator)
  {
    err = grub_errno;
    goto fail;
  }

  grub_relocator_chunk_t ch;
  {
    err = grub_relocator_alloc_chunk_align (relocator, &ch,
					    0x1000, 0x90000,
					    0x1000, 0x1000,
					    GRUB_RELOCATOR_PREFERENCE_LOW,
					    0);
    if (err)
      goto fail;
  }

  addr = (void *)get_virtual_current_address(ch);

fail:
  grub_relocator_unload(relocator);
  return addr;
}

grub_err_t
grub_slaunch_boot_skinit (struct grub_slaunch_params *slparams)
{
  char *new_zeropage = NULL;
  grub_uint8_t *slb = (grub_uint8_t *)0x2000000;

  grub_printf("%s:%d: real_mode_target: 0x%x\r\n", __FUNCTION__, __LINE__, slparams->real_mode_target);
  grub_printf("%s:%d: prot_mode_target: 0x%x\r\n", __FUNCTION__, __LINE__, slparams->prot_mode_target);
  grub_printf("%s:%d: params: %p\r\n", __FUNCTION__, __LINE__, slparams->params);

  new_zeropage = allocate_zeropage();
  if (new_zeropage == NULL) {
    grub_dprintf("linux", "Couldn't allocate low memory for zeropage\r\n");
    return GRUB_ERR_OUT_OF_MEMORY;
  }
  new_zeropage -= 0x1000;	/* FIXME: memmove throws exception with allocated address, why? */
  grub_printf("%s:%d: new_zeropage: %p\r\n", __FUNCTION__, __LINE__, new_zeropage);

  /* FIXME: some variant of this is done by grub_relocator32_boot(),
   * check if some additional magic code relocation stuff is done there */
  grub_memmove((void*)0x1000000, (void*)0x100000, 6*1024*1024);

  grub_printf("%s:%d: memmove(%p, %p, 0x%lx) \r\n", __FUNCTION__, __LINE__, new_zeropage,
		slparams->params, sizeof(*slparams->params));
  grub_memmove(new_zeropage, slparams->params, sizeof(*slparams->params));
  grub_printf("%s:%d:         DONE\r\n", __FUNCTION__, __LINE__);

  /* FIXME: rewrite into something human-readable */
  *((grub_uint32_t *)&slb[0x18]) = (grub_uint32_t)new_zeropage;
  grub_dprintf("linux", "Invoke SKINIT\r\n");

  if (grub_slaunch_get_modules()) {
    __asm__ ("skinit;"
	     : /* no output */
	     : "a" ( slb )
	     : /* no clobbered reg */
	);

    grub_dprintf("linux", "SKINIT exit\r\n");
  } else {
    grub_dprintf("linux", "Secure Loader module not loaded, run slaunch_module\r\n");
  }
  return GRUB_ERR_NONE;
}
