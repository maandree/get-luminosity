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

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <pthread.h>


static Display* restrict display;


static double get_brightness(int screen)
{
  auto Window root = XRootWindow(display, screen);
  auto long width  = (long)XDisplayWidth(display, screen);
  auto long height = (long)XDisplayHeight(display, screen);
  auto XImage* restrict image = XGetImage(display, root, 0, 0, width, height, AllPlanes, ZPixmap);
  register double sum = 0;
  register unsigned char* restrict pixels;
  register unsigned char* restrict end;
  
  if (image == NULL)
    return -1;
  
  pixels = (unsigned char*)(image->data);
  end = pixels + width * height * 4;
  
  while (pixels != end)
    {
      sum += LINEAR_MAP[*pixels++];
      sum += LINEAR_MAP[*pixels++];
      sum += LINEAR_MAP[*pixels++];
      pixels++;
    }
  
  XDestroyImage(image);
  return sum / (double)(height * width * 3);
}


int main(void)
{
  int screen, screens;
  
  if (display = XOpenDisplay(NULL), display == NULL)
    return 1;
  
  screens = (size_t)ScreenCount(display);
  for (screen = 0; screen < screens; screen++)
    fprintf(stderr, "%lf\n", get_brightness(screen));
  
  XCloseDisplay(display);
  return 0;
}

