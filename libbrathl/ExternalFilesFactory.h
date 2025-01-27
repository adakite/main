/*
* 
*
* This file is part of BRAT
*
* BRAT is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* BRAT is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA	 02110-1301, USA.
*/
#ifndef _ExternalFilesFactory_h_
#define _ExternalFilesFactory_h_

#include <netcdf.h>
#include "Expression.h"
#include "Stl.h"

namespace brathl
{



/** \addtogroup tools Tools
  @{ */

/**
  External files access.



 \version 1.0
*/



/*
** Return NULL if file cannot be identified or an error occured
*/
CExternalFiles* BuildExistingExternalFileKind
		(const string		&Name);

/** @} */

}

#endif // Already included .h
