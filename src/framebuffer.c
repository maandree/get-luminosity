/**
 * get-luminosity — Determine how bright it is in your room
 * Copyright © 2014  Mattias Andrée (maandree@member.fsf.org)
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "linearise.h"

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stropts.h>
#include <linux/fb.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>


int main()
{
  auto struct fb_var_screeninfo var_info;
  long width;
  long height;
  long y;
  auto int fd;
  auto const char* restrict mem;
  register double grand_sum = 0;
  double inv_width;
  
  if (fd = open("/dev/fb0", O_RDONLY), fd == -1)
    return perror("get-luminosity::framebuffer: open: /dev/fb0"), close(fd), 1;
  
  if (ioctl(fd, (unsigned long)FBIOGET_VSCREENINFO, &var_info))
    return perror("get-luminosity::framebuffer: ioctl"), close(fd), 1;
  
  width       = var_info.xres;
  height      = var_info.yres;
  inv_width   = 1 / (double)width;
  
  mem = mmap(NULL, (size_t)(width * height * 4), PROT_READ, MAP_SHARED, fd, (off_t)0);
  if (mem == MAP_FAILED)
    return perror("get-luminosity::framebuffer: mmap"), close(fd), 1;
  
  for (y = 0; y < height; y++)
    {
      register const int32_t* restrict linemem = (const int32_t*)(mem + y * width * 4);
      register const int32_t* restrict end     = linemem + width;
      double sum = 0;
      while (linemem != end)
	{
	  int32_t value = *linemem++;
	  sum += linearise(value, 0) + linearise(value, 1) + linearise(value, 2);
	}
      grand_sum += sum * inv_width;
    }
  grand_sum /= (double)(height * 3);
  
  fprintf(stderr, "%lf\n", (double)grand_sum);
  fflush(stdout);
  
  close(fd);
  return 0;
}

