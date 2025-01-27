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


#include <cstdarg>
#include <cstdio> 

#include "Tools.h"
#include "Trace.h"
#include "TraceLog.h"
using namespace brathl;

namespace brathl
{

const int MAXLINE_TRACE = 1024;

  // Static initialization
CTrace* CTrace::m_instance = NULL;
int32_t	CTrace::m_TraceLevel = 0;

//----------------------------------------------------

CTrace::CTrace()
{
  m_fOut	= &cerr;
  m_bFileLog	= false;
}
//----------------------------------------------------

CTrace::~CTrace()
{
  // Set m_instance to NULL when the object is deleted.
  CTrace::m_instance = NULL;
}
//----------------------------------------------------


ostream* CTrace::GetDumpContextReel()
{
   return m_fOut;
}

//----------------------------------------------------

ostream* CTrace::SetDumpContextReal(ostream* value)
{

  ostream* old = m_fOut;

  m_fOut = value;

  return old;
}

//----------------------------------------------------

string CTrace::ParseArg(int argc, char *argv[])
{

   string szFileLog ="";

   vector<string> vectArgs(argc);

   // Remplissage du vecteur des arguments
   for (int i = 0; i < argc ; i++)
   {
      vectArgs[i] = argv[i];
   }


   // Lecture des arguments
   vector<string>::const_iterator it = vectArgs.begin();

   while ( it < vectArgs.end())
   {
      const string strArg = (*it);

      if ( strArg[0] == '-')
      {
         // Traitement option AUTOTEST
         if (strArg == "-trace")
         {
            it++;
            if ( it != vectArgs.end())
            {
               szFileLog = *it;
               break;
            }
         }
      }
      it++;
   }

   return szFileLog; 

}
//----------------------------------------------------

CTrace* CTrace::CreateObject(const string& szFileLog)
{
   if (szFileLog.empty() == true)
   {
      return new CTrace;
   }
   else
   {
      return new CTraceLog(szFileLog);
   }
}
//----------------------------------------------------

CTrace* CTrace::CreateObject(int argc, char *argv[])
{
   string szFileLog = CTrace::ParseArg(argc, argv);

   if (szFileLog.empty() == true)
   {
      return new CTrace;
   }
   else
   {
      return new CTraceLog(szFileLog);
   }
}

//----------------------------------------------------

CTrace* CTrace::GetInstance(const string& szFileLog)
 {
   if ( CTrace::m_instance == NULL)
   {
     CTrace::m_instance = CTrace::CreateObject(szFileLog);
     prepareSmartCleaner();
   }
   return CTrace::m_instance;
 }
//----------------------------------------------------

CTrace* CTrace::GetInstance(int argc, char *argv[])
 {
   if ( CTrace::m_instance == NULL)
   {
     CTrace::m_instance = CTrace::CreateObject(argc, argv);
     prepareSmartCleaner();
   }
   return CTrace::m_instance;
 }
//----------------------------------------------------

CTrace* CTrace::GetInstance()
 {
   if ( CTrace::m_instance == NULL)
   {
     CTrace::m_instance = new CTrace;
     prepareSmartCleaner();
   }
   return CTrace::m_instance;
 }
//----------------------------------------------------

void CTrace::prepareSmartCleaner(void)
{
  // make a single entry point for creating the smart cleaner because there are multiple
  // GetInstacne interfaces.
  static CSmartCleaner object(&m_instance);
}
//----------------------------------------------------

ostream* CTrace::SetDumpContext(ostream* value)
{
   CTrace * pTrace = GetInstance();
   return pTrace->SetDumpContextReal(value);
}
//----------------------------------------------------

ostream* CTrace::GetDumpContext()
{
   CTrace * pTrace = GetInstance();
   return pTrace->GetDumpContextReel();
}
//----------------------------------------------------

bool CTrace::IsTrace
		(int32_t	Level	/*= 5*/)
{
  if (Level < 0)
    return true;
  if (m_TraceLevel == 0)
    return false;
  return (Level <= m_TraceLevel) ||
	 (m_TraceLevel == 5);
}

//----------------------------------------------------
void CTrace::Tracer
		(const string	&message)
{
  if (! IsTrace(5))
    return;

  PrintText(true, message);
}
//----------------------------------------------------

void CTrace::Tracer
		(const char	*message,
		 ...)
{
  if (! IsTrace(5))
    return;

  va_list args;
  va_start( args, message );

  PrintText(true, message, args);

  va_end( args );

}


//----------------------------------------------------

void CTrace::Tracer
		(const int32_t	Level,
		 const char	*message,
		 ...)
{
  if (! IsTrace(Level))
    return;

  va_list args;
  va_start( args, message );

  PrintText(true, message, args);

  va_end( args );
}

//----------------------------------------------------


void CTrace::Tracer
		(const int32_t	Level,
		 const string	&message)
{
  if (! IsTrace(Level))
    return;

  PrintText(true, message);
}

//----------------------------------------------------

void CTrace::Print
		(const string	&message)
{
  if (! IsTrace(5))
    return;

  PrintText(false, message);
}
//----------------------------------------------------

void CTrace::Print
		(const char	*message,
		 ...)
{
  if (! IsTrace(5))
    return;

  va_list args;
  va_start( args, message );

  PrintText(false, message, args);

  va_end( args );

}


//----------------------------------------------------

void CTrace::Print
		(const int32_t	Level,
		 const char	*message,
		 ...)
{
  if (! IsTrace(Level))
    return;

  va_list args;
  va_start( args, message );

  PrintText(false, message, args);

  va_end( args );
}

//----------------------------------------------------


void CTrace::Print
		(const int32_t	Level,
		 const string	&message)
{
  if (! IsTrace(Level))
    return;

  PrintText(false, message);
}
//----------------------------------------------------


void CTrace::PrintText
		(bool		NewLine,
		 const char	*message,
		 va_list	args)
{
  ostream* fOut = CTrace::GetDumpContext();

  *fOut << CTools::Format(MAXLINE_TRACE, message, args);
  if (NewLine)
    *fOut << endl;
}
//----------------------------------------------------

void CTrace::PrintText
		(bool		NewLine,
		 const string	&message)
{
  ostream* fOut = CTrace::GetDumpContext();

  *fOut << message;
  if (NewLine)
    *fOut << endl;
}
//----------------------------------------------------

void CTrace::SetTraceLevel
		(int32_t	Level)
{
  if (Level < 0)
    m_TraceLevel	= 0;
  else if (Level > 5)
    m_TraceLevel	= 5;
  else
    m_TraceLevel	= Level;
}

//----------------------------------------------------

int32_t CTrace::GetTraceLevel
		()
{
  return m_TraceLevel;
}


}
