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

#if !defined(_TraceLog_h_)
#define _TraceLog_h_


#include "Trace.h"
using namespace brathl;

namespace brathl
{

class CTraceLog : public CTrace  
{
public:
   CTraceLog(const string& szFilelog);
   virtual ~CTraceLog();

//Methods
public:

protected:

   void OpenLogFile();

   virtual ostream* GetDumpContextReel();
// Attributes
public:

protected:
   /*
   * Log file
   */
   ofstream *m_pFileLog ;  
   /** 
   * Log file name
   */
   string  m_szFileLogName ;       

};

}
#endif // !defined(_TraceLog_h_)
