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

#include <cstdlib>
#include <cstdio>
#include <cstring> 
#include <typeinfo> 

#include "brathl_error.h" 
#include "brathl.h" 

#include "Stl.h" 

#include "TraceLog.h" 
#include "Tools.h" 
#include "Exception.h" 
#include "ProductRiverLake.h" 

using namespace brathl;

namespace brathl
{

CProductRiverLake::CProductRiverLake()
{
  Init();
}


//----------------------------------------

CProductRiverLake::CProductRiverLake(const string& fileName)
      : CProduct(fileName)
{
  Init();  
}


//----------------------------------------
CProductRiverLake::CProductRiverLake(const CStringList& fileNameList)
      : CProduct(fileNameList)

{
  Init();
}

//----------------------------------------

CProductRiverLake::~CProductRiverLake()
{
}

//----------------------------------------
void CProductRiverLake::Init()
{
  m_label = "River & Lake product";

  InitDateRef();
  InitCriteriaInfo();
}

//----------------------------------------
void CProductRiverLake::InitDateRef()
{
  m_refDate = REF20000101;
}

//----------------------------------------
void CProductRiverLake::InitCriteriaInfo()
{
  CProduct::InitCriteriaInfo();
}

//----------------------------------------
void CProductRiverLake::Dump(ostream& fOut /* = cerr */)
{
  if (CTrace::IsTrace() == false)
  {
    return;
  }


  fOut << "==> Dump a CProductRiverLake Object at "<< this << endl;

  //------------------
  CProduct::Dump(fOut);
  //------------------

  fOut << "==> END Dump a CProductRiverLake Object at "<< this << endl;

  fOut << endl;

}


} // end namespace
