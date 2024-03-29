/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
* Copyright 2004-2008 Embedded Access Inc.
* Copyright 1989-2008 ARC International
*
* This software is owned or controlled by Freescale Semiconductor.
* Use of this software is governed by the Freescale MQX RTOS License
* distributed with this Material.
* See the MQX_RTOS_LICENSE file distributed for more details.
*
* Brief License Summary:
* This software is provided in source form for you to use free of charge,
* but it is not open source software. You are allowed to use this software
* but you cannot redistribute it or derivative works of it in source form.
* The software may be used only in connection with a product containing
* a Freescale microprocessor, microcontroller, or digital signal processor.
* See license agreement file for full license terms including other
* restrictions.
*****************************************************************************
*
* Comments:
*
*   This file contains the source for an MFS shell function.
*
*
*END************************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <mqx.h>
#include <shell.h>
#include <sh_prv.h>

#include "nio.h"
#include "fcntl.h"
#include "fs_supp.h"

#if SHELLCFG_USES_MFS
#include <mfs.h>

#define MIN_COPY_BLOCK_SIZE  512
#define MAX_COPY_BLOCK_SIZE 8192

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :  Shell_copy
* Returned Value   :  uint32_t error code
* Comments  :  Renames or moves a file.
*
*END*---------------------------------------------------------------------*/

int32_t  Shell_copy(int32_t argc, char *argv[] )
{ /* Body */
   bool     print_usage, shorthelp = FALSE;
   int32_t      size = 0, wsize = 0, copysize = MAX_COPY_BLOCK_SIZE, return_code = SHELL_EXIT_SUCCESS;
   int in_fd = -1, out_fd = -1;
   char    *abs_path=NULL;
   bool     cache_enabled=TRUE;
   char    *copybuffer = NULL;
   _mqx_int    c;
   SHELL_CONTEXT_PTR    shell_ptr = Shell_get_context( argv );
   int32_t               error = 0;

   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)
   {
      if (argc != 3)
      {
         fprintf(shell_ptr->STDOUT, "Error, invalid number of parameters\n");
         return_code = SHELL_EXIT_ERROR;
         print_usage=TRUE;
      }
      /* check if filesystem is mounted */
      else if (0 > Shell_get_current_filesystem(argv))
      {
         fprintf(shell_ptr->STDOUT, "Error, file system not mounted\n");
         return_code = SHELL_EXIT_ERROR;
      }
      else if (MFS_alloc_path(&abs_path) != MFS_NO_ERROR)
      {
         fprintf(shell_ptr->STDOUT, "Error, unable to allocate memory for paths\n" );
         return_code = SHELL_EXIT_ERROR;
      }
      else
      {
         error = _io_rel2abs(abs_path,shell_ptr->CURRENT_DIR,(char *) argv[1],PATHNAME_SIZE,shell_ptr->CURRENT_DEVICE_NAME);
         if(!error)
         {
             in_fd = open(abs_path, O_RDONLY);
         }

         if (0 > in_fd || error)  {
             fprintf(shell_ptr->STDOUT, "Error, unable to open source file\n" );
             return_code = SHELL_EXIT_ERROR;
         } else {
            error = _io_rel2abs(abs_path,shell_ptr->CURRENT_DIR,(char *) argv[2],PATHNAME_SIZE,shell_ptr->CURRENT_DEVICE_NAME);
            if(!error)
            {
                out_fd = open(abs_path, O_WRONLY | O_CREAT);
            }
            if (0 > out_fd || error)  {
               fprintf(shell_ptr->STDOUT, "Error, unable to create destination file\n" );
               return_code = SHELL_EXIT_ERROR;
            } else {
               MFS_free_path(abs_path);
               abs_path = NULL;

               do {
                  copybuffer = _mem_alloc(copysize);
                  if (copybuffer != NULL) break;
                  else copysize >>= 1;
               } while (copysize >= MIN_COPY_BLOCK_SIZE);

               if (copybuffer == NULL)
               {
                   char ch;
                   fprintf(shell_ptr->STDOUT, "Warning, unable to allocate copy buffer, copy will be slower\n" );

                   while (0 < read(in_fd, &ch, sizeof(ch))) {
                       write(out_fd, &ch, sizeof(ch));
                   }
               }
               else {
                  do {
                     size = read(in_fd, copybuffer, copysize);
                     if (size > 0) {
                        wsize = write(out_fd, copybuffer, size);
                     }
                     else
                        break;
                  } while (wsize == size);
               }
            }
         }
         if (abs_path) MFS_free_path(abs_path);
         if (copybuffer) _mem_free(copybuffer);
         if (0 <= in_fd) close(in_fd);
         if (0 <= out_fd) close(out_fd);
      }
   }


   if (print_usage)  {
      if (shorthelp)  {
         fprintf(shell_ptr->STDOUT, "%s <source> <dest> \n", argv[0]);
      } else  {
         fprintf(shell_ptr->STDOUT, "Usage: %s <source> <dest>\n", argv[0]);
         fprintf(shell_ptr->STDOUT, "   <source> = source file to copy\n");
         fprintf(shell_ptr->STDOUT, "   <dest>   = name of new file\n");
      }
   }
   return return_code;
} /* Endbody */
#endif //SHELLCFG_USES_MFS

/* EOF*/
