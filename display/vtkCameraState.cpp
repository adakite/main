/////////////////////////////////////////////////////////////////////////////
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
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "vtkCameraState.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif


#include "vtkObjectFactory.h"
#include "vtkCameraState.h"

//-------------------------------------------------------------
//------------------- vtkCameraState class --------------------
//-------------------------------------------------------------
vtkStandardNewMacro(vtkCameraState);

//----------------------------------------------------------------------------
vtkCameraState::vtkCameraState()
{

}
//----------------------------------------------------------------------------
vtkCameraState::~vtkCameraState()
{

}
//----------------------------------------------------------------------------
void vtkCameraState::PrintSelf(ostream& os, vtkIndent indent)
{
 
  os << indent << "ClippingRange: (" << this->ClippingRange[0] << ", " 
     << this->ClippingRange[1] << ")\n";
  os << indent << "FocalPoint: (" << this->FocalPoint[0] << ", " 
     << this->FocalPoint[1] << ", " << this->FocalPoint[2] << ")\n";
  os << indent << "ViewUp: (" << this->ViewUp[0] << ", " 
     << this->ViewUp[1] << ", " << this->ViewUp[2] << ")\n";
  os << indent << "Position: (" << this->Position[0] << ", " 
     << this->Position[1] << ", " << this->Position[2] << ")\n";
  os << indent << "ViewAngle: " << this->ViewAngle << "\n";
  os << indent << "ParallelScale: " << this->ParallelScale << "\n";
}



