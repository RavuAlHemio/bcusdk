/*
    EIBD eib bus access and management daemon
    Copyright (C) 2005 Martin Kögler <mkoegler@auto.tuwien.ac.at>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "loadimage.h"
#include "image.h"

BCU_LOAD_RESULT
PrepareLoadImage (const CArray & im, BCUImage * &img)
{
  img = 0;
  Image *i = Image::fromArray (im);
  if (!i)
    return IMG_UNRECOG_FORMAT;

  if (!i->isValid ())
    {
      delete i;
      return IMG_INVALID_FORMAT;
    }

  STR_BCUType *b = (STR_BCUType *) i->findStream (S_BCUType);
  if (!b)
    {
      delete i;
      return IMG_NO_BCUTYPE;
    }
  STR_Code *c = (STR_Code *) i->findStream (S_Code);
  if (!c)
    {
      delete i;
      return IMG_NO_CODE;
    }
  if (b->bcutype == 0x0012)
    {
      STR_BCU1Size *s = (STR_BCU1Size *) i->findStream (S_BCU1Size);
      if (!s)
	{
	  delete i;
	  return IMG_NO_SIZE;
	}

      if (s->datasize + s->bsssize + s->stacksize > 18)
	{
	  delete i;
	  return IMG_LODATA_OVERFLOW;
	}
      if (s->textsize > 0xfe)
	{
	  delete i;
	  return IMG_TEXT_OVERFLOW;
	}

      img = new BCUImage;
      img->code = c->code;
      img->BCUType = BCUImage::B_bcu1;
      return IMG_IMAGE_LOADABLE;
    }
  if (b->bcutype == 0x0020)
    {


    }
  delete i;
  return IMG_UNKNOWN_BCUTYPE;
}

#define _(A) (A)

String
decodeBCULoadResult (BCU_LOAD_RESULT r)
{
  switch (r)
    {
    case IMG_UNKNOWN_ERROR:
      return _("unknown error");
      break;
    case IMG_UNRECOG_FORMAT:
      return _("data not regcognized as image");
      break;
    case IMG_INVALID_FORMAT:
      return _("invalid streams in the image");
      break;
    case IMG_NO_BCUTYPE:
      return _("no bcu type specified");
      break;
    case IMG_UNKNOWN_BCUTYPE:
      return _("don't know how to load the bcutype");
      break;
    case IMG_NO_CODE:
      return _("no text segment found");
      break;
    case IMG_NO_SIZE:
      return _("size information not found");
      break;
    case IMG_LODATA_OVERFLOW:
      return _("too many data for low-ram");
      break;
    case IMG_HIDATA_OVERFLOW:
      return _("too many data for hi-ram");
      break;
    case IMG_TEXT_OVERFLOW:
      return _("to many data for eeprom");
      break;
    case IMG_IMAGE_LOADABLE:
      return _("Image is loadable");
      break;
    default:
      return _("errorcode not defined");
    }
}
