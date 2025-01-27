
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
//#define BRAT_INTERNAL

#include "coda.h"

#include "Stl.h"

#include "TraceLog.h"
#include "Tools.h"
#include "Exception.h"


#include "Expression.h"

#include "ExternalFiles.h"
#include "ExternalFilesFactory.h"
#include "ExternalFilesATP.h"
#include "ExternalFilesJason2.h"

#include "Product.h"
#include "ProductAop.h"
#include "ProductCryosat.h"
#include "ProductEnvisat.h"
#include "ProductErs.h"
#include "ProductErsWAP.h"
#include "ProductGfo.h"
#include "ProductJason.h"
#include "ProductJason2.h"
#include "ProductNetCdf.h"
#include "ProductNetCdfCF.h"
#include "ProductPodaac.h"
#include "ProductRads.h"
#include "ProductRiverLake.h"
#include "ProductTopex.h"
#include "ProductTopexSDR.h"

// When debugging changes all calls to "new" to be calls to "DEBUG_NEW" allowing for memory leaks to
// give you the file name and line number where it occurred.
// Needs to be included after all #include commands
#include "Win32MemLeaksAccurate.h"

using namespace brathl;

namespace brathl
{


const char* CProduct::m_transposeFieldValuesFileName = "brathl_transposefieldvalues.txt";

const int32_t CProduct::NUMBER_OF_STATISTICS	= 5;
const uint32_t CProduct::COUNT_INDEX = 0;
const uint32_t CProduct::MEAN_INDEX = 1;
const uint32_t CProduct::STDDEV_INDEX = 2;
const uint32_t CProduct::MIN_INDEX = 3;
const uint32_t CProduct::MAX_INDEX = 4;


//-------------------------------------------------------------
//------------------- CProductList class --------------------
//-------------------------------------------------------------
CProductList::CProductList()
{

}


//----------------------------------------

CProductList::CProductList(const CProductList& lst)
{
  Set(lst);
}

//----------------------------------------

CProductList::CProductList(const string& fileName)
{

  this->Insert(fileName);

}


//----------------------------------------
CProductList::CProductList(const CStringList& fileNameList)
{

  this->Insert(fileNameList);

}

//----------------------------------------
CProductList::CProductList(const CStringArray& fileNameArray)
{

  this->Insert(fileNameArray);

}

//----------------------------------------

CProductList::~CProductList()
{

}


//----------------------------------------
bool CProductList::CheckFileList()
{
  if (this->empty() )
  {
    CProductException e("CProductList::CheckFiles - file list is empty()", BRATHL_COUNT_ERROR);
    CTrace::Tracer("%s", e.what());
    Dump(*CTrace::GetDumpContext());
    throw (e);
  }

  return true;
}

//----------------------------------------
const CProductList& CProductList::operator =(const CProductList& lst)
{
  Set(lst);

  return *this;

}

//----------------------------------------
void CProductList::Set(const CProductList& lst)
{

  CStringList::operator =(lst);

  m_productClass = lst.m_productClass;
  m_productType = lst.m_productType;
  m_productFormat = lst.m_productFormat;

}


//----------------------------------------
bool CProductList::IsHdfOrNetcdfCodaFormat(coda_format format)
{
  return ((format == coda_format_hdf4) || (format == coda_format_hdf5) || (format == coda_format_netcdf));
}

//----------------------------------------
bool CProductList::IsHdfOrNetcdfCodaFormat()
{
  return IsHdfOrNetcdfCodaFormat(m_productFormat);
}

//----------------------------------------
bool CProductList::IsJason2()
{
  if (IsNetCdfCFProduct() == false)
  {
    return false;
  }

  string str = CTools::StringToLower(m_productType).substr(0, CExternalFilesJason2::m_missionName.size());

  return (CTools::StringToUpper(str).compare(CExternalFilesJason2::m_missionName) == 0);

}

//----------------------------------------
bool CProductList::IsATP()
{
  if (IsNetCdfProduct() == false)
  {
    return false;
  }
  string str = CTools::StringToLower(m_productType).substr(0, CExternalFilesATP::m_ALONG_TRACK_PRODUCT.size());

  return (CTools::StringToUpper(str).compare(CExternalFilesATP::m_ALONG_TRACK_PRODUCT) == 0);

}

//----------------------------------------
bool CProductList::IsZFXY()
{
  return (CTools::StringToLower(m_productType).compare(CTools::StringToLower(ZFXY_NETCDF_TYPE)) == 0);
}
//----------------------------------------
bool CProductList::IsYFX()
{
  return (CTools::StringToLower(m_productType).compare(CTools::StringToLower(YFX_NETCDF_TYPE)) == 0);
}
//----------------------------------------
bool CProductList::IsGenericNetCdf()
{
  return (CTools::StringToLower(m_productType).compare(CTools::StringToLower(GENERIC_NETCDF_TYPE)) == 0);
}
//----------------------------------------
bool CProductList::IsSameProduct(const string& productClass, const string& productType)
{
  return ((m_productClass.compare(productClass) == 0) && (m_productType.compare(productType) == 0));
}
//----------------------------------------

bool CProductList::CheckFiles(bool onlyFirstFile /* = false */)
{
  int32_t result  = 0;

  m_message = "";

  const char* productClassRead = NULL;
  const char* productTypeRead = NULL;
  coda_format productFormatRead;
  int32_t productVersionRead = 0;
  int64_t fileSizeRead = 0;

  stringlist::iterator it;

  CheckFileList();

  for ( it = this->begin(); it != this->end(); it++ )
  {

    result = coda_recognize_file((*it).c_str(),
                                 &fileSizeRead,
                                 &productFormatRead,
                                 &productClassRead,
                                 &productTypeRead,
                                 &productVersionRead
                                 );
    if (result != 0)
    {
      string msg = CTools::Format("CProductList::CheckFiles - Error while checking file - errno:#%d:%s",
                                   coda_errno, coda_errno_to_string(coda_errno));
      CFileException e(msg, *it, BRATHL_IO_ERROR);
      CTrace::Tracer("%s", e.what());
      Dump(*CTrace::GetDumpContext());
      throw (e);
    }

    // Quick solution: mimic the old brat_recognize_file's
    // behaviour in the face of NetCDF files:
    switch( productFormatRead )
    {
        case coda_format_hdf4:
        {
            productClassRead = "HDF4";
            break;
        }
        case coda_format_hdf5:
        {
            productClassRead = "HDF5";
            break;
        }
        case coda_format_netcdf:
        {
            productClassRead = "NETCDF";
            break;
        }
    default:
        break;
    }

        // It is now still possible for coda_recognize to return
        // success, yet to not actually have recognized the file
        // format, leading to null pointers where we don't want
        // 'em. For now, we just test for those here, and abort
        // if that's the case.
    if (productClassRead == NULL)
    {
      string msg = CTools::Format("CProductList::CheckFiles - Unknown file format");
      CFileException e(msg, *it, BRATHL_IO_ERROR);
      CTrace::Tracer("%s", e.what());
      Dump(*CTrace::GetDumpContext());
      throw (e);
    }

    //BRATHL_INCONSISTENCY_ERROR

    bool bFileOk = true;

    if (it != this->begin())
    {
      bFileOk &= ((m_productClass.compare(productClassRead) != 0) ? false : true);
      if (! CProductList::IsHdfOrNetcdfCodaFormat(productFormatRead))
      {
        bFileOk &= ((m_productType.compare(productTypeRead) != 0) ? false : true);
      }
      else
      {
        bFileOk &= ((m_productType.compare("") != 0) ? false : true);
      }

      if (bFileOk == false)
      {
        string msg = CTools::Format("CProductList::CheckFiles - Files are not in the same way - Expected Product Class/Type: '%s/%s",
                                     m_productClass.c_str(), m_productType.c_str());
        CProductException e(msg,
                            *it,
                            productClassRead,
                            ((productTypeRead == NULL) ? "" : productTypeRead),
                            BRATHL_INCONSISTENCY_ERROR);
        CTrace::Tracer("%s", e.what());
        Dump(*CTrace::GetDumpContext());
        throw (e);

      }

    }

    m_productClass = productClassRead;
    m_productFormat = productFormatRead;
    m_productType = (productTypeRead != NULL ? productTypeRead : "");

    if (onlyFirstFile)
    {
      break;
    }

  }

  // if product is recognized as hdf4 by Brat, we consider it as a NetCdf product.
  if (IsHdfOrNetcdfCodaFormat())
  {
    return CheckFilesNetCdf();
  }

  return true;

}

//----------------------------------------
bool CProductList::CheckFilesNetCdf()
{
  m_productClass = NETCDF_PRODUCT_CLASS;

  string productTypeRead;

  stringlist::iterator it;
  CExternalFiles *f = NULL;

  for ( it = this->begin(); it != this->end(); it++ )
  {
    f = BuildExistingExternalFileKind((*it).c_str());
    if (f != NULL)
    {
      m_productClass = f->GetProductClass();
      productTypeRead = f->GetType();
    }
    //BRATHL_INCONSISTENCY_ERROR

    bool bFileOk = true;

    if (it != this->begin())
    {
      bFileOk &= ((m_productType.compare(productTypeRead) != 0) ? false : true);

      if (bFileOk == false)
      {
        string msg = CTools::Format("CProductList::CheckFilesNetCdf - Netcdf Files are not in the same way - Expected Product Class/Type: '%s/%s",
                                     m_productClass.c_str(), m_productType.c_str());
        CProductException e(msg,
                            *it, m_productClass, productTypeRead, BRATHL_INCONSISTENCY_ERROR);
        CTrace::Tracer("%s", e.what());
        Dump(*CTrace::GetDumpContext());

        if (f != NULL)
        {
          delete f;
          f = NULL;
        }

        throw (e);

      }

    }

    if (f != NULL)
    {
      delete f;
      f = NULL;
    }

    m_productType = productTypeRead;
  }

  if ( (IsYFX() || IsZFXY() || IsGenericNetCdf()) && this->size() > 1)
  {
    m_message = "Warning - You have to check that all the files in the list : "
                "\n1) are in the same way (same structure, same fields with same dimension...)"
                "\n2) contain the same kind of data"
                "\n\n otherwhise results may be ill-defined and confused or Brat may return a reading error.";
  }

  return true;
}
//----------------------------------------
void CProductList::Dump(ostream& fOut /* = cerr */)
{
  if (CTrace::IsTrace() == false)
  {
    return;
  }


  fOut << "==> Dump a CProductList Object at "<< this << endl;


  fOut << "m_productFormat = " << m_productFormat << endl;
  fOut << "m_productClass = " << m_productClass << endl;
  fOut << "m_productType = " << m_productType << endl;

  //------------------
  CStringList::Dump(fOut);
  //------------------


  fOut << "==> END Dump a CProductList Object at "<< this << endl;

  fOut << endl;

}


//-------------------------------------------------------------
//------------------- CProduct class --------------------
//-------------------------------------------------------------

coda_array_ordering  CProduct::m_arrayOrdering = coda_array_ordering_c;
//bool CProduct::m_codaReleaseWhenDestroy = true;

const string CProduct::m_treeRootName = "Root";

int32_t CProduct::m_codaRefCount = 0;


CProduct::CProduct()
    : m_listFields(false)
{

  Init();

}


//----------------------------------------

CProduct::CProduct(const string& fileName)
    : m_listFields(false)
{
  Init();

  AddFile(fileName);

  LoadAliases();

}


//----------------------------------------
CProduct::CProduct(const CStringList& fileNameList)
    : m_listFields(false)
{
  Init();

  AddFile(fileNameList);

  LoadAliases();

}


//----------------------------------------

CProduct::~CProduct()
{
  Release();
  DeleteLogFile();

}

/*
//----------------------------------------
string CProduct::GetHighResolutionLatDiffFieldName()
{
  CProductException e("ERROR - CProduct;;GetHighResolutionLatDiffFieldName should not be called", m_currFileName, GetProductClass(), GetProductType(), BRATHL_LOGIC_ERROR);
  throw(e);
}
//----------------------------------------
string CProduct::GetHighResolutionLonDiffFieldName()
{
  CProductException e("ERROR - CProduct;;GetHighResolutionLonDiffFieldName should not be called", m_currFileName, GetProductClass(), GetProductType(), BRATHL_LOGIC_ERROR);
  throw(e);
}
*/

//----------------------------------------
/*
void CProduct::InitDateRef()
{
  m_refDate = REF20000101;
}
*/


//----------------------------------------
void CProduct::AddCriteria(CCriteria* criteria, bool erase /*= true*/)
{

  if (erase)
  {
    m_criteriaMap.Erase(criteria->GetKey());
  }

  m_criteriaMap.Insert(criteria->GetKey(), criteria);

}

//----------------------------------------
void CProduct::RemoveCriteria()
{
  m_criteriaMap.RemoveAll();
}



//----------------------------------------
bool CProduct::GetValueMinMax(CExpression& expr, const string& recordName, double& valueMin, double& valueMax, const CUnit& unit)
{
  CTools::SetDefaultValue(valueMin);
  CTools::SetDefaultValue(valueMax);

  CStringList listFieldsToRead;
  listFieldsToRead.InsertUnique(expr.GetFieldNames());

  if (listFieldsToRead.empty())
  {
    return true;
  }

  m_indexProcessedFile = 0;
  CProductList::iterator itFile;
  
  double offset = 0;

  // Searches for each files
  for ( itFile = m_fileList.begin(); itFile != m_fileList.end(); itFile++ )
  {
    m_indexProcessedFile++;

    //this->Open(*itFile, recordName, listFieldsToRead);
    this->Open(*itFile, recordName, listFieldsToRead);

    // Get the number of record for the default record name (set in Open method of CProduct above)
    int32_t nRecords = this->GetNumberOfRecords();

    //CTrace::Tracer(1,"Reading record data from %s ... and writing output data...", (*itFile).c_str());

    for (int32_t iRecord = 0 ; iRecord < nRecords ; iRecord++)
    {
      //Read fields for the record name  (listof field and record name are set in Open method of CProduct above)
      this->ReadBratRecord(iRecord);

      CObArray::iterator itDataSet;
      uint32_t nbValues;

      CRecordSet* recordSet = NULL;
      for (itDataSet = m_dataSet.begin(); itDataSet != m_dataSet.end() ; itDataSet++)
      {
        recordSet = m_dataSet.GetRecordSet(itDataSet);

        CExpressionValue exprValue;
        recordSet->ExecuteExpression(expr, recordName, exprValue);

        nbValues	= exprValue.GetNbValues();
        if (nbValues == 0)
        {
          continue; // No Data
        }

        CUnit	wantedUnit = unit;
        wantedUnit.SetConversionFromBaseUnit();

        wantedUnit.ConvertVector(exprValue.GetValues(), exprValue.GetNbValues());

        CField::AdjustValidMinMax(exprValue.GetValues(), exprValue.GetNbValues(), valueMin, valueMax);


        if (IsNetCdf())
        {
          // Set the right brat index data offset
          // Because all values have been read at once
          // and the right offdet is 'nbValues'
          offset += nbValues;
          this->SetOffset(offset);
        }

      }

    }

    this->Close();
  }

  m_indexProcessedFile = -1;

  return true;

}
//----------------------------------------
bool CProduct::GetLatLonMinMax(double& latMin, double& lonMin, double& latMax, double& lonMax)
{
  CLatLonRect latlonRectMinMax;

  bool result =  GetLatLonMinMax(latlonRectMinMax);

  latMin = latlonRectMinMax.GetLatMin();
  lonMin = latlonRectMinMax.GetLonMin();
  latMax = latlonRectMinMax.GetLatMax();
  lonMax = latlonRectMinMax.GetLonMax();

  return result;

}
//----------------------------------------
bool CProduct::GetLatLonMinMax(CLatLonRect& latlonRectMinMax)
{
  latlonRectMinMax.SetDefaultValue();

  int32_t iRecord = 0;

  CRecordDataMap listRecord;

  if (!HasLatLonCriteriaInfo())
  {
    return false;
  }

  // Sets a list of fields to be read (fields of all criteria)
  // To optimize reading data, fields are organized by data record
  CCriteriaLatLonInfo* criteriaInfo = this->GetLatLonCriteriaInfo();
  if (criteriaInfo == NULL)
  {
    return false;
  }

  criteriaInfo->GetFields(listRecord);

  CProductList::iterator itFile;

  m_indexProcessedFile = 0;
  // Searches for each files
  for ( itFile = m_fileList.begin(); itFile != m_fileList.end(); itFile++ )
  {
    m_indexProcessedFile++;
    //CTrace::Tracer(1,"Process file %d of %d", (long)m_indexProcessedFile, (long)m_fileList.size());

    this->Open(*itFile);

    // For each record, read data fields
    CObMap::iterator itMapListRecord;
    for ( itMapListRecord = listRecord.begin(); itMapListRecord != listRecord.end(); itMapListRecord++ )
    {
      m_dataSetNameToRead = itMapListRecord->first;

      CObMap* fieldsInfo = listRecord.GetFields(itMapListRecord->first);

      CStringList listFieldToRead;

      fieldsInfo->GetKeys(listFieldToRead);

      InitInternalFieldName(listFieldToRead, false);

      Rewind();

      ReadBratRecord(iRecord);

      //m_dataSet.Dump();
//      DumpDictionary("dumpDict.txt");


      // If no data, it's an error
      CRecordSet* recordSet = m_dataSet.GetRecordSet(iRecord);
      if (recordSet == NULL)
      {

        string msg = CTools::Format("ERROR - CProduct::GetLatLonMinMax() - There is no data for record '%s' and fields '%s'. ",
                                    m_dataSetNameToRead.c_str(),
                                    listFieldToRead.ToString().c_str());
        CProductException e(msg, m_currFileName, GetProductClass(), GetProductType(), BRATHL_INCONSISTENCY_ERROR);
        CTrace::Tracer("%s", e.what());
        throw (e);
      }

      // Criteria data record does not correspond to read record   --> next criteria
      if (criteriaInfo->GetDataRecord().compare(m_dataSetNameToRead) != 0)
      {
        continue;
      }

      CStringList criteriaFieldNames;
      criteriaInfo->GetFieldNames(criteriaFieldNames);

      bool fieldsExists = true;

      // Tests if criteria fields correspond to read fields
      CStringList::iterator itCritFieldNames;
      for ( itCritFieldNames = criteriaFieldNames.begin(); itCritFieldNames != criteriaFieldNames.end(); itCritFieldNames++ )
      {
        CFieldSet *fieldSet = m_dataSet.GetFieldSet( m_fieldNameEquivalence.Exists(*itCritFieldNames) );
        if (fieldSet == NULL)
        {
          fieldsExists = false;
          break;
        }
      }

      // At least one fields does not correspond to --> next criteria
      if (!fieldsExists)
      {
        continue;
      }

      // Gets start latitude
      double lat;
      CTools::SetDefaultValue(lat);

      if (CTools::IsDefaultValue(m_forceLatMinCriteriaValue))
      {
        lat = m_dataSet.GetFieldSetAsDblValue(m_fieldNameEquivalence.Exists(criteriaInfo->GetStartLatFieldName()));
      }
      else
      {
        lat = m_forceLatMinCriteriaValue;
      }

      // Gets start longitude
      double lon = m_dataSet.GetFieldSetAsDblValue(m_fieldNameEquivalence.Exists(criteriaInfo->GetStartLonFieldName()));
      CLatLonPoint left(lat, lon);

      // Gets end latitude
      CTools::SetDefaultValue(lat);

      if (CTools::IsDefaultValue(m_forceLatMinCriteriaValue))
      {
        lat = m_dataSet.GetFieldSetAsDblValue(m_fieldNameEquivalence.Exists(criteriaInfo->GetEndLatFieldName()));
      }
      else
      {
        lat = m_forceLatMaxCriteriaValue;
      }

      // Gets end longitude
      lon = m_dataSet.GetFieldSetAsDblValue(m_fieldNameEquivalence.Exists(criteriaInfo->GetEndLonFieldName()));
      CLatLonPoint right(lat, lon);

      CLatLonRect latLonRect(left, right);

      latlonRectMinMax.Extend(latLonRect);

      string str = latlonRectMinMax.AsString();
    }

    this->Close();
  }

  m_indexProcessedFile = -1;

  return true;

}

//----------------------------------------
bool CProduct::GetDateMinMax(CDate& dateMin, CDate& dateMax)
{
  CDatePeriod datePeriodMinMax;

  bool result =  GetDateMinMax(datePeriodMinMax);

  dateMin = datePeriodMinMax.GetFrom();
  dateMax = datePeriodMinMax.GetTo();

  return result;
}
//----------------------------------------
bool CProduct::GetDateMinMax(CDatePeriod& datePeriodMinMax)
{
  datePeriodMinMax.SetDefaultValue();

  int32_t iRecord = 0;

  CRecordDataMap listRecord;

  if (!HasDatetimeCriteriaInfo())
  {
    return false;
  }

  // Sets a list of fields to be read (fields of all criteria)
  // To optimize reading data, fields are organized by data record
  CCriteriaDatetimeInfo * criteriaInfo = this->GetDatetimeCriteriaInfo();
  if (criteriaInfo == NULL)
  {
    return false;
  }

  criteriaInfo->GetFields(listRecord);

  CProductList::iterator itFile;

  m_indexProcessedFile = 0;
  // Searches for each files
  for ( itFile = m_fileList.begin(); itFile != m_fileList.end(); itFile++ )
  {
    m_indexProcessedFile++;
    //CTrace::Tracer(1,"Process file %d of %d", (long)m_indexProcessedFile, (long)m_fileList.size());

    this->Open(*itFile);

    // For each record, read data fields
    CObMap::iterator itMapListRecord;
    for ( itMapListRecord = listRecord.begin(); itMapListRecord != listRecord.end(); itMapListRecord++ )
    {
      m_dataSetNameToRead = itMapListRecord->first;

      CObMap* fieldsInfo = listRecord.GetFields(itMapListRecord->first);

      CStringList listFieldToRead;

      fieldsInfo->GetKeys(listFieldToRead);

      InitInternalFieldName(listFieldToRead, false);

      Rewind();

      ReadBratRecord(iRecord);

      //m_dataSet.Dump();
//      DumpDictionary("dumpDict.txt");


      // If no data, it's an error
      CRecordSet* recordSet = m_dataSet.GetRecordSet(iRecord);
      if (recordSet == NULL)
      {

        string msg = CTools::Format("ERROR - CProduct::GetDateMinMax() - There is no data for record '%s' and fields '%s'. ",
                                    m_dataSetNameToRead.c_str(),
                                    listFieldToRead.ToString().c_str());
        CProductException e(msg, m_currFileName, GetProductClass(), GetProductType(), BRATHL_INCONSISTENCY_ERROR);
        CTrace::Tracer("%s", e.what());
        throw (e);
      }

      // Criteria data record does not correspond to read record   --> next criteria
      if (criteriaInfo->GetDataRecord().compare(m_dataSetNameToRead) != 0)
      {
        continue;
      }

      CStringList criteriaFieldNames;
      criteriaInfo->GetFieldNames(criteriaFieldNames);

      bool fieldsExists = true;

      // Tests if criteria fields correspond to read fields
      CStringList::iterator itCritFieldNames;
      for ( itCritFieldNames = criteriaFieldNames.begin(); itCritFieldNames != criteriaFieldNames.end(); itCritFieldNames++ )
      {
        CFieldSet *fieldSet = m_dataSet.GetFieldSet( m_fieldNameEquivalence.Exists(*itCritFieldNames) );
        if (fieldSet == NULL)
        {
          fieldsExists = false;
          break;
        }
      }

      // At least one fields does not correspond to --> next criteria
      if (!fieldsExists)
      {
        continue;
      }



      // Gets start datetime
      //CDate startDate(m_dataSet.GetFieldSetAsDblValue(m_fieldNameEquivalence.Exists(criteriaInfo->GetStartDateFieldName())),
      //                criteriaInfo->GetRefDate() );

      CDate startDate(m_dataSet.GetFieldSetAsDblValue(m_fieldNameEquivalence.Exists(criteriaInfo->GetStartDateFieldName())));

      // Gets end datetime
      //CDate endDate(m_dataSet.GetFieldSetAsDblValue(m_fieldNameEquivalence.Exists(criteriaInfo->GetEndDateFieldName())),
      //              criteriaInfo->GetRefDate() );

      CDate endDate(m_dataSet.GetFieldSetAsDblValue(m_fieldNameEquivalence.Exists(criteriaInfo->GetEndDateFieldName())));


      datePeriodMinMax.Union(startDate, endDate);

      string str = datePeriodMinMax.AsString();
    }

    this->Close();
  }

  m_indexProcessedFile = -1;

  return true;

}
//----------------------------------------
void CProduct::AddCriteria(CProduct* product)
{
  RemoveCriteria();

  if (product == NULL)
  {
    return;
  }

  if (HasLatLonCriteriaInfo() && product->IsSetLatLonCriteria())
  {
   AddCriteria(new CCriteriaLatLon(product->GetLatLonCriteria()));
  }

  if (HasDatetimeCriteriaInfo() && product->IsSetDatetimeCriteria())
  {
   AddCriteria(new CCriteriaDatetime(product->GetDatetimeCriteria()));
  }

  if (HasPassStringCriteriaInfo() && product->IsSetPassStringCriteria())
  {
   AddCriteria(new CCriteriaPassString( product->GetPassStringCriteria()));
  }

  // Set always pass criteria as integer after pass citeria as string
  if (HasPassIntCriteriaInfo() && product->IsSetPassIntCriteria())
  {
   AddCriteria(new CCriteriaPassInt(product->GetPassIntCriteria()));
  }

  if (HasCycleCriteriaInfo()  && product->IsSetCycleCriteria())
  {
   AddCriteria(new CCriteriaCycle(product->GetCycleCriteria()));
  }

}
//----------------------------------------
void CProduct::AddCriteria(bool force /* = false */)
{

  RemoveCriteria();

  force &= HasCriteriaInfo();

  if (HasLatLonCriteriaInfo() || force)
  {
   AddCriteria(new CCriteriaLatLon());
  }

  if (HasDatetimeCriteriaInfo() || force)
  {
   AddCriteria(new CCriteriaDatetime());
  }

  if (HasPassStringCriteriaInfo() || force)
  {
   AddCriteria(new CCriteriaPassString());
  }

  // Set always pass criteria as integer after pass citeria as string
  if (HasPassIntCriteriaInfo() || force)
  {
   AddCriteria(new CCriteriaPassInt());
  }

  if (HasCycleCriteriaInfo() || force)
  {
   AddCriteria(new CCriteriaCycle());
  }

}
//----------------------------------------
void CProduct::LogSelectionResult(const string& fileName, bool result)
{
    Log("\t\t\t>>>>>>>>>>>>>>>>>>>> Result of selection <<<<<<<<<<<<<<<<<<<<");
    Log("==>File ", false);
    Log(fileName, false);

    if (result)
    {
      Log(" is candidate");
    }
    else
    {
      Log(" is rejected");
    }
    Log("\t\t\t===============================================================");


}
//----------------------------------------
void CProduct::ApplyCriteria(CStringList& filteredFileList, const string& logFileName /* = "" */)
{

  if (!logFileName.empty())
  {
    CreateLogFile(logFileName);
  }

  InitApplyCriteriaStats();

  int32_t iRecord = 0;

  // Initializes files list corresponding to criteria (all files)
  // then files that does not correspond to criteria will be removed from the list.
  filteredFileList.Insert(m_fileList);

  CRecordDataMap listRecord;

  // Sets a list of fields to be read (fields of all criteria)
  // To optimize reading data, fields are organized by data record
  BuildCriteriaFieldsToRead(listRecord);

  CProductList::iterator itFile;

  m_indexProcessedFile = 0;

  // Searches for each files if it corresponds to criteria
  for ( itFile = m_fileList.begin(); itFile != m_fileList.end(); itFile++ )
  {
    bool fileOk = true;

    m_indexProcessedFile++;
    CTrace::Tracer(1,"Process file %ld of %ld", (long)m_indexProcessedFile, (long)m_fileList.size());

    try
    {
      Log("---------------------------");
      Log("File ", false);
      Log(*itFile);
      Log("---------------------------");

      this->Open(*itFile);
    }
    catch (CException& e)
    {
      Log("Error while opening file:", false);
      Log(e.what());
      continue;
    }
    catch (...)
    {
      Log("Unknown error while opening file");
      continue;
    }

    // For each record, read data fields
    CObMap::iterator itMapListRecord;
    for ( itMapListRecord = listRecord.begin(); itMapListRecord != listRecord.end(); itMapListRecord++ )
    {
      try
      {
        m_dataSetNameToRead = itMapListRecord->first;

        CObMap* fieldsInfo = listRecord.GetFields(itMapListRecord->first);

        CStringList listFieldToRead;

        fieldsInfo->GetKeys(listFieldToRead);

        InitInternalFieldName(listFieldToRead, false);

        Rewind();

        ReadBratRecord(iRecord);

        //m_dataSet.Dump();
  //      DumpDictionary("dumpDict.txt");


        // If no data, it's an error
        CRecordSet* recordSet = m_dataSet.GetRecordSet(iRecord);
        if (recordSet == NULL)
        {

          string msg = CTools::Format("ERROR - CProduct::ApplyCriteria() - There is no data for record '%s' and fields '%s'. ",
                                      m_dataSetNameToRead.c_str(),
                                      listFieldToRead.ToString().c_str());
          CProductException e(msg, m_currFileName, GetProductClass(), GetProductType(), BRATHL_INCONSISTENCY_ERROR);
          CTrace::Tracer("%s", e.what());
          throw (e);
        }

        // Tests each criteria
        CObIntMap::iterator itMapCritInfo;
        for ( itMapCritInfo = m_criteriaInfoMap.begin(); itMapCritInfo != m_criteriaInfoMap.end(); itMapCritInfo++ )
        {

          CCriteriaInfo* criteriaInfo = CCriteriaInfo::GetCriteriaInfo(itMapCritInfo->second);
          // Criteria data record does not correspond to read record   --> next criteria
          if (criteriaInfo->GetDataRecord().compare(m_dataSetNameToRead) != 0)
          {
            continue;
          }

          CStringList criteriaFieldNames;
          criteriaInfo->GetFieldNames(criteriaFieldNames);

          bool fieldsExists = true;

          // Tests if criteria fields correspond to read fields
          CStringList::iterator itCritFieldNames;
          for ( itCritFieldNames = criteriaFieldNames.begin(); itCritFieldNames != criteriaFieldNames.end(); itCritFieldNames++ )
          {
            CFieldSet *fieldSet = m_dataSet.GetFieldSet( m_fieldNameEquivalence.Exists(*itCritFieldNames) );
            if (fieldSet == NULL)
            {
              fieldsExists = false;
              break;
            }
          }

          // At least one fields does not correspond to --> next criteria
          if (!fieldsExists)
          {
            continue;
          }


          // Tests Lat/Lon criteria
          bool latLonCriteriaOk = ApplyCriteriaLatLon(criteriaInfo);

          if (!latLonCriteriaOk)
          {
            filteredFileList.Erase(*itFile);
            fileOk = false;
            break;
          }
          // Tests Datetime criteria
          bool datetimeCriteriaOk = ApplyCriteriaDatetime(criteriaInfo);

          if (!datetimeCriteriaOk)
          {
            filteredFileList.Erase(*itFile);
            fileOk = false;
            break;
          }

          // Tests Pass criteria
          bool passCriteriaOk = ApplyCriteriaPass(criteriaInfo);

          if (!passCriteriaOk)
          {
            filteredFileList.Erase(*itFile);
            fileOk = false;
            break;
          }

          // Tests cycle criteria
          bool cycleCriteriaOk = ApplyCriteriaCycle(criteriaInfo);

          if (!cycleCriteriaOk)
          {
            filteredFileList.Erase(*itFile);
            fileOk = false;
            break;
          }

        }

        if (!fileOk)
        {
          // next file
          break;
        }

      }
      catch (CException& e)
      {
        Log("Error while processing file:", false);
        Log(e.what());
        fileOk = false;
        break;
      }
      catch (...)
      {
        Log("Unknown error while processing file");
        fileOk = false;
        break;
      }

    }

    LogSelectionResult(*itFile, fileOk);

    this->Close();
  }

  m_indexProcessedFile = -1;

  EndApplyCriteriaStats(filteredFileList);

  DeleteLogFile();

}

//----------------------------------------
CCriteria* CProduct::GetCriteria(CCriteriaInfo* criteriaInfo)
{
  if (criteriaInfo == NULL)
  {
    string msg = "ERROR - CProduct::GetCriteria() - Unknown criteria info is null";
    CProductException e(msg, m_currFileName, GetProductClass(), GetProductType(), BRATHL_INCONSISTENCY_ERROR);
    CTrace::Tracer("%s", e.what());
    throw (e);
  }


  CCriteria* criteria = CCriteria::GetCriteria(m_criteriaMap.Exists(criteriaInfo->GetKey()), false);
/*
  if (criteria == NULL)
  {
    string msg = CTools::Format("ERROR - CProduct::GetCriteria() - Unknown criteria key %d in criteria map",
                                criteriaInfo->GetKey());
    CProductException e(msg, m_currFileName, GetProductClass(), GetProductType(), BRATHL_INCONSISTENCY_ERROR);
    CTrace::Tracer(e.what());
    throw (e);

  }
*/
  return criteria;

}
//----------------------------------------
void CProduct::BuildCriteriaFieldsToRead(CRecordDataMap& listRecord)
{

  if (IsSetLatLonCriteria())
  {
    CCriteriaLatLonInfo * critInfo = this->GetLatLonCriteriaInfo();
    critInfo->GetFields(listRecord);
  }
  if (IsSetDatetimeCriteria())
  {
    CCriteriaDatetimeInfo * critInfo = this->GetDatetimeCriteriaInfo();
    critInfo->GetFields(listRecord);
  }
  if (IsSetPassIntCriteria())
  {
    CCriteriaPassIntInfo * critInfo = this->GetPassIntCriteriaInfo();
    critInfo->GetFields(listRecord);
  }
  if (IsSetPassStringCriteria())
  {
    CCriteriaPassStringInfo * critInfo = this->GetPassStringCriteriaInfo();
    critInfo->GetFields(listRecord);
  }
  if (IsSetCycleCriteria())
  {
    CCriteriaCycleInfo * critInfo = this->GetCycleCriteriaInfo();
    critInfo->GetFields(listRecord);
  }

}
//----------------------------------------
bool CProduct::ApplyCriteriaLatLon(CCriteriaInfo* criteriaInfo)
{

  if (criteriaInfo == NULL)
  {
    return true;
  }

  CCriteria* criteria = this->GetCriteria(criteriaInfo);
  if (criteria == NULL)
  {
    return true;
  }

  CCriteriaLatLon* criteriaLatLon = CCriteriaLatLon::GetCriteria(criteria, false);

  if (criteriaLatLon == NULL)
  {
    return true;
  }
  if (criteriaLatLon->IsDefaultValue())
  {
    return true;
  }

  CCriteriaLatLonInfo* criteriaLatLonInfo = CCriteriaLatLonInfo::GetCriteriaInfo(criteriaInfo);

  // Gets start latitude
  double lat = m_dataSet.GetFieldSetAsDblValue(m_fieldNameEquivalence.Exists(criteriaLatLonInfo->GetStartLatFieldName()));
  // Gets start longitude
  double lon = m_dataSet.GetFieldSetAsDblValue(m_fieldNameEquivalence.Exists(criteriaLatLonInfo->GetStartLonFieldName()));
  CLatLonPoint left(lat, lon);

  // Gets end latitude
  lat = m_dataSet.GetFieldSetAsDblValue(m_fieldNameEquivalence.Exists(criteriaLatLonInfo->GetEndLatFieldName()));
  // Gets end longitude
  lon = m_dataSet.GetFieldSetAsDblValue(m_fieldNameEquivalence.Exists(criteriaLatLonInfo->GetEndLonFieldName()));
  CLatLonPoint right(lat, lon);

  CLatLonRect latLonRect(left, right);
  CLatLonRect intersect;

  bool bOk = criteriaLatLon->Intersect(latLonRect, intersect);

  Log("Criteria Lat/Lon box: ", false);
  Log(criteriaLatLon->GetAsText());
  Log("File Lat/Lon box: ", false);
  Log(latLonRect.GetAsText(), false);
  Log(". Intersect: ", false);
  Log(bOk, (bOk ? false : true));
  if (bOk)
  {
    Log(". Intersection box ", false);
    Log(intersect.GetAsText());
  }

  return bOk;

}

//----------------------------------------
bool CProduct::ApplyCriteriaDatetime(CCriteriaInfo* criteriaInfo)
{
  if (criteriaInfo == NULL)
  {
    return true;
  }

  CCriteria* criteria = this->GetCriteria(criteriaInfo);
  if (criteria == NULL)
  {
    return true;
  }

  CCriteriaDatetime* criteriaDatetime = CCriteriaDatetime::GetCriteria(criteria, false);

  if (criteriaDatetime == NULL)
  {
    return true;
  }
  if (criteriaDatetime->IsDefaultValue())
  {
    return true;
  }

  CCriteriaDatetimeInfo* criteriaDatetimeInfo = CCriteriaDatetimeInfo::GetCriteriaInfo(criteriaInfo);

  // Gets start datetime
  //CDate startDate(m_dataSet.GetFieldSetAsDblValue(m_fieldNameEquivalence.Exists(criteriaDatetimeInfo->GetStartDateFieldName())),
  //                criteriaDatetimeInfo->GetRefDate() );

  CDate startDate(m_dataSet.GetFieldSetAsDblValue(m_fieldNameEquivalence.Exists(criteriaDatetimeInfo->GetStartDateFieldName())));

  // Gets end datetime
  //CDate endDate(m_dataSet.GetFieldSetAsDblValue(m_fieldNameEquivalence.Exists(criteriaDatetimeInfo->GetEndDateFieldName())),
  //              criteriaDatetimeInfo->GetRefDate() );

  CDate endDate(m_dataSet.GetFieldSetAsDblValue(m_fieldNameEquivalence.Exists(criteriaDatetimeInfo->GetEndDateFieldName())));

  CDatePeriod datePeriod(startDate, endDate);

  //string str = startDate.AsString();
  //str = endDate.AsString();
  CDatePeriod intersect;

  bool bOk =  criteriaDatetime->Intersect(datePeriod, intersect);

  Log("Criteria Date period: ", false);
  Log(criteriaDatetime->GetAsText());
  Log("File Date period: ", false);
  Log(datePeriod.GetAsText(), false);
  Log(". Intersect: ", false);
  Log(bOk, (bOk ? false : true));
  if (bOk)
  {
    Log(". Intersection box ", false);
    Log(intersect.GetAsText());
  }

  return bOk;
}
//----------------------------------------
bool CProduct::ApplyCriteriaPass(CCriteriaInfo* criteriaInfo)
{
  if (criteriaInfo == NULL)
  {
    return true;
  }

  CCriteria* criteria = this->GetCriteria(criteriaInfo);
  if (criteria == NULL)
  {
    return true;
  }

  CCriteriaPassInt* criteriaPassInt = CCriteriaPassInt::GetCriteria(criteria, false);

  if (criteriaPassInt != NULL)
  {
    return ApplyCriteriaPassInt(criteriaInfo);
  }

  CCriteriaPassString* criteriaPassString = CCriteriaPassString::GetCriteria(criteria, false);

  if (criteriaPassString != NULL)
  {
    return ApplyCriteriaPassString(criteriaInfo);
  }

  return true;
}

//----------------------------------------
bool CProduct::ApplyCriteriaPassInt(CCriteriaInfo* criteriaInfo)
{
  if (criteriaInfo == NULL)
  {
    return true;
  }

  CCriteria* criteria = this->GetCriteria(criteriaInfo);
  if (criteria == NULL)
  {
    return true;
  }

  CCriteriaPassInt* criteriaPass = CCriteriaPassInt::GetCriteria(criteria, false);

  if (criteriaPass == NULL)
  {
    return true;
  }
  if (criteriaPass->IsDefaultValue())
  {
    return true;
  }

  CCriteriaPassIntInfo* criteriaPassInfo = CCriteriaPassIntInfo::GetCriteriaInfo(criteriaInfo);

  // Gets start and end pass
  double startPassValue = m_dataSet.GetFieldSetAsDblValue(m_fieldNameEquivalence.Exists(criteriaPassInfo->GetStartPassFieldName()));
  double endPassValue = startPassValue;

  if (!criteriaPassInfo->GetEndPassFieldName().empty())
  {
    endPassValue = m_dataSet.GetFieldSetAsDblValue(m_fieldNameEquivalence.Exists(criteriaPassInfo->GetEndPassFieldName()));
  }


  CIntArray intersect;

  bool bOk = criteriaPass->Intersect(startPassValue, endPassValue, intersect);

  Log("Criteria Pass: ", false);
  Log(criteriaPass->GetAsText());
  Log("File Pass from: ", false);
  Log(startPassValue, false);
  Log(" Pass end: ", false);
  Log(endPassValue, false);
  Log(". Intersect: ", false);
  Log(bOk, (bOk ? false : true));
  if (bOk)
  {
    Log(". Intersection box ", false);
    Log(intersect.ToString());
  }

  return bOk;

}
//----------------------------------------
bool CProduct::ApplyCriteriaPassString(CCriteriaInfo* criteriaInfo)
{
  if (criteriaInfo == NULL)
  {
    return true;
  }

  CCriteria* criteria = this->GetCriteria(criteriaInfo);
  if (criteria == NULL)
  {
    return true;
  }

  CCriteriaPassString* criteriaPass = CCriteriaPassString::GetCriteria(criteria, false);

  if (criteriaPass == NULL)
  {
    return true;
  }
  if (criteriaPass->IsDefaultValue())
  {
    return true;
  }

  CCriteriaPassStringInfo* criteriaPassInfo = CCriteriaPassStringInfo::GetCriteriaInfo(criteriaInfo);

  // Gets start and end pass
  CStringArray passes;

  string startPassValue = m_dataSet.GetFieldSetAsStringValue(m_fieldNameEquivalence.Exists(criteriaPassInfo->GetStartPassFieldName()));
  passes.Insert(startPassValue);

  if (!criteriaPassInfo->GetEndPassFieldName().empty())
  {
    string endPassValue = m_dataSet.GetFieldSetAsStringValue(m_fieldNameEquivalence.Exists(criteriaPassInfo->GetEndPassFieldName()));
    passes.Insert(endPassValue);
  }

  Log("Passes: ", false);
  Log(passes.ToString());

  CStringArray intersect;

  bool bOk = criteriaPass->Intersect(passes, intersect);

  Log("Criteria Passes: ", false);
  Log(criteriaPass->GetAsText());
  Log("File Passes: ", false);
  Log(passes.ToString(), false);
  Log(". Intersect: ", false);
  Log(bOk, (bOk ? false : true));
  if (bOk)
  {
    Log(". Intersection box ", false);
    Log(intersect.ToString());
  }

  return bOk;

}
//----------------------------------------
bool CProduct::ApplyCriteriaCycle(CCriteriaInfo* criteriaInfo)
{
  if (criteriaInfo == NULL)
  {
    return true;
  }

  CCriteria* criteria = this->GetCriteria(criteriaInfo);
  if (criteria == NULL)
  {
    return true;
  }

  CCriteriaCycle* criteriaCycle = CCriteriaCycle::GetCriteria(criteria, false);

  if (criteriaCycle == NULL)
  {
    return true;
  }
  if (criteriaCycle->IsDefaultValue())
  {
    return true;
  }

  CCriteriaCycleInfo* criteriaCycleInfo = CCriteriaCycleInfo::GetCriteriaInfo(criteriaInfo);

  // Gets start and end cycle
  double startCycleValue = m_dataSet.GetFieldSetAsDblValue(m_fieldNameEquivalence.Exists(criteriaCycleInfo->GetStartCycleFieldName()));
  double endCycleValue = startCycleValue;


  if (!criteriaCycleInfo->GetEndCycleFieldName().empty())
  {
    endCycleValue = m_dataSet.GetFieldSetAsDblValue(m_fieldNameEquivalence.Exists(criteriaCycleInfo->GetEndCycleFieldName()));
  }

  CIntArray intersect;

  bool bOk = criteriaCycle->Intersect(startCycleValue, endCycleValue, intersect);

  Log("Criteria Cycle: ", false);
  Log(criteriaCycle->GetAsText());
  Log("File Cycle from: ", false);
  Log(startCycleValue, false);
  Log(" Cycle end: ", false);
  Log(endCycleValue, false);
  Log(". Intersect: ", false);
  Log(bOk, (bOk ? false : true));
  if (bOk)
  {
    Log(". Intersection box ", false);
    Log(intersect.ToString());
  }

  return bOk;

}

//----------------------------------------

void CProduct::AddFile(const string& fileName, bool bEnd /*= true*/, bool checkFiles /*= true*/)
{
  m_fileList.Insert(fileName, bEnd);
  if (checkFiles)
  {
    m_fileList.CheckFiles();
  }

}
//----------------------------------------

void CProduct::AddFile(const CStringList& fileNameList, bool bEnd /*= true*/, bool checkFiles /*= true*/)
{
  m_fileList.Insert(fileNameList, bEnd);
  if (checkFiles)
  {
    m_fileList.CheckFiles();
  }

}
//----------------------------------------
void CProduct::SetProductList(const string& fileName, bool checkFiles /*= true*/)
{
  m_fileList.clear();
  AddFile(fileName, true, checkFiles);

  LoadAliases();

}
//----------------------------------------
void CProduct::SetProductList(const CStringList& fileList, bool checkFiles /*= true*/)
{
  m_fileList.clear();
  AddFile(fileList, true, checkFiles);

  if (!IsSameProduct(m_fileList)) 
  {
    LoadAliases();
  }

}

//----------------------------------------
bool CProduct::IsSameProduct(const CProductList fileList)
{
  return IsSameProduct(fileList.m_productClass, fileList.m_productType);

}
//----------------------------------------
bool CProduct::IsSameProduct(const string& productClass, const string& productType)
{
  return m_fileList.IsSameProduct(productClass, productType);

}

//----------------------------------------

CProduct* CProduct::Construct(CStringArray& fileNameArray)
{
  CProductList productList(fileNameArray);

  return CProduct::Construct(productList);

}
//----------------------------------------

CProduct* CProduct::Construct(CStringList& fileNameList)
{
  CProductList productList(fileNameList);

  return CProduct::Construct(productList);

}

//----------------------------------------
CProduct* CProduct::Construct(const string& fileName)
{
  CProductList productList(fileName);

  return CProduct::Construct(productList);

}


//----------------------------------------

CProduct* CProduct::Construct(CProductList& fileNameList)
{
  CProduct* product = NULL;

  CProduct::CodaInit();

  try
  {
    fileNameList.CheckFiles();

    string productClass = fileNameList.m_productClass;
    string productType = fileNameList.m_productType;

    if (productClass.compare("Altimeter_Ocean_Pathfinder") == 0)
    {
      //product = new CProductAop(fileNameList);
      string msg = CTools::Format("ERROR - CProduct::Construct : UNIMPLEMENTED PRODUCT :'%s'",
                                  productClass.c_str());
      CUnImplementException e(msg, BRATHL_UNIMPLEMENT_ERROR);
      throw(e);
    }
    else if (productClass.compare("CRYOSAT") == 0)
    {
      product = new CProductCryosat(fileNameList);
    }
    else if (productClass.compare("ERS_RA") == 0)
    {
      if (productType.compare(CProductErs::m_WAP) == 0)
      {
        product = new CProductErsWAP(fileNameList);
      }
      else
      {
        product = new CProductErs(fileNameList);
      }
    }
    else if (productClass.compare("ENVISAT_RA2MWR") == 0)
    {
      product = new CProductEnvisat(fileNameList);
    }
    else if (productClass.compare("GFO") == 0)
    {
      product = new CProductGfo(fileNameList);
    }
    else if (productClass.compare("JASON") == 0)
    {
      product = new CProductJason(fileNameList);
    }
    else if (productClass.compare("PODAAC") == 0)
    {
      product = new CProductPodaac(fileNameList);
    }
    else if (productClass.compare("RADS") == 0)
    {
      //product = new CProductRads(fileNameList);
      string msg = CTools::Format("ERROR - CProduct::Construct : UNIMPLEMENTED PRODUCT :'%s'",
                                  productClass.c_str());
      CUnImplementException e(msg, BRATHL_UNIMPLEMENT_ERROR);
      throw(e);
    }
    else if (productClass.compare("River_Lake") == 0)
    {
        product = new CProductRiverLake(fileNameList);
    }
    else if (productClass.compare("Topex_Poseidon") == 0)
    {
      if (productType.compare(CProductTopexSDR::m_SDR_PASS_FILE) == 0)
      {
        product = new CProductTopexSDR(fileNameList);
      }
      else
      {
        product = new CProductTopex(fileNameList);
      }
    }
    else if (productClass.compare(NETCDF_PRODUCT_CLASS) == 0)
    {
      product = new CProductNetCdf(fileNameList);
    }
    else if (productClass.compare(NETCDF_CF_PRODUCT_CLASS) == 0)
    {
      if (fileNameList.IsJason2())
      {
        product = new CProductJason2(fileNameList);
      }
      else
      {
        product = new CProductNetCdfCF(fileNameList);
      }

    }
    else
    {
      product = new CProductGeneric(fileNameList);
      /*
      string msg = CTools::Format("CProduct::Construct - Unknown product found:%s in file %s",
                                   fileNameList.m_productClass.c_str(),
                                   fileNameList.front().c_str());
      CProductException e(msg, BRATHL_INCONSISTENCY_ERROR);
      CTrace::Tracer("%s", e.what());
      throw (e);
      */
    }
  }
  catch (CException& e)
  {
    CProduct::CodaRelease();
    throw (e);
  }

  CProduct::CodaRelease();
  return product;

}

//----------------------------------------

bool CProduct::CheckFiles()
{

  return m_fileList.CheckFiles();

}


//----------------------------------------
bool CProduct::Open(const string& fileName)
{
  Close();

  m_currFileName = fileName;

  return Open();
}

//----------------------------------------
void CProduct::RewindInit()
{
  m_nbRecords = -1;
  m_traceProcessRecordRatio = 1;

}
//----------------------------------------
void CProduct::RewindProcess()
{
}
//----------------------------------------
void CProduct::RewindEnd()
{
  //m_countForTrace = static_cast<uint32_t>(CTools::Round(GetNumberOfRecords() / 4));
  m_countForTrace = static_cast<uint32_t>(CTools::Round(GetNumberOfRecords() / 4));
  if (m_countForTrace <= 0)
  {
    m_countForTrace = 1;
  }
}
//----------------------------------------
void CProduct::Rewind()
{
  RewindInit();

  RewindProcess();

  RewindEnd();
}
//----------------------------------------
void CProduct::SetListFieldToRead(CStringList& listFieldToRead, bool convertDate /*= false*/)
{
  InitInternalFieldName(listFieldToRead, convertDate);

  Rewind();
}

//----------------------------------------
bool CProduct::Open(const string& fileName, const string& dataSetName)
{
  m_dataSetNameToRead = dataSetName;

  bool bOk = Open(fileName);

  return bOk;
}
//----------------------------------------
bool CProduct::Open(const string& fileName, const string& dataSetName, CStringList& listFieldToRead,  bool convertDate /*= false */)
{
  bool bOk = Open(fileName, dataSetName);

  SetListFieldToRead(listFieldToRead, convertDate);

  return bOk;
}
//----------------------------------------
bool CProduct::Open()
{
  Close();

  InitBratOptions();

  if (m_currFileName.empty())
  {
    CFileException e("CProduct::Open - Error opening file -  file name is empty", BRATHL_IO_ERROR);
    CTrace::Tracer("%s", e.what());
    Dump(*CTrace::GetDumpContext());
    throw (e);
  }

  // If the file to open is the first in the list or if the file is not in the list
  // reset offset
  int32_t indexFile = m_fileList.FindIndex(m_currFileName);
  if (indexFile <= 0)
  {
    SetOffset(0.0);
  }


  int32_t result = coda_open(m_currFileName.c_str(), &m_currFile);
  if (result != 0)
  {
    string msg = CTools::Format("CProduct::Open - Error while opening file - errno:#%d:%s", coda_errno, coda_errno_to_string(coda_errno));
    CFileException e(msg, m_currFileName, BRATHL_IO_ERROR);
    CTrace::Tracer("%s", e.what());
    Dump(*CTrace::GetDumpContext());
    throw (e);
  }

  // Loads data dictionary
  LoadFieldsInfo();

  FillDescription();

  return true;
}

//----------------------------------------

bool CProduct::Close()
{

  if (m_currFile == NULL)
  {
    return true;
  }

  int32_t result = coda_close(m_currFile);
  if (result != 0)
  {
    string msg = CTools::Format("CProduct::Close - Error while closing file - errno:#%d:%s", coda_errno, coda_errno_to_string(coda_errno));
    CFileException e(msg, m_currFileName, BRATHL_IO_ERROR);
    CTrace::Tracer("%s", e.what());
    Dump(*CTrace::GetDumpContext());
    throw (e);
  }

  m_currFile = NULL;
  m_currFileName = "";

  m_currentRecord = -1;

  return true;

}
//----------------------------------------
bool CProduct::IsOpened()
{
  return (m_currFile == NULL) ? false : true;
}
//----------------------------------------
bool CProduct::IsOpened(const string& fileName)
{
  bool bOpen = IsOpened();

  if (!bOpen)
  {
    return bOpen;
  }

  if (!CTools::CompareNoCase(m_currFileName, fileName))
  {
    bOpen = false;
  }

  return bOpen;
}

//----------------------------------------
void CProduct::CheckFileOpened()
{
  if (m_currFileName.empty())
  {
    CFileException e("CProduct::CheckFileOpened - No current file is setting for the product", BRATHL_LOGIC_ERROR);
    CTrace::Tracer("%s", e.what());
    Dump(*CTrace::GetDumpContext());
    throw (e);

  }

  if (IsOpened() == false)
  {
    CFileException e("CProduct::CheckFileOpened - File is not opened", m_currFileName, BRATHL_IO_ERROR);
    CTrace::Tracer("%s", e.what());
    Dump(*CTrace::GetDumpContext());
    throw (e);

  }
}
/*
//----------------------------------------
CField* CProduct::GetFieldRead(const string& fieldName)
{
  return FindFieldByName(fieldName, false);
}
*/
//----------------------------------------
bool CProduct::HasEqualDims(const string& value, string& msg)
{

  return HasEqualDims(value, m_dataSetNameToRead, msg);
}
//----------------------------------------
bool CProduct::HasEqualDims(const string& value, const string& dataSetName, string& msg)
{
  CExpression expr;

  bool bOk = CExpression::SetExpression(value, expr, msg);

  if (!bOk)
  {
    return bOk;
  }

  return HasEqualDims(expr, dataSetName, msg);
}
//----------------------------------------
bool CProduct::HasEqualDims(const CExpression& expr, string& msg)
{
  return HasEqualDims(expr.GetFieldNames(), m_dataSetNameToRead, msg);
}
//----------------------------------------
bool CProduct::HasEqualDims(const CExpression& expr, const string& dataSetName, string& msg)
{
  return HasEqualDims(expr.GetFieldNames(), dataSetName, msg);
}
//----------------------------------------
bool CProduct::HasEqualDims(const CStringArray* fieldNames, string& msg)
{
  return HasEqualDims(fieldNames, m_dataSetNameToRead, msg);
}
//----------------------------------------
bool CProduct::HasEqualDims(const CStringArray* fieldNames, const string& dataSetName,  string& msg)
{

  bool bOk = true;

  CStringArray::const_iterator it;

  CField* firstField = NULL;

  for (it = fieldNames->begin() ; it != fieldNames->end() ; it++)
  {
    CField *field = FindFieldByName(*it, dataSetName, false);
    if (field == NULL)
    {
      msg.append(CTools::Format("Field '%s' not found (record is '%s')\n",
                                it->c_str(),
                                dataSetName.c_str()));
      bOk &= false;
      continue;
    }

    if (firstField == NULL)
    {
      firstField = field;
      continue;
    }

    if (!firstField->HasEqualDims(field))
    {
      bOk &= false;
      msg.append(CTools::Format("Dimensions of fields '%s' and '%s'  are different: '%s' and '%s'\n",
                                firstField->GetName().c_str(),
                                field->GetName().c_str(),
                                firstField->GetDimAsString().c_str(),
                                field->GetDimAsString().c_str())
                                );

    }
  }

  return bOk;
}



//----------------------------------------
bool CProduct::HasCompatibleDims(const string& value, string& msg, bool useVirtualDims,
                                 CUIntArray* commonDimensions)
{

  return HasCompatibleDims(value, m_dataSetNameToRead, msg, useVirtualDims, commonDimensions);
}
//----------------------------------------
bool CProduct::HasCompatibleDims(const string& value, const string& dataSetName, string& msg, bool useVirtualDims,
                                 CUIntArray* commonDimensions)
{
  CExpression expr;

  bool bOk = CExpression::SetExpression(value, expr, msg);

  if (!bOk)
  {
    return bOk;
  }

  return HasCompatibleDims(expr, dataSetName, msg, useVirtualDims, commonDimensions);
}
//----------------------------------------
bool CProduct::HasCompatibleDims(const CExpression& expr, string& msg, bool useVirtualDims,
                                 CUIntArray* commonDimensions)
{
  return HasCompatibleDims(expr.GetFieldNames(), m_dataSetNameToRead, msg, useVirtualDims, commonDimensions);
}
//----------------------------------------
bool CProduct::HasCompatibleDims(const CExpression& expr, const string& dataSetName, string& msg, bool useVirtualDims,
                                 CUIntArray* commonDimensions)
{
  return HasCompatibleDims(expr.GetFieldNames(), dataSetName, msg, useVirtualDims, commonDimensions);
}
//----------------------------------------
bool CProduct::HasCompatibleDims(const CStringArray* fieldNames, string& msg, bool useVirtualDims,
                                 CUIntArray* commonDimensions)
{
  return HasCompatibleDims(fieldNames, m_dataSetNameToRead, msg, useVirtualDims, commonDimensions);
}
//----------------------------------------
bool CProduct::HasCompatibleDims(const CStringArray* fieldNames, const string& dataSetName, string& msg, bool useVirtualDims,
                                 CUIntArray* commonDimensions)
{

  bool bOk = true;

  int32_t maxNbDims = 0;
  int32_t nbDims = 0;

  int32_t compareTo = (useVirtualDims ? 0 : 1);

  CField* firstField = NULL;

  bool allFieldsAreHighResolution = true;

  CStringArray::const_iterator it;
  for (it = fieldNames->begin() ; it != fieldNames->end() ; it++)
  {
    CField *field = FindFieldByName(*it, dataSetName, false);
    if (field == NULL)
    {
      msg.append(CTools::Format("Field '%s' not found (record is '%s')\n",
                                it->c_str(),
                                dataSetName.c_str()));
      bOk &= false;
      continue;
    }

    if (useVirtualDims)
    {
      // GetVirtualNbDims ignore first non-fixed dim or first high resolution
      // dim for non-netcdf files
      nbDims = field->GetVirtualNbDims();
    }
    else
    {
      nbDims = field->GetNbDims();
    }

    if (nbDims <= compareTo)
    {
      continue;
    }

    // Notice thet netcdf fields are never declare as High resoution field
    // Only non-netcdf fields are.
    allFieldsAreHighResolution &= field->IsHighResolution();

    maxNbDims = (maxNbDims < nbDims ? nbDims : maxNbDims);

    if (firstField == NULL)
    {
      firstField = field;
      continue;
    }


    if (maxNbDims != nbDims)
    {
      bOk &= false;
      msg.append(CTools::Format("Dimensions of fields '%s' and '%s'  are not compatible: '%s' and '%s'\n",
                                firstField->GetName().c_str(),
                                field->GetName().c_str(),
                                firstField->GetDimAsString().c_str(),
                                field->GetDimAsString().c_str())
                                );
      continue;
    }


    for (int32_t i = compareTo ; i < maxNbDims ; i++)
    {
      if (firstField->GetDim()[i] != field->GetDim()[i])
      {
        bOk &= false;
        msg.append(CTools::Format("Dimensions of fields '%s' and '%s'  are different: '%s' and '%s'\n",
                                  firstField->GetName().c_str(),
                                  field->GetName().c_str(),
                                  firstField->GetDimAsString().c_str(),
                                  field->GetDimAsString().c_str())
                                  );
      }

    }

  }


  if ((commonDimensions != NULL) && (firstField != NULL))
  {
    int32_t offset = (allFieldsAreHighResolution ? firstField->GetOffsetDim() : 0);

    for (int32_t i = compareTo + offset ; i < maxNbDims + offset ; i++)
    {
      commonDimensions->Insert(firstField->GetDim()[i]);
    }

  }


  return bOk;

}

//----------------------------------------
bool CProduct::AddRecordNameToField(const CExpression& expr, const string& dataSetName, CExpression& exprOut, string& errorMsg)
{


  string exprStr;
  try
  {
    exprStr = exprOut.AsString().c_str();
  }
  catch (CException& e)
  {
    errorMsg = e.what();
    return false;

  }

  string exprStrOut;

  bool bOk = AddRecordNameToField(exprStr, dataSetName, exprStrOut, errorMsg);

  try
  {
    exprOut = exprStrOut;
  }
  catch (CException& e)
  {
    // Do nothing
    errorMsg = e.what();
    bOk = false;
  }

  return bOk;

}

//----------------------------------------
bool CProduct::AddRecordNameToField(const string& in, const string& dataSetName, string& out, string& errorMsg)
{

  CTools::ReplaceAliases(in, out);

  CExpression expr;

  try
  {
    expr.SetExpression(out);
  }
  catch (CException& e)
  {
    errorMsg = e.what();
    return false;
  }

  const CStringArray* fields = expr.GetFieldNames();

  return AddRecordNameToField(in, dataSetName, *fields, out, errorMsg);



}

//----------------------------------------
bool CProduct::AddRecordNameToField(const string& in, const string& dataSetName, const CStringArray& fieldsIn, string& out, string& errorMsg)
{
  bool bOk = true;

  CStringArray::const_iterator it;

  CStringList datasetNames;

  out = in;

  GetDataDictionaryFieldNames();

  for (it = fieldsIn.begin() ; it != fieldsIn.end() ; it++)
  {
    string msg;
    CField *field = FindFieldByName(*it, dataSetName, false, &msg);
    if (field != NULL)
    {
      // if field's record name is the same than the default record (dataSetName)
      // then don't add record name and replace by field name without record
      if (field->GetRecordName().compare(dataSetName) != 0)
      {
        out = CTools::ReplaceWord(out, *it, field->GetFullNameWithRecord());
      }
      else
      {
        out = CTools::ReplaceWord(out, *it, field->GetFullName());
      }
    }
    else
    {
      errorMsg.append(msg);
      bOk = false;
    }
  }

  return bOk;

}
//----------------------------------------
bool CProduct::CheckFieldNames(const CExpression& expr, CStringArray& fieldNamesNotFound)
{
  return CheckFieldNames(expr, m_dataSetNameToRead, fieldNamesNotFound);
}
//----------------------------------------
bool CProduct::CheckFieldNames(const CExpression& expr, const string& dataSetName, CStringArray& fieldNamesNotFound)
{

  return CheckFieldNames(expr.GetFieldNames(), dataSetName, fieldNamesNotFound);
}
//----------------------------------------
bool CProduct::CheckFieldNames(const CStringArray* fieldNames, const string& dataSetName, CStringArray& fieldNamesNotFound)
{
  bool bOk = true;

  CStringArray::const_iterator it;

  for (it = fieldNames->begin() ; it != fieldNames->end() ; it++)
  {
    CField *field = FindFieldByName(*it, dataSetName, false);
    if (field == NULL)
    {
      bOk &= false;
      fieldNamesNotFound.Insert(*it);
    }
  }

  return bOk;
}
/*
//----------------------------------------
bool CProduct::CheckFieldNames(const CStringArray* fieldNames, CStringArray& fieldNamesNotFound)
{
  return CheckFieldNames(fieldNames, m_dataSetNameToRead, fieldNamesNotFound);
}
*/
//----------------------------------------
CField* CProduct::FindFieldByName(const string& fieldName, const string& dataSetName, bool withExcept /*= true*/, string* errorMsg /*= NULL*/, bool showTrace /*=true*/)
{
  // field can be already coded as an internal name (i.e. record.name or datasetname.field)

  string internalFieldName = MakeInternalNameByAddingRoot(fieldName);

  CField* field = FindFieldByInternalName( internalFieldName, false);
  if (field != NULL)
  {
    return field;
  }

  // if field is not found as internal name, search it with the dataSetName passed as input parameter
  field = FindFieldByInternalName( MakeInternalFieldName(dataSetName, fieldName), false);

  if (field != NULL)
  {
    return field;
  }


  // if field is not found as internal name, search it within all dataset names in the product

  string errorString;
  string fieldNameOut = fieldName;


  GetDataDictionaryFieldNames();

  CIntArray indexes;
  m_dataDictionaryFieldNames.FindIndexes(fieldNameOut, indexes, true);

  if (indexes.size() <= 0)
  {
    string msg;
    if (field == NULL)
    {
      msg = CTools::Format("Field '%s' not found.",
                                  fieldName.c_str());
    }

    if (errorMsg != NULL)
    {
      *errorMsg = msg;
    }

    if (withExcept)
    {
      throw CProductException(msg, m_currFileName, GetProductClass(), GetProductType(), BRATHL_SYNTAX_ERROR);
    }
    return field;
  }

//      m_dataDictionaryFieldNames.Dump(*CTrace::GetDumpContext());
//      m_dataDictionaryFieldNamesWithDatasetName.Dump(*CTrace::GetDumpContext());
//      indexes.Dump(*CTrace::GetDumpContext());

  if (indexes.size() == 1)
  {
    fieldNameOut = CTools::ReplaceWord(fieldName, fieldName, m_dataDictionaryFieldNamesWithDatasetName.at(indexes.at(0)));

    field = FindFieldByName(fieldNameOut, dataSetName, withExcept);

  }
  else if (showTrace)
  {
    string possibleFieldNames;
    m_dataDictionaryFieldNamesWithDatasetName.GetValues(indexes, possibleFieldNames);

    string msg = CTools::Format("Ambiguous field '%s' in the expression."
                  " You should resolve ambiguity by changing it to one of the following names:\n%s\n",
      fieldNameOut.c_str(), possibleFieldNames.c_str());
    CTrace::Tracer(1,"\n>>>>>>>>>>>>>>>>>>>> WARNING >>>>>>>>>>>>>>>>>>>>>>>>>\n");
    CTrace::Tracer(1,"%s", msg.c_str());
    CTrace::Tracer(1,">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

    if (errorMsg != NULL)
    {
      *errorMsg = msg;
    }
  }

  return field;


}
//----------------------------------------
CField* CProduct::FindFieldByName(const string& fieldName, bool withExcept /*= true*/, string* errorMsg /*= NULL*/ , bool showTrace /*= true*/)
{
  //return FindFieldByInternalName( MakeInternalFieldName(fieldName), withExcept);
  return FindFieldByName(fieldName, m_dataSetNameToRead, withExcept, errorMsg, showTrace);

}
//----------------------------------------
CField* CProduct::FindFieldByInternalName(const string& internalFieldName, bool withExcept /*= true*/)
{
  CField* field = dynamic_cast<CField*>(m_tree.FindObject(internalFieldName));
  if (withExcept)
  {
    if (field == NULL)
    {
      CException e(CTools::Format("CProduct::FindFieldByInternalName - dynamic_cast<CField*>(ob) returns NULL "
                                  "object seems not to be an instance of CField or fieldname %s is not found",
                                   internalFieldName.c_str()),
                   BRATHL_LOGIC_ERROR);
      throw (e);
    }
  }

  return field;

}
//----------------------------------------
void CProduct::CheckFields(bool convertDate /*= false*/)
{
  m_listFieldExpandArray.RemoveAll();

  m_hasHighResolutionFieldToProcess = false;
  if (m_dataSetNameToRead.empty())
  {
    return;
  }

  CStringList::iterator itField;

  // Check the list of fields to read.
  for (itField = m_listInternalFieldName.begin() ; itField != m_listInternalFieldName.end() ; itField++)
  {
    CField* field =  FindFieldByInternalName((*itField), false);
    if (field == NULL)
    {
      string msg = CTools::Format("ERROR in CProduct::CheckFields - Field (Key field) '%s' not found",
                                  (*itField).c_str());
      CProductException e(msg, m_currFileName, GetProductClass(), GetProductType(), BRATHL_INCONSISTENCY_ERROR);
      throw (e);
    }

    // Field is in the user list, set hidden flag to false (by default, all fields has hidden flag to true
    field->SetHidden(false);

    field->SetConvertDate(convertDate);

    //Check if field is a 'high resolution' field
    if (field->IsHighResolution())
    {
      m_hasHighResolutionFieldToProcess = true;
    }

    // array processing : transform each array element to a field record
    if (m_expandArray)
    {
      m_listFieldExpandArray.Insert(field->GetName());
    }
    else
    {
      // array processing for specific fields : transform each array element to a field record
      if (field->IsExpandArray())
      {
        m_listFieldExpandArray.Insert(field->GetName());
      }
    }
  }

  if (m_hasHighResolutionFieldToProcess)
  {
    // add to the list, fields which are mandatory to compute high resolution
    // if these fields are hidden and will be not set in output, unless thy are in user list fields.
    // fields are added just once, even if there are several hihg resolution fields (insert unique).
    AddInternalHighResolutionFieldCalculation();
  }

}
//----------------------------------------
void CProduct::CodaInit()
{

  if (CProduct::m_codaRefCount <= 0)
  {
    coda_set_definition_path(CTools::GetDataDir().c_str());
    coda_init();
  }

  CProduct::m_codaRefCount++;

}
//----------------------------------------
void CProduct::CodaRelease()
{
  if (CProduct::m_codaRefCount > 0)
  {
    CProduct::m_codaRefCount--;
    if (CProduct::m_codaRefCount <= 0)
    {
      coda_done();
    }
  }

}

//----------------------------------------
void CProduct::SetPerformBoundaryChecks(bool performBoundaryChecks)
{
  m_performBoundaryChecks = performBoundaryChecks;
  coda_set_option_perform_boundary_checks(m_performBoundaryChecks);
  // brat_set_option_perform_boundary_checks doesn't set brat_errno, so don't call
  // HandleBratError because you will handle the last error found in brat, and that's irrelevant
  //HandleBratError("brat_set_option_perform_boundary_checks");
}

//----------------------------------------
void CProduct::SetPerformConversions(bool performConversions)
{
  m_performConversions = performConversions;
  coda_set_option_perform_conversions(m_performConversions);
  // brat_set_option_perform_boundary_checks doesn't set brat_errno, so don't call
  // HandleBratError because you will handle the last error found in brat, and that's irrelevant
  //HandleBratError("brat_set_option_perform_conversions");
}

//----------------------------------------
void CProduct::Init()
{
  m_productAliases = NULL;

  m_fieldsHaveDefaultValue = true;

  LoadTransposeFieldsValue(m_fieldsToTranspose);

  CTools::SetDefaultValue(m_forceLatMinCriteriaValue);
  CTools::SetDefaultValue(m_forceLatMaxCriteriaValue);

  m_indexProcessedFile = -1;

  m_logFile = NULL;

  m_label = "Unknown";

  coda_errno = 0;

  m_expandArray = false;
  m_createVirtualField = false;

  m_currFile = NULL;
  m_currFileName = "";
  m_description = "";

  m_recordCount = 0;

  m_nbRecords = -1;
  m_currentRecord = -1;


  //m_bratReleaseWhenDestroy = true;

  CProduct::CodaInit();

  SetPerformBoundaryChecks(false);
  SetPerformConversions(true);

  m_hasHighResolutionFieldToProcess = false;

  CTools::SetDefaultValue(m_deltaTimeHighResolution);
  CTools::SetDefaultValue(m_numHighResolutionMeasure);
  CTools::SetDefaultValue(m_refPoint);

  CTools::SetDefaultValue(m_previousLatitude);
  CTools::SetDefaultValue(m_previousLongitude);

  m_previousTimeStamp = 0.0;

  m_nSkippedRecord = 0;

  m_traceProcessRecordRatio = 1;

  m_countForTrace = 1;

  m_disableTrace = false;
  
  m_offset = 0.0;

  InitCriteriaInfo();

}
//----------------------------------------
void CProduct::GroupAliases(const CProduct* product, const CStringMap* formulaAliases, CStringMap& allAliases)
{
  if (product != NULL)
  {
    if (product->GetAliasesAsString() != NULL)
    {
      allAliases.Insert(*(product->GetAliasesAsString()), false);
    }
  }

  if (formulaAliases != NULL)
  {
    allAliases.Insert(*formulaAliases, false);
  }

}
//----------------------------------------
const CStringMap* CProduct::GetAliasesAsString(const CProduct* product)
{
  if (product == NULL)
  {
    return NULL;
  }

  return product->GetAliasesAsString();
}
//----------------------------------------
void CProduct::LoadAliases()
{
  CAliasesDictionary* aliasesDictionary = NULL;
  try
  {
    aliasesDictionary = CAliasesDictionary::GetInstance();
  }
  catch (CException& e)
  {
    throw CLoadAliasesException(CTools::Format("The aliases dictionary can't be loaded properly - Native Error is '%s'", e.what()), BRATHL_LOGIC_ERROR);
  }

  CXmlNode* node = aliasesDictionary->FindProductNode(this->GetProductClass(), false);

  if (node == NULL)
  {
    return;
  }
  
  // ---------------------
  // Gets default records 
  // ---------------------
  CObArray defaultRecordsArray;
  aliasesDictionary->GetDefaultRecords(node, defaultRecordsArray);

  //CObArray::const_iterator it;

  //for(it = defaultRecordsArray.begin() ; it != defaultRecordsArray.end() ; it++)
  //{
  //  CDefaultRecord* ob = dynamic_cast<CDefaultRecord*>(*it);
  //  ob->Dump(*(CTrace::GetInstance()->GetDumpContext()));
  //}
  //
  
  // ---------------------
  // Gets aliases
  // ---------------------
  CObArray aliasesArray;
  aliasesDictionary->GetAliases(node, aliasesArray);

  //for(it = aliasesArray.begin() ; it != aliasesArray.end() ; it++)
  //{
  //  CAliases* ob = dynamic_cast<CAliases*>(*it);
  //  ob->Dump(*(CTrace::GetInstance()->GetDumpContext()));
  //}
  
  // ---------------------
  // Maps default records by product type
  // ---------------------

  CDefaultRecord* defaultRecord = NULL;
  CObArray::const_iterator it;
  
  CObMap recordByProductType(false);
  CObMap::const_iterator itMap;
  
  for (it = defaultRecordsArray.begin() ; it != defaultRecordsArray.end() ; it++)
  {
    CDefaultRecord* defaultRecord = dynamic_cast<CDefaultRecord*>(*it);
    if (defaultRecord == NULL)
    {
      continue;
    }
    
    //defaultRecord->Dump(*(CTrace::GetInstance()->GetDumpContext()));

    defaultRecord->GetRecordByProductType(recordByProductType);
  }
  
  //for(itMap = recordByProductType.begin() ; itMap != recordByProductType.end() ; itMap++)
  //{
  //  string key = itMap->first;

  //  CDefaultRecord* defaultRecord = dynamic_cast<CDefaultRecord*>(itMap->second);
  //  if (defaultRecord == NULL)
  //  {
  //    continue;
  //  }
  //  
  //  cout << "ProductType: " << key << " - defaultRecord: " << defaultRecord->GetName() << endl;
  //}

  // ---------------------
  // Maps aliases by product type
  // ---------------------

  CObMap aliasesByProductType(false);

  for (it = aliasesArray.begin() ; it != aliasesArray.end() ; it++)
  {
    CAliases* aliases = dynamic_cast<CAliases*>(*it);
    if (aliases == NULL)
    {
      continue;
    }
    
    //aliases->Dump(*(CTrace::GetInstance()->GetDumpContext()));

    aliases->ResolveSynonyms();

    aliases->GetAliasesByProductType(aliasesByProductType);
  }

  //for(itMap = aliasesByProductType.begin() ; itMap != aliasesByProductType.end() ; itMap++)
  //{
  //  string key = itMap->first;

  //  CAliases* aliases = dynamic_cast<CAliases*>(itMap->second);
  //  if (aliases == NULL)
  //  {
  //    continue;
  //  }
  //  
  //  cout << "ProductType: '" << key << "'" <<  endl;

  // 
  //}


  // ---------------------
  // Maps aliases according to the product 
  // ---------------------
  string record;
  // First, get the default record whatever the product type
  defaultRecord = dynamic_cast<CDefaultRecord*>(recordByProductType.Exists(CAliases::m_ALL));
  if (defaultRecord != NULL)
  {
    record = defaultRecord->GetName();
  }

  // Get the default record for the current product type
  defaultRecord = dynamic_cast<CDefaultRecord*>(recordByProductType.Exists(GetProductType()));
  if (defaultRecord != NULL)
  {
    record = defaultRecord->GetName();
  }

  DeleteProductAliases();

  // If record is empty: perhaps there is a some missing data in definition 
  // But this may not be a mistake, then don't raise an exception, just log a message in the cerr.
  // If product is Netcdf: there is no record.
  if (record.empty())
  {
    if (!this->IsNetCdfOrNetCdfCFProduct())
    {
      string msg = CTools::Format("The aliases dictionary has some missing data - There is no record defined for product class/type %s/%s ", 
                                                  this->GetProductClass().c_str(),
                                                  this->GetProductType().c_str());
      cerr << "WARNING: " <<  msg << endl;
    }
    //throw CLoadAliasesException(CTools::Format("The aliases dictionary has some errors or missing data - There is no record defined for product class/type %s/%s ", 
    //                                            this->GetProductClass().c_str(),
    //                                            this->GetProductType().c_str()),
    //                            BRATHL_LOGIC_ERROR);
  }
  
  m_productAliases = new CProductAliases(record);

  // First, add aliases that are valid whatever the product type
  CAliases* aliasesToAdd = dynamic_cast<CAliases*>(aliasesByProductType.Exists(CAliases::m_ALL));
  if (aliasesToAdd != NULL)
  {
    m_productAliases->AddAlias(aliasesToAdd);
  }

  // Add/overwrite aliases that are valid for the current product type
  aliasesToAdd = dynamic_cast<CAliases*>(aliasesByProductType.Exists(GetProductType()));
  if (aliasesToAdd != NULL)
  {
    CAliases* aliasesRef = CAliasesDictionary::ResolveRef(aliasesToAdd, &aliasesArray);
    m_productAliases->AddAlias(aliasesRef);
    m_productAliases->AddAlias(aliasesToAdd);
  }

  string errorMsg;


  // Add record name to the fields contained in the alias value
  // Opening a file is needed to check the field name (field name with record name)
  // However, it's not needed if file is a netcdf file).
  if (!this->IsNetCdfOrNetCdfCFProduct())
  {
    if (m_fileList.size() > 0)
    {
      Open(*(m_fileList.begin()));
    }
  }

  bool bOk = m_productAliases->AddRecordNameToField(m_mapStringAliases, this, errorMsg);
  if (!bOk) 
  {
    throw CException(CTools::Format("Error in CProduct::LoadAliases() - Unable to add record name to fields:  %s", errorMsg.c_str()), BRATHL_SYNTAX_ERROR);
  }

  if (!this->IsNetCdfOrNetCdfCFProduct())
  {
    Close();
  }
  //m_productAliases->Dump(*(CTrace::GetInstance()->GetDumpContext()));


}

//----------------------------------------
const CProductAlias* CProduct::GetAlias(const string& key)
{
  if (m_productAliases == NULL) 
  {
    return NULL;
  }
  
  return dynamic_cast<CProductAlias*>(m_productAliases->Exists(key));

}
//----------------------------------------
void CProduct::GetAliasKeys(CStringArray& keys)
{
  m_mapStringAliases.GetKeys(keys);

}
//----------------------------------------
string CProduct::GetAliasExpandedValue(const string& key)
{
  return m_mapStringAliases.Exists(key);
}

//----------------------------------------
bool CProduct::CheckAliases(const string& fileName, CStringArray& errors)
{
  CProduct* product = NULL;
  bool bOk = true;

  try
  {
    product = CProduct::Construct(fileName);
    if (product == NULL)
    {
      string msg = "Unable to process: product is NULL";
      errors.Insert(msg);
      return false;
    }

    string msg = CTools::Format("Checking aliases for product class/type %s/%s",  
                                product->GetProductClass().c_str(),
                                product->GetProductType().c_str());
    errors.Insert(msg);

    bOk = product->CheckAliases(errors);
    if (bOk)
    {
      string msg = "No error has been found";
      errors.Insert(msg);
    }
  }
  catch(CException& e)
  {
    errors.Insert(e.what());
    bOk = false;
  }

  if (product != NULL)
  {
    delete product;
    product = NULL;
  }

  return bOk;
}

//----------------------------------------
bool CProduct::CheckAliases(CStringArray& errors)
{
  Open(*(m_fileList.begin()));

  CStringMap::const_iterator itMap;
  bool noError = true;

  for (itMap = m_mapStringAliases.begin() ; itMap != m_mapStringAliases.end() ; itMap++)
  {
    CStringArray fieldNames;
    string errorString;
    bool bOk = CExpression::GetFieldNames(itMap->second, fieldNames, errorString);
    noError &= bOk;
    if (!bOk)
    {
      errors.Insert(errorString);
      continue;
    }


    CStringArray::const_iterator itArray;
    for (itArray = fieldNames.begin(); itArray != fieldNames.end() ; itArray++)
    {
      errorString.clear();
      CField* field = FindFieldByName(*itArray, "", false, &errorString , false);
      if (field == NULL)
      {
        string nativeError;
        if (!errorString.empty())
        {
          nativeError = CTools::Format("(Native error: %s).", errorString.c_str());
        }

        noError = false;
        string msg = CTools::Format("Product class '%s' - Product type '%s' - Alias '%s': Field '%s' not found %s.",
                                    this->GetProductClass().c_str(),
                                    this->GetProductType().c_str(),
                                    itMap->first.c_str(),
                                    itArray->c_str(),
                                    nativeError.c_str());

        errors.Insert(msg);
      }
    }
  }
  
  Close();

  return noError;
}
//----------------------------------------
void CProduct::DeleteLogFile()
{
  if (m_logFile != NULL)
  {
    m_logFile->Flush();
    delete m_logFile;
    m_logFile = NULL;
  }
}
//----------------------------------------
void CProduct::CreateLogFile(const string& logFileName, uint32_t mode /* = CFile::modeWrite|CFile::typeText */)
{
  DeleteLogFile();
  try
  {
    m_logFile = new CFile(logFileName, mode);
  }
  catch (CException e)
  {
    DeleteLogFile();
  }
  catch (...)
  {
    DeleteLogFile();
  }

}
//----------------------------------------
void CProduct::Log(bool n, bool bCrLf /*= true*/)
{
  string str = (n ? "yes" : "no");
  Log(str, bCrLf);
}
//----------------------------------------
void CProduct::Log(double n, bool bCrLf /*= true*/)
{
  Log(CTools::DoubleToStr(n), bCrLf);
}
//----------------------------------------
void CProduct::Log(int32_t n, bool bCrLf /*= true*/)
{
  Log(CTools::IntToStr(n), bCrLf);
}
//----------------------------------------
void CProduct::Log(const string& str, bool bCrLf /*= true*/)
{
  Log(str.c_str(), bCrLf);
}
//----------------------------------------
void CProduct::Log(const CStringList& l, bool bCrLf /*= true*/)
{
  CStringList::const_iterator it;
  for (it = l.begin() ; it != l.end() ; it++)
  {
    Log(it->c_str(), bCrLf);
  }
}
//----------------------------------------
void CProduct::Log(const char* str, bool bCrLf /*= true*/)
{

  if (m_logFile == NULL)
  {
    return;
  }

  if (!m_logFile->IsOpen())
  {
    return;
  }

  try
  {
    m_logFile->Write(str);
    if (bCrLf)
    {
      m_logFile->Write('\n');
    }
  }
  catch (CException& e)
  {
    e.what(); // to avoid compiler complaint
  }
  catch (...)
  {
  }

}
//----------------------------------------
void CProduct::InitApplyCriteriaStats()
{
  m_dateProcessBegin.SetDateNow();
  Log("\t\t====================================");
  Log("\t\t\tTABLE OF CONTENTS");
  Log("\t\t====================================");
  Log("");
  Log("1 - SELECTION DETAILS");
  Log("2 - SUMMARY/STATISTIC");
  Log("3 - SELECTED FILES");
  Log("4 - REJECTED FILES");
  Log("");
  Log("====================================");
  Log("1 - DETAILS: ");
  Log("====================================");
  Log("");

}
//----------------------------------------
void CProduct::EndApplyCriteriaStats(const CStringList& filteredFileList)
{
  m_dateProcessEnd.SetDateNow();

  int32_t countIn = m_fileList.size();
  int32_t countOut = filteredFileList.size();

  Log("====================================");
  Log("2 - SUMMARY/STATISTIC:");
  Log("====================================");
  Log("");
  Log("Process date: ", false);
  Log(m_dateProcessEnd.AsString());
  Log("Number of files processed: ", false);
  Log(countIn, false);
  Log(". Number of files selected: ", false);
  Log(countOut, false);
  Log(". Number of files rejected: ", false);
  Log((countIn - countOut));
  Log("Elapsed time: ", false);
  Log((m_dateProcessEnd - m_dateProcessBegin), false);
  Log(" seconds (", false);
  Log(((m_dateProcessEnd - m_dateProcessBegin) / 60.0), false);
  Log(" minutes)");
  Log("");

  Log("====================================");
  Log("3 - SELECTED FILES:");
  Log("====================================");
  Log("");
  Log(filteredFileList);
  Log("");

  Log("====================================");
  Log("4 - REJECTED FILES:");
  Log("====================================");
  Log("");

  CStringList rejectedfiles;
  filteredFileList.Complement(m_fileList, rejectedfiles);

  Log(rejectedfiles);
  Log("");

  Log("\t\t====================================");
  Log("\t\t\tEND OF REPORT");
  Log("\t\t====================================");

}
//----------------------------------------
void CProduct::InitCriteriaInfo()
{
  m_criteriaInfoMap.RemoveAll();
}
//----------------------------------------
void CProduct::DeleteProductAliases()
{

  if (m_productAliases != NULL)
  {
    delete m_productAliases;
    m_productAliases = NULL;
  }

}
//----------------------------------------
void CProduct::Release()
{
  Close();

  m_currFile = NULL;
  m_currFileName = "";
  
  DeleteProductAliases();

  //if (m_codaReleaseWhenDestroy)
  //{
  //  CodaRelease();
  //}

  CodaRelease();

}


//----------------------------------------

void CProduct::HandleBratError(const string& str /* = ""*/, int32_t errClass /* = BRATHL_LOGIC_ERROR*/)
{
  if (coda_errno == 0)
  {
    return;
  }

  string msg = CTools::Format("BRAT ERROR - %s - errno:#%d:%s", str.c_str(), coda_errno, coda_errno_to_string (coda_errno));
  CProductException e(msg, m_currFileName, GetProductClass(), GetProductType(), errClass);
  CTrace::Tracer("%s", e.what());
  Dump(*CTrace::GetDumpContext());
  throw (e);

}


//----------------------------------------
void CProduct::GetRootType()
{
  int32_t result = coda_cursor_set_product(&m_cursor, m_currFile);
  HandleBratError("coda_cursor_set_product");

  result = coda_get_product_root_type(m_currFile, &(m_listInfo.Back()->m_type));
  HandleBratError("coda_get_product_root_type");
}


//----------------------------------------
void CProduct::InitBratOptions()
{
    coda_set_option_perform_boundary_checks(GetPerformBoundaryChecks());
    coda_set_option_perform_conversions(GetPerformConversions());
}

//----------------------------------------
void CProduct::ReplaceNamesCaseSensitive(const CExpression& exprIn, const CStringArray& fieldsIn, CExpression& exprOut, bool forceReload /*= false*/)
{
  string in = exprIn.AsString();
  string out;

  ReplaceNamesCaseSensitive(in, fieldsIn, out, forceReload);

  exprOut.SetExpression(out);

}
//----------------------------------------
void CProduct::ReplaceNamesCaseSensitive(const string& in, const CStringArray& fieldsIn, string& out, bool forceReload /*= false*/)
{
  CStringArray fieldsOutNoCaseSensitive;
  CStringArray fieldsOutCaseSensitive;

  GetNamesCaseSensitive(fieldsIn, fieldsOutNoCaseSensitive, fieldsOutCaseSensitive, forceReload);

  out = CTools::ReplaceWord(in, fieldsOutNoCaseSensitive, fieldsOutCaseSensitive);

}

//----------------------------------------
void CProduct::ReplaceNamesCaseSensitive(const CExpression& exprIn, string& out, bool forceReload /*= false*/)
{
  string in = exprIn.AsString();

  ReplaceNamesCaseSensitive(in, out, forceReload);

}
//----------------------------------------
void CProduct::ReplaceNamesCaseSensitive(const string& in, string& out, bool forceReload /*= false*/)
{
  //CStringArray aliasesFound;

  //CTools::FindAliases(in.c_str(), aliasesFound);
  ////CTrace::GetInstance()->SetTraceLevel(5);
  ////aliasesFound.Dump();

  //out = in;

  //CStringArray::const_iterator it;

  //for (it = aliasesFound.begin() ; it != aliasesFound.end() ; it++)
  //{
  //  string pattern = *it;
  //  pattern = CTools::Replace(pattern, "\\%\\{", "\\%\\{");
  //  pattern = CTools::Replace(pattern, "\\}", "\\}");
  //  out = CTools::Replace(out, pattern, "1");
  //}

  CTools::ReplaceAliases(in, out);

  CExpression expr;

  try
  {
    expr.SetExpression(out);
  }
  catch (CException& e)
  {
    // Do nothing
    e.what(); // to avoid compilation warning

  }

  const CStringArray* fields = expr.GetFieldNames();

  ReplaceNamesCaseSensitive(in, *fields, out, forceReload);

}

//----------------------------------------
void CProduct::GetNamesCaseSensitive(const CStringArray& fieldsIn, CStringArray& fieldsOutNoCaseSensitive, CStringArray& fieldsOutCaseSensitive, bool forceReload /*= false*/)
{
  GetDataDictionaryFieldNames(forceReload);

  CStringArray::const_iterator it;

  for (it = fieldsIn.begin() ; it != fieldsIn.end() ; it++)
  {
    int32_t index = m_dataDictionaryFieldNames.FindIndex(*it, true);
    if (index >= 0)
    {
      fieldsOutNoCaseSensitive.Insert(*it);
      fieldsOutCaseSensitive.Insert(m_dataDictionaryFieldNames.at(index));
    }
    else
    {
      index = m_dataDictionaryFieldNamesWithDatasetName.FindIndex(*it, true);
      if (index >= 0)
      {
        fieldsOutNoCaseSensitive.Insert(*it);
        fieldsOutCaseSensitive.Insert(m_dataDictionaryFieldNamesWithDatasetName.at(index));
      }
    }

  }

}

//----------------------------------------
CStringArray* CProduct::GetDataDictionaryFieldNamesWithDatasetName(bool forceReload /*= false*/)
{
  GetDataDictionaryFieldNames(forceReload);

  return &m_dataDictionaryFieldNamesWithDatasetName;

}

//----------------------------------------
CStringArray* CProduct::GetDataDictionaryFieldNames(bool forceReload /*= false*/)
{

  if (forceReload)
  {
    m_dataDictionaryFieldNames.RemoveAll();
    m_dataDictionaryFieldNamesWithDatasetName.RemoveAll();
  }

  if (m_dataDictionaryFieldNames.size() > 0)
  {
    return &m_dataDictionaryFieldNames;
  }


  if (m_tree.GetRoot() == NULL)
  {
    return &m_dataDictionaryFieldNames;
  }

  m_tree.SetWalkDownRootPivot();

  do
  {
    if (m_tree.GetWalkCurrent() == NULL)
    {
      throw CProductException("ERROR in CProduct::GetDataDictionaryFieldNames - at least one of the tree object has a NUll node",
                                m_currFileName, GetProductClass(), GetProductType(), BRATHL_LOGIC_ERROR);
    }

    CField *field  = dynamic_cast<CField*>(m_tree.GetWalkCurrent()->GetData());
    if (field == NULL)
    {
      throw CProductException("ERROR in CProduct::GetDataDictionaryFieldNames - at least one of the tree object is not a CField object",
                                m_currFileName, GetProductClass(), GetProductType(), BRATHL_LOGIC_ERROR);
    }

    string fieldFullName = field->GetFullName();
    string fieldNameWithRecord = field->GetFullNameWithRecord();

    if (fieldFullName.empty() == false)
    {

      //m_dataDictionaryFieldNames.InsertUnique(field->GetFullName());
      m_dataDictionaryFieldNames.Insert(field->GetFullName());

      string recordName = field->GetRecordName();

      //m_dataDictionaryFieldNamesWithDatasetName.InsertUnique(fieldNameWithRecord);
      m_dataDictionaryFieldNamesWithDatasetName.Insert(fieldNameWithRecord);
    }
  }
  while (m_tree.SubTreeWalkDown());

  return &m_dataDictionaryFieldNames;

}

//----------------------------------------
void CProduct::LoadFieldsInfo()
{


  m_tree.DeleteTree();
  m_listInfo.RemoveAll();

  m_listInfo.AddNew();

  GetRootType();

  TraverseData();

  //DumpDictionary("c:\\dev\\dumpdd.log");

  SetDynInfo();

  //RemoveUnusedFields();

}

//----------------------------------------
void CProduct::RemoveUnusedFields()
{
  m_tree.SetWalkDownRootPivot();
  m_tree.SubTreeWalkDown();
  do
  {
    CField *field  = m_tree.GetCurrentData();
    CObjectTreeNode* walkCurrent = m_tree.GetWalkCurrent();
    CObjectTreeNode* parent = walkCurrent->GetParent();

    if (field->IsToBeRemoved())
    {
      m_tree.GoLevelUp(false);
      parent->Delete(field->GetKey());
      m_tree.Go(parent);
      CFieldRecord* fieldParent = CTreeField::GetDataAsFieldRecordObject(parent, false);
      if (fieldParent != NULL)
      {
        fieldParent->SetNbFields(fieldParent->GetNbFields() - 1);
      }
    }


  }while (m_tree.SubTreeWalkDown());

//  DumpDictionary("c:\\dev\\dumpdd2.log");

}
//----------------------------------------

void CProduct::SetDynInfo()
{
  int32_t result = 0;

  GetRootType();


  int32_t nbDims = 0;
  //int32_t nbElts = 0;
  long dim[MAX_NUM_DIMS];

  m_tree.SetWalkDownRootPivot();
  m_tree.SubTreeWalkDown();
  int32_t level = m_tree.GetWalkCurrent()->GetLevel();

  do
  {
    int32_t available = 0;
    nbDims = 0;
    //nbElts = 0;
    memset(&dim, 0, sizeof(dim));

    while (level > m_tree.GetWalkCurrent()->GetLevel())
    {
      result = coda_cursor_goto_parent(&m_cursor);
      HandleBratError("coda_cursor_goto_parent in CProduct::SetDynInfo()");
      level--;

      // if the new current coda_type_class cursor is 'coda_array_class'
      // goto parent one more time to goto a record type class
      coda_type_class typeclassTmp;
      coda_cursor_get_type_class(&m_cursor, &typeclassTmp);
      if (typeclassTmp == coda_array_class)
      {
        coda_cursor_goto_parent(&m_cursor);
        HandleBratError("coda_cursor_goto_parent in CProduct::SetDynInfo()");
      }

    }

    level = m_tree.GetWalkCurrent()->GetLevel();

    CField *field  = m_tree.GetCurrentData();

    coda_type_class typeclass;
    coda_cursor_get_type_class(&m_cursor, &typeclass);

    //cout << field->GetTypeClass() <<" " << field->GetName() << endl;

    if (typeid(*field) == typeid(CFieldRecord))
    {

      CFieldRecord* fieldRecord = dynamic_cast<CFieldRecord*>(field);

      if (fieldRecord->GetTypeClass() == coda_array_class)
      {
        if (fieldRecord->IsFixedSize() == false)
        {
          //test the current type  cursor
          //if it's an array goto the first element before going to the record field
          if (typeclass == coda_array_class) //test the type of the current cursor
          {
            result = coda_cursor_goto_first_array_element(&m_cursor);
            HandleBratError("coda_cursor_goto_first_array_element in CProduct::SetDynInfo()");

            //result = brat_cursor_get_num_elements(&m_cursor, &nbElts);
            //HandleBratError("brat_cursor_get_num_elements in CProduct::SetDynInfo()");
          }

          result = coda_cursor_get_record_field_available_status(&m_cursor, field->GetIndex(), &available);
          HandleBratError("coda_cursor_get_record_field_available_status in CProduct::SetDynInfo()");

          if (available == 1)
          {
            result = coda_cursor_goto_record_field_by_name(&m_cursor, field->GetName().c_str());
            HandleBratError("coda_cursor_goto_record_field_by_name in CProduct::SetDynInfo()");

            result = coda_cursor_get_array_dim(&m_cursor, &nbDims, dim);
            HandleBratError("coda_cursor_get_array_dim in CProduct::SetDynInfo()");

            fieldRecord->SetDim(nbDims, dim);
          }
          else
          {
            fieldRecord->SetDim(0);
          }

          //result = brat_cursor_get_num_elements(&m_cursor, &nbElts);
          //HandleBratError("brat_cursor_get_num_elements in CProduct::SetDynInfo()");

        }
        else
        {
          //test the current type  cursor
          //if it's an array goto the first element before going to the record field
          if (typeclass == coda_array_class) //test the type of the current cursor
          {
            result = coda_cursor_goto_first_array_element(&m_cursor);
            HandleBratError("coda_cursor_goto_first_array_element in CProduct::SetDynInfo()");

            //result = brat_cursor_get_num_elements(&m_cursor, &nbElts);
            //HandleBratError("brat_cursor_get_num_elements in CProduct::SetDynInfo()");
          }
          result = coda_cursor_goto_record_field_by_name(&m_cursor, field->GetName().c_str());
          HandleBratError("coda_cursor_goto_record_field_by_name in CProduct::SetDynInfo()");

        }
      }
      else
      {
        //test the current type  cursor
        //if it's an array goto the first element before going to the record field
        if (typeclass == coda_array_class) //test the type of the current cursor
        {
          result = coda_cursor_goto_first_array_element(&m_cursor);
          HandleBratError("coda_cursor_goto_first_array_element in CProduct::SetDynInfo()");

        }

        if (field->IsUnion())
        {

          long indexUnion = -1;

          result = coda_cursor_get_available_union_field_index(&m_cursor, &indexUnion);
          HandleBratError("coda_cursor_get_available_union_field_index");

          if (indexUnion != field->GetIndex())
          {
            field->SetToBeRemoved(true);
          }

        }


        if (field->IsToBeRemoved())
        {
          m_tree.GoLevelUp(false); // Go one level up but don't reset parent child position
        }
        else
        {
          result = coda_cursor_goto_record_field_by_name(&m_cursor, field->GetName().c_str());
          HandleBratError("coda_cursor_goto_record_field_by_name in CProduct::SetDynInfo()");

          //coda_special_type special_type;

          //result = 	coda_cursor_get_special_type(&m_cursor, &special_type);
          ////HandleBratError("coda_cursor_get_special_type in CProduct::SetDynInfo()");

          //if (result == 0)
          //{
          //  field->SetSpecialType(special_type);
          //}
          //else
          //{
          //  coda_errno = 0;
          //}
        }
      }

    }
    else if (typeid(*field) == typeid(CFieldArray))
    {

      CFieldArray* fieldArray = dynamic_cast<CFieldArray*>(field);

      if ((fieldArray->GetTypeClass() == coda_array_class) && (fieldArray->IsFixedSize() == false))
      {

        result = coda_cursor_get_array_dim(&m_cursor, &nbDims, dim);
        HandleBratError("coda_cursor_get_array_dim in CProduct::SetDynInfo()");

      }


    }


  }while (m_tree.SubTreeWalkDown());


}

//----------------------------------------
void CProduct::FillListFields(const string& key)
{
  CStringList lstWork;

  m_fieldsToProcess.clear();

  m_fieldsToProcess.ExtractKeys(key, CTreeField::m_keyDelimiter);

  if (m_fieldsToProcess.empty())
  {
    string msg = CTools::Format("ERROR - CProduct::FillListFields(key) - invalid key '%s' - No field found - Verify your field syntax with the file structure in the datadictionary",
                                key.c_str());
    CProductException e(msg, m_currFileName, GetProductClass(), GetProductType(), BRATHL_LOGIC_ERROR);
    CTrace::Tracer("%s", e.what());
    Dump(*CTrace::GetDumpContext());
    throw (e);
  }

  if (m_tree.IsKey(key) == false)
  {
    string msg = CTools::Format("ERROR - CProduct::FillListFields(key) - invalid key '%s' - Field does not exist or some elements in its name are missing - Verify your field syntax with the file structure in the datadictionary",
                                key.c_str());
    CProductException e(msg, m_currFileName, GetProductClass(), GetProductType(), BRATHL_LOGIC_ERROR);
    CTrace::Tracer("%s", e.what());
    Dump(*CTrace::GetDumpContext());
    throw (e);
  }

  if (m_fieldsToProcess.front().compare(CProduct::m_treeRootName) == 0)
  {
    m_fieldsToProcess.pop_front();
  }

  return;

}
//----------------------------------------
bool CProduct::IsNetCdf()
{
  CProductNetCdf* productNetCdf = dynamic_cast<CProductNetCdf*>(this);
  return (productNetCdf != NULL);
}
//----------------------------------------
string CProduct::MakeInternalNameByAddingRoot(const string& name)
{
  return CProduct::m_treeRootName + CTreeField::m_keyDelimiter + name;
}
//----------------------------------------
string CProduct::MakeInternalFieldName(const string& dataSetName, const string& field)
{
  return CProduct::m_treeRootName + CTreeField::m_keyDelimiter + dataSetName + CTreeField::m_keyDelimiter + field;
}
//----------------------------------------
string CProduct::MakeInternalFieldName(const string& field)
{
  return MakeInternalFieldName(m_dataSetNameToRead, field);
}
//----------------------------------------
string CProduct::MakeInternalDataSetName(const string& dataSetName)
{
  return MakeInternalNameByAddingRoot(dataSetName);
}

//----------------------------------------
void CProduct::InitInternalFieldName(const string& field, bool convertDate /*= false*/)
{
  CStringList listField;

  listField.Insert( field );

  InitInternalFieldName(listField, convertDate);

}
//----------------------------------------
void CProduct::InitInternalFieldName(CStringList& listField, bool convertDate /*= false*/)
{
  InitInternalFieldName(m_dataSetNameToRead, listField, convertDate);
}
//----------------------------------------
void CProduct::InitInternalFieldName(const string& dataSetName, CStringList& listField, bool convertDate /*= false*/)
{
  m_listInternalFieldName.RemoveAll();
  m_fieldNameEquivalence.RemoveAll();
  m_listFieldOrigin.RemoveAll();
  m_listFieldOrigin.Insert(listField);

  m_tree.ResetHiddenFlag();

  CStringList::iterator itField;

  string internalFieldName;

  for (itField = listField.begin() ; itField != listField.end() ; itField++)
  {
    CField* field = FindFieldByName(*itField, dataSetName);
    
    if (field == NULL)
    {
      string msg = CTools::Format("ERROR in CProduct::InitInternalFieldName - field '%s' in record name '%s' not found",
                                  (*itField).c_str(), dataSetName.c_str());
      CProductException e(msg, m_currFileName, GetProductClass(), GetProductType(), BRATHL_LOGIC_ERROR);
      CTrace::Tracer("%s", e.what());
      Dump(*CTrace::GetDumpContext());
      throw (e);
    }

    string internalFieldName = MakeInternalNameByAddingRoot(field->GetFullNameWithRecord());
/*
    CField* field = FindFieldByInternalName( internalFieldName, false);
    if (field == NULL)
    {
      internalFieldName = MakeInternalFieldName(dataSetName, (*itField));
    }
*/
    m_listInternalFieldName.Insert( internalFieldName );
    m_fieldNameEquivalence.Insert((*itField), internalFieldName);
  }

  CheckFields(convertDate);

}
/*
//----------------------------------------
void CProduct::ReadBratFile(const string& fileName, const string& dataSetName, const string& field)
{

  CStringList listField;

  listField.Insert(field);

  ReadBratFile(fileName, dataSetName, listField);

}


//----------------------------------------
void CProduct::ReadBratFile(const string& fileName, const string& dataSetName, CStringList& listField)
{
  CStringList::iterator itField;

  m_dataSet.RemoveAll();

  m_recordCount = 0;

  string str;

  Open(fileName);

  for (itField = listField.begin() ; itField != listField.end() ; itField++)
  {
    str = MakeInternalFieldName(dataSetName, (*itField));
    ReadBratField(str);
  }
  // Close current file
  m_recordCount += m_dataSet.size();
  Close();

}
*/

//----------------------------------------
string CProduct::DatasetRecordsNumberToString(const CIntMap& datasetRecordsNumber)
{
  string str;
  CIntMap::const_iterator itMap;

  for (itMap = datasetRecordsNumber.begin() ; itMap != datasetRecordsNumber.end() ; itMap++)
  {
    string key = itMap->first;
    int32_t num = itMap->second;
    str.append(CTools::Format("Dataset/Record name: '%s' has %d record(s)\n", key.c_str(), num));
  }

  return str;

}
//----------------------------------------
int32_t CProduct::GetNumberOfRecords()
{
  //return GetNumberOfRecords(m_dataSetNameToRead);
  if (m_nbRecords >= 0)
  {
    return m_nbRecords;
  }

  if (IsOpened() == false)
  {
    return -1;
  }



  int32_t min = 0;
  int32_t max = 0;
  CIntMap datasetRecordsNumber;

  GetMinMaxNumberOfRecords(min, max, &datasetRecordsNumber);

  m_nbRecords = max;

  string msgRecordNumber = DatasetRecordsNumberToString(datasetRecordsNumber);

  if (min <= 0)
  {
    m_nbRecords = min;

    string msg = CTools::Format("Regarding the file '%s', at least one dataset contains no data:\n\n%s\n\nFile '%s' will be skipped\n",
      m_currFileName.c_str(), msgRecordNumber.c_str(), m_currFileName.c_str());
    CTrace::Tracer(1,"\n>>>>>>>>>>>>>>>>>>>> WARNING >>>>>>>>>>>>>>>>>>>>>>>>>\n");
    CTrace::Tracer(1,"%s", msg.c_str());
    CTrace::Tracer(1,">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
  }
  else
  {
    if (datasetRecordsNumber.size() > 1)
    {
      string msg = CTools::Format("Regarding the file '%s', number of records for each dataset are:\n\n%s\n",
        m_currFileName.c_str(), msgRecordNumber.c_str());
      CTrace::Tracer(1,"\n>>>>>>>>>>>>>>>>>>>> INFORMATION >>>>>>>>>>>>>>>>>>>>>>>>>\n");
      CTrace::Tracer(1,"%s",msg.c_str());
      CTrace::Tracer(1,">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    }

    CIntMap datasetRecordsNumberTmp;

    GetMinMaxNumberOfRecords(min, max, &datasetRecordsNumberTmp, 1);


    if (!HasEqualsNumberOfRecord(datasetRecordsNumberTmp))
    {
      string msgRecordNumberTmp = DatasetRecordsNumberToString(datasetRecordsNumberTmp);
      string msg = CTools::Format("Regarding the file '%s', number of records for datasets below are different.\n"
                                  "Only %d data record(s) will be consider.\n\n%s\n",
        m_currFileName.c_str(), min, msgRecordNumberTmp.c_str());
      CTrace::Tracer(1,"\n>>>>>>>>>>>>>>>>>>>> WARNING >>>>>>>>>>>>>>>>>>>>>>>>>\n");
      CTrace::Tracer(1,"%s", msg.c_str());
      CTrace::Tracer(1,">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

      m_nbRecords = min;
    }


  }

  return m_nbRecords;

}
//----------------------------------------
int32_t CProduct::GetNumberOfRecords(const string& dataSetName)
{
  if (m_nbRecords >= 0)
  {
    return m_nbRecords;
  }

  if (IsOpened() == false)
  {
    return -1;
  }

  if (dataSetName.empty())
  {
    /*
    CProductException e("ERROR in CProduct::GetNumberOfRecords - for this product dataset is mandatory", m_currFileName, GetProductClass(), GetProductType(), BRATHL_LOGIC_ERROR);
    CTrace::Tracer("%s", e.what());
    Dump(*CTrace::GetDumpContext());
    throw (e);
    */
    return -1;
  }

//  string str = CProduct::m_treeRootName + CTreeField::m_keyDelimiter + dataSetName;

  CFieldRecord* field =  dynamic_cast<CFieldRecord*>(FindFieldByInternalName( MakeInternalDataSetName(dataSetName),
                                                                              false ));

  if (field == NULL)
  {
    string msg = CTools::Format("ERROR in CProduct::GetNumberOfRecords - field '%s' not found",
                                dataSetName.c_str());
    CProductException e(msg, m_currFileName, GetProductClass(), GetProductType(), BRATHL_LOGIC_ERROR);
    CTrace::Tracer("%s", e.what());
    Dump(*CTrace::GetDumpContext());
    throw (e);
  }

  m_nbRecords = field->GetMaxPos() + 1;
  return m_nbRecords;

}

//----------------------------------------
void CProduct::GetNumberOfRecords(const CStringList& datasetNames, CIntMap& datasetRecordsNumber)
{


  CStringList tempList;
  CStringList::const_iterator itDataset;

  CStringList::iterator itTempList;

  for (itDataset = datasetNames.begin() ; itDataset != datasetNames.end() ; itDataset++)
  {

    CFieldRecord* field =  dynamic_cast<CFieldRecord*>(FindFieldByInternalName( MakeInternalDataSetName(*itDataset),
                                                                              false ));
    if (field == NULL)
    {
      string msg = CTools::Format("ERROR in CProduct::GetNumberOfRecords - field '%s' is not a record",
                                  itDataset->c_str());
      CProductException e(msg, m_currFileName, GetProductClass(), GetProductType(), BRATHL_LOGIC_ERROR);
      CTrace::Tracer("%s", e.what());
      Dump(*CTrace::GetDumpContext());
      throw (e);
    }

//    if (field->IsFixedSize())
//    {
//      continue;
//    }

    datasetRecordsNumber.Insert(*itDataset, field->GetNbElts());

  }

}
//----------------------------------------
void CProduct::GetMinMaxNumberOfRecords(int32_t& min, int32_t& max, CIntMap* datasetRecordsNumber /* = NULL */, int32_t minThreshold /* = -1 */)
{


  if (m_listInternalFieldName.size() <= 0)
  {
    string msg = "ERROR in CProduct::GetMinMaxNumberOfRecords - List of fields is empty (m_listInternalFieldName) -  Perhaps CProduct::SetListFieldToRead has not been called yet.";
    CProductException e(msg, m_currFileName, GetProductClass(), GetProductType(), BRATHL_LOGIC_ERROR);
    CTrace::Tracer("%s", e.what());
    Dump(*CTrace::GetDumpContext());
    throw (e);

  }


  CStringList datasetNames;

  ExtractDatasetNamesFromFields(m_listInternalFieldName, datasetNames);

  CIntMap datasetRecordsNumberTmp;

  GetNumberOfRecords(datasetNames, datasetRecordsNumberTmp);


  CTools::SetDefaultValue(min);
  CTools::SetDefaultValue(max);


  CIntMap::iterator itMap;

  for (itMap = datasetRecordsNumberTmp.begin() ; itMap != datasetRecordsNumberTmp.end() ; itMap++)
  {
    int32_t num = itMap->second;

    if (num <= minThreshold)
    {
      continue;
    }

    if (datasetRecordsNumber != NULL)
    {
      datasetRecordsNumber->Insert(itMap->first, itMap->second);
    }

    if (CTools::IsDefaultValue(max))
    {
      max = num;
    }
    else if (num > max)
    {
      max = num;
    }

    if (CTools::IsDefaultValue(min))
    {
      min = num;
    }
    else if (num < min)
    {
      min = num;
    }
  }

  if (CTools::IsDefaultValue(min))
  {
    min = 0;
  }

  if (CTools::IsDefaultValue(max))
  {
    max = 0;
  }


}

//----------------------------------------
bool CProduct::HasEqualsNumberOfRecord(const CIntMap& datasetRecordsNumber)
{
  bool isEqual = true;

  if (datasetRecordsNumber.size() <= 0)
  {
    return isEqual;
  }



  CIntMap::const_iterator itMapFirst = datasetRecordsNumber.begin();
  int32_t num = itMapFirst->second;

  CIntMap::const_iterator itMap;

  for (itMap = datasetRecordsNumber.begin()++ ; itMap != datasetRecordsNumber.end() ; itMap++)
  {
    if (num != itMap->second)
    {
      isEqual = false;
    }
  }

  return isEqual;


}
//----------------------------------------
void CProduct::ExtractDatasetNamesFromFields(const CStringList& listFields, CStringList& datasetNames)
{

  if (m_dataSetNameToRead.empty() == false)
  {
    datasetNames.InsertUnique(m_dataSetNameToRead);
  }

  CStringList tempList;
  CStringList::const_iterator itField;

  CStringList::iterator itTempList;

  for (itField = listFields.begin() ; itField != listFields.end() ; itField++)
  {
    tempList.ExtractStrings(*itField, CTreeField::m_keyDelimiter);

    for (itTempList = tempList.begin() ; itTempList != tempList.end() ; itTempList++)
    {
      if ((*itTempList).compare(CProduct::m_treeRootName) == 0)
      {
        continue;
      }

      CFieldRecord* field =  dynamic_cast<CFieldRecord*>(FindFieldByInternalName( MakeInternalDataSetName(*itTempList),
                                                                              false ));
      if (field == NULL)
      {
        continue;
      }

//      if (field->IsFixedSize())
//      {
//        continue;
//      }

      datasetNames.InsertUnique(*itTempList);

      break;

    }

  }

}
/*
//----------------------------------------
void CProduct::ExpandArray()
{
  bool canExpandArray = true;
  bool hasArray = false;

  int32_t countRecord = m_dataSet.size();

  if (countRecord <= 0)
  {
    return;
  }

  // Get the first recordset
  CRecordSet* recordSetToProcess = m_dataSet.GetFirstRecordSet();

  // All array must have the same dimensions
  int32_t nbDims;
  CTools::SetDefaultValue(nbDims);
  CUIntArray dim;

  CRecordSet::iterator it;
  for (it = recordSetToProcess->begin() ; it != recordSetToProcess->end() ; it++)
  {
    CFieldSet* fieldSet = recordSetToProcess->GetFieldSet(it);
    //CFieldSetDbl* fieldSetDbl = dynamic_cast<CFieldSetDbl*>(fieldSet);
    CFieldSetArrayDbl* fieldSetArrayDbl = dynamic_cast<CFieldSetArrayDbl*>(fieldSet);

    // the field is hidden, it is just use for calculation, don't set it in output
    if (fieldSet->GetField()->IsHidden())
    {
      continue;
    }

    // data is not an array of double
    if (fieldSetArrayDbl == NULL)
    {
      continue;
    }

    hasArray = true;

    if (CTools::IsDefaultValue(nbDims))
    {
      nbDims = fieldSetArrayDbl->m_nbDims;
      dim.Insert(fieldSetArrayDbl->m_dim);
      continue;
    }

    // Array must have the same number of dimensions and the same element values
    if (dim != fieldSetArrayDbl->m_dim)
    {
      canExpandArray = false;
      break;
    }
  }

  if (canExpandArray == false)
  {
    return;
  }

  if (hasArray == false)
  {
    return;
  }

  // Create a new dataset which contains simple fields
  // This dataset will contains as  many records as number of array element number
  CDataSet dataSetArray("Array", false);
  uint32_t insertRecordAt = 0;

  uint32_t repeat = 1;
  for (int32_t i = 0 ; i < nbDims ; i++)
  {
    repeat *= dim[i];
  }


  for (int32_t index = 0 ; index < countRecord ; index++)
  {
    recordSetToProcess = m_dataSet.GetRecordSet(index);

    if (recordSetToProcess == NULL)
    {
      string msg = "ERROR in CProduct::ExpandArray - No current recordset";
      CProductException e(msg, m_currFileName, GetProductClass(), GetProductType(), BRATHL_LOGIC_ERROR);
      throw (e);
    }

    //insertRecordAt = (dataSetArray.size() == 0) ? 0 : dataSetArray.size() - 1;
    insertRecordAt = dataSetArray.size();

    CRecordSet::iterator it;
    for (it = recordSetToProcess->begin() ; it != recordSetToProcess->end() ; it++)
    {
      CFieldSet* fieldSet = recordSetToProcess->GetFieldSet(it);
      CFieldSetDbl* fieldSetDbl = dynamic_cast<CFieldSetDbl*>(fieldSet);
      CFieldSetArrayDbl* fieldSetArrayDbl = dynamic_cast<CFieldSetArrayDbl*>(fieldSet);

      // the field is hidden, it is just use for calculation, don't set it in output
      if (fieldSet->GetField()->IsHidden())
      {
        continue;
      }

      // data is an array of double
      if (fieldSetArrayDbl != NULL)
      {
        // field is an array
        // put it into the set of records (array is repeated for each record)
        PutFlat(&dataSetArray, fieldSetArrayDbl, insertRecordAt);
      }

      // data is a double
      if (fieldSetDbl != NULL)
      {
        // field is a double
        // put it into the set of records (value is repeated for each record)
        Put(&dataSetArray, fieldSetDbl, repeat, insertRecordAt);
      }
    }

    recordSetToProcess = NULL;
  }

  // Erase the main dataset
  m_dataSet.RemoveAll();

  // Insert the records of the new dataset in the main dataset
  m_dataSet.InsertDataset(&dataSetArray);
}
*/
//----------------------------------------
void CProduct::ExpandFieldsArray()
{

  if (m_listFieldExpandArray.size() <= 0)
  {
    return;
  }

  bool canExpandArray = true;
  bool hasArray = false;

  int32_t countRecord = m_dataSet.size();

  if (countRecord <= 0)
  {
    return;
  }

  // Get the first recordset
  CRecordSet* recordSetToProcess = m_dataSet.GetFirstRecordSet();

  // All array must have the same dimensions
  int32_t nbDims;
  CTools::SetDefaultValue(nbDims);
  CUIntArray dim;

  CRecordSet::iterator it;
  for (it = recordSetToProcess->begin() ; it != recordSetToProcess->end() ; it++)
  {
    CFieldSet* fieldSet = recordSetToProcess->GetFieldSet(it);
    if (fieldSet == NULL)
    {
      continue;
    }

    if (! recordSetToProcess->IsFieldHasToBeExpanded(it, m_listFieldExpandArray) )
    {
      continue;
    }
    //CFieldSetDbl* fieldSetDbl = dynamic_cast<CFieldSetDbl*>(fieldSet);
    CFieldSetArrayDbl* fieldSetArrayDbl = dynamic_cast<CFieldSetArrayDbl*>(fieldSet);

    // the field is hidden, it is just use for calculation, don't set it in output
    if (fieldSet->GetField()->IsHidden())
    {
      continue;
    }

    // data is not an array of double
    if (fieldSetArrayDbl == NULL)
    {
      continue;
    }

    hasArray = true;

    if (CTools::IsDefaultValue(nbDims))
    {
      nbDims = fieldSetArrayDbl->m_nbDims;
      dim.Insert(fieldSetArrayDbl->m_dim);
      continue;
    }

    // Array must have the same number of dimensions and the same element values
    if (dim != fieldSetArrayDbl->m_dim)
    {
      canExpandArray = false;
      break;
    }
  }

  if (hasArray == false)
  {
    return;
  }

  if (canExpandArray == false)
  {
    return;
  }


  // Create a new dataset which contains simple fields
  // This dataset will contains as  many records as number of array element number
  CDataSet dataSetArray("Array", false);
  uint32_t insertRecordAt = 0;

  uint32_t repeat = 1;
  for (int32_t i = 0 ; i < nbDims ; i++)
  {
    repeat *= dim[i];
  }


  for (int32_t index = 0 ; index < countRecord ; index++)
  {
    recordSetToProcess = m_dataSet.GetRecordSet(index);

    if (recordSetToProcess == NULL)
    {
      string msg = "ERROR in CProduct::ExpandFieldsArray - No current recordset";
      CProductException e(msg, m_currFileName, GetProductClass(), GetProductType(), BRATHL_LOGIC_ERROR);
      throw (e);
    }

    //insertRecordAt = (dataSetArray.size() == 0) ? 0 : dataSetArray.size() - 1;
    insertRecordAt = dataSetArray.size();

    CRecordSet::iterator it;
    for (it = recordSetToProcess->begin() ; it != recordSetToProcess->end() ; it++)
    {
      CFieldSet* fieldSet = recordSetToProcess->GetFieldSet(it);
      CFieldSetDbl* fieldSetDbl = dynamic_cast<CFieldSetDbl*>(fieldSet);
      CFieldSetArrayDbl* fieldSetArrayDbl = dynamic_cast<CFieldSetArrayDbl*>(fieldSet);

      // the field is hidden, it is just use for calculation, don't set it in output
      if (fieldSet->GetField()->IsHidden())
      {
        continue;
      }

      // data is an array of double
      if (fieldSetArrayDbl != NULL)
      {
        // field is an array and have to be expanded
        // put it into the set of records (array is repeated for each record)

        if (recordSetToProcess->IsFieldHasToBeExpanded(it, m_listFieldExpandArray) )
        {
          PutFlat(&dataSetArray, fieldSetArrayDbl, insertRecordAt);
        }
        else
        {
          Put(&dataSetArray, fieldSetArrayDbl, repeat, insertRecordAt);
        }

      }

      // data is a double
      if (fieldSetDbl != NULL)
      {
        // field is a double
        // put it into the set of records (value is repeated for each record)
        Put(&dataSetArray, fieldSetDbl, repeat, insertRecordAt);
      }
    }

    recordSetToProcess = NULL;
  }

  // Erase the main dataset
  m_dataSet.RemoveAll();

  // Insert the records of the new dataset in the main dataset
  m_dataSet.InsertDataset(&dataSetArray);
}
//----------------------------------------
void CProduct::ProcessHighResolution()
{
  if (m_hasHighResolutionFieldToProcess == false)
  {
    return;
  }

  if (HasHighResolutionFieldCalculation())
  {
    // there are fields in the product which allow high resolution calculation
    //(e.g. latitude difference, longitude difference, timestamp difference, it depends of the product - see derived classes of CProduct)
    ProcessHighResolutionWithFieldCalculation();
  }
  else
  {
    // there are no fields in the product which allow high resolution calculation
    //(e.g. latitude difference, longitude difference, timestamp difference, it depends of the product - see derived classes of CProduct)
    // so latitude, longitude et timestamp are estimated
    ProcessHighResolutionWithoutFieldCalculation();
  }


}
//----------------------------------------
void CProduct::ProcessHighResolutionWithFieldCalculation()
{

}
//----------------------------------------
void CProduct::ProcessHighResolutionWithoutFieldCalculation()
{
}


//----------------------------------------
void CProduct::CheckConsistencyHighResolutionField(CFieldSetArrayDbl* fieldSetArrayDbl)
{
  if ( (fieldSetArrayDbl->m_nbDims <= 0) || (fieldSetArrayDbl->m_nbDims > 2) )
  {
    string msg = CTools::Format("ERROR - CProduct::CheckConsistencyHighResolutionField() - Number of array dim %d not implemented for this method "
                                "(field '%s')",
                                fieldSetArrayDbl->m_nbDims,
                                fieldSetArrayDbl->GetField()->GetKey().c_str());
    CProductException e(msg, m_currFileName, GetProductClass(), GetProductType(), BRATHL_UNIMPLEMENT_ERROR);
    throw (e);
  }

  if (CTools::IsDefaultValue(m_numHighResolutionMeasure))
  {
    string msg = CTools::Format("ERROR - CProduct::CheckConsistencyHighResolutionField() - Number of high resolution measures is not intialized (equals default value"
                                "(field '%s')",
                                fieldSetArrayDbl->GetField()->GetKey().c_str());

    CProductException e(msg, m_currFileName, GetProductClass(), GetProductType(), BRATHL_INCONSISTENCY_ERROR);
    throw (e);
  }

  if (fieldSetArrayDbl->m_dim[0] != m_numHighResolutionMeasure)
  {
    string msg = CTools::Format("ERROR - CProduct::CheckConsistencyHighResolutionField() - Array dim[0]: %d is not equal to numer of high resolution measures: %d "
                                "(field '%s')",
                                fieldSetArrayDbl->m_dim[0],
                                m_numHighResolutionMeasure,
                                fieldSetArrayDbl->GetField()->GetKey().c_str());

    CProductException e(msg, m_currFileName, GetProductClass(), GetProductType(), BRATHL_INCONSISTENCY_ERROR);
    throw (e);
  }
}
//----------------------------------------
void CProduct::PutFlat(CDataSet* dataSet, CFieldSetArrayDbl* fieldSetArrayDbl, uint32_t insertRecordAt /*= 0*/)
{
  if (dataSet == NULL)
  {
    return;
  }
  if (fieldSetArrayDbl == NULL)
  {
    return;
  }

  int32_t nbDims = fieldSetArrayDbl->m_nbDims;
  uint32_t count = 1;
  for (int32_t iDim = 0 ; iDim < nbDims ; iDim++)
  {
    count *= fieldSetArrayDbl->m_dim[iDim];
  }

  for (uint32_t i = 0 ; i < count ; i++)
  {
    CFieldSetDbl* fieldSetDblTmp = new CFieldSetDbl(fieldSetArrayDbl->GetName());
    fieldSetDblTmp->m_value = fieldSetArrayDbl->m_vector[i];
    fieldSetDblTmp->SetField(fieldSetArrayDbl->GetField());

    InsertRecord(*dataSet, (insertRecordAt + i + 1));

    dataSet->InsertFieldSet(fieldSetDblTmp->GetField()->GetKey(), fieldSetDblTmp);
  }
}


//----------------------------------------
void CProduct::PutFlatHighResolution(CDataSet* dataSet, CFieldSetArrayDbl* fieldSetArrayDbl)
{
  if (dataSet == NULL)
  {
    return;
  }
  if (fieldSetArrayDbl == NULL)
  {
    return;
  }

  CheckConsistencyHighResolutionField(fieldSetArrayDbl);

  CFieldSet* fieldSet = NULL;

  for (uint32_t i = 0 ; i < m_numHighResolutionMeasure ; i++)
  {
    fieldSet = NULL;
    if (fieldSetArrayDbl->m_nbDims == 1)
    {
      CFieldSetDbl* fieldSetDblTmp = new CFieldSetDbl(fieldSetArrayDbl->GetName());
      fieldSetDblTmp->m_value = fieldSetArrayDbl->m_vector[i];
      fieldSetDblTmp->SetField(fieldSetArrayDbl->GetField());
      fieldSet = fieldSetDblTmp;
    }
    else if (fieldSetArrayDbl->m_nbDims == 2)
    {
      CFieldSetArrayDbl* fieldSetArrayDblTmp = new CFieldSetArrayDbl(fieldSetArrayDbl->GetName());
      fieldSetArrayDblTmp->m_nbDims = 1;
      fieldSetArrayDblTmp->m_dim.Insert(fieldSetArrayDbl->m_dim[1]);
      fieldSetArrayDblTmp->m_vector.Insert(fieldSetArrayDbl->m_vector,
                                           i * fieldSetArrayDblTmp->m_dim[0],
                                           ((i + 1) * fieldSetArrayDblTmp->m_dim[0]));
      fieldSetArrayDblTmp->SetField(fieldSetArrayDbl->GetField());
      fieldSet = fieldSetArrayDblTmp;

    }
    else
    {
      string msg = CTools::Format("ERROR - CProduct::PutFlatHighResolution() - Number of array dim %d not implemented for this method "
                                  "(field '%s')",
                                  fieldSetArrayDbl->m_nbDims,
                                  fieldSetArrayDbl->GetField()->GetKey().c_str());
      CProductException e(msg, m_currFileName, GetProductClass(), GetProductType(), BRATHL_UNIMPLEMENT_ERROR);
      throw (e);
    }

    InsertRecord(*dataSet, (i + 1));

    dataSet->InsertFieldSet(fieldSet->GetField()->GetKey(), fieldSet);
  }

}
//----------------------------------------
void CProduct::Put(CDataSet* dataSet, CFieldSetDbl* fieldSetDbl, uint32_t repeat, uint32_t insertRecordAt /*= 0*/ )
{
  if (dataSet == NULL)
  {
    return;
  }
  if (fieldSetDbl == NULL)
  {
    return;
  }

  for (uint32_t i = 0 ; i < repeat ; i++)
  {
    CFieldSetDbl* fieldSetDblTmp = new CFieldSetDbl(*fieldSetDbl);

    InsertRecord(*dataSet, (insertRecordAt + i + 1));

    dataSet->InsertFieldSet(fieldSetDblTmp->GetField()->GetKey(), fieldSetDblTmp);
  }
}
//----------------------------------------
void CProduct::Put(CDataSet* dataSet, CFieldSetArrayDbl* fieldSetArrayDbl, uint32_t repeat, uint32_t insertRecordAt /*= 0*/ )
{
  if (dataSet == NULL)
  {
    return;
  }
  if (fieldSetArrayDbl == NULL)
  {
    return;
  }

  if (CTools::IsDefaultValue(repeat))
  {
    string msg = CTools::Format("ERROR - CProduct::Put() - Number of high resolution measures is not intialized (equals default value"
                                "(field '%s')",
                                fieldSetArrayDbl->GetField()->GetKey().c_str());

    CProductException e(msg, m_currFileName, GetProductClass(), GetProductType(), BRATHL_INCONSISTENCY_ERROR);
    throw (e);
  }

  for (uint32_t i = 0 ; i < repeat ; i++)
  {
    CFieldSetArrayDbl* fieldSetArrayDblTmp = new CFieldSetArrayDbl(*fieldSetArrayDbl);

    InsertRecord(*dataSet, (insertRecordAt + i + 1));

    dataSet->InsertFieldSet(fieldSetArrayDblTmp->GetField()->GetKey(), fieldSetArrayDblTmp);
  }

}
//----------------------------------------
void CProduct::Put(CDataSet* dataSet, CFieldSetDbl* fieldSetDbl)
{
  if (dataSet == NULL)
  {
    return;
  }
  if (fieldSetDbl == NULL)
  {
    return;
  }

  if (CTools::IsDefaultValue(m_numHighResolutionMeasure))
  {
    string msg = CTools::Format("ERROR - CProduct::Put() - Number of high resolution measures is not intialized (equals default value"
                                "(field '%s')",
                                fieldSetDbl->GetField()->GetKey().c_str());

    CProductException e(msg, m_currFileName, GetProductClass(), GetProductType(), BRATHL_INCONSISTENCY_ERROR);
    throw (e);
  }

  for (uint32_t i = 0 ; i < m_numHighResolutionMeasure ; i++)
  {
    CFieldSetDbl* fieldSetDblTmp = new CFieldSetDbl(*fieldSetDbl);

    if (i != 0)
    {
      // If field an index data: 
      // - Increment offsets: 
      // - Set new value (incremented)
      CFieldIndexData* fieldIndexData = dynamic_cast<CFieldIndexData*>(fieldSetDblTmp->GetField());
      if (fieldIndexData != NULL)
      {       
        AddOffset(1.0, fieldIndexData);
        fieldSetDblTmp->SetData(fieldIndexData->GetValue() - 1);
      }
    }

    InsertRecord(*dataSet, (i + 1));
    
    dataSet->InsertFieldSet(fieldSetDblTmp->GetField()->GetKey(), fieldSetDblTmp);
  }

}

//----------------------------------------
void CProduct::AddOffset(double value, CField* field /*= NULL*/)
{
  m_offset += value;

  if (field != NULL)
  {
    CFieldIndexData* fieldIndexData = dynamic_cast<CFieldIndexData*>(field);
    if (fieldIndexData != NULL)
    {
      fieldIndexData->AddOffset(value);
    }
  }
}
//----------------------------------------
void CProduct::FillDescription()
{
}

//----------------------------------------
void CProduct::ReadBratRecord(const string& dataSetName, const string& field, int32_t iRecord)
{
  m_dataSetNameToRead = dataSetName;

  InitInternalFieldName(field);

  ReadBratRecord(iRecord);
}
//----------------------------------------
void CProduct::ReadBratRecord(const string& dataSetName, CStringList& listField, int32_t iRecord)
{
  m_dataSetNameToRead = dataSetName;

  InitInternalFieldName(listField);

  ReadBratRecord(iRecord);

}
//----------------------------------------
void CProduct::ReadBratRecord(int32_t iRecord)
{
  CheckFileOpened();

  if (iRecord < 0)
  {
    string msg = CTools::Format("ERROR in CProduct::ReadBratRecord - record index %d out of range (min = 0)",
                                iRecord);
    CProductException e(msg, m_currFileName, GetProductClass(), GetProductType(), BRATHL_RANGE_ERROR);
    CTrace::Tracer("%s", e.what());
    Dump(*CTrace::GetDumpContext());
    throw (e);
  }

  m_currentRecord = iRecord;

  CStringList::iterator itField;

  m_dataSet.RemoveAll();

  m_recordCount = 0;

  string str;


  if (!m_disableTrace)
  {
    if (CTools::Mod(iRecord+1, m_countForTrace * m_traceProcessRecordRatio) == 0)
    {
      CTrace::Tracer(1,"Process record %d of %d", iRecord+1, m_nbRecords);
      m_traceProcessRecordRatio++;
    }
  }

  for (itField = m_listInternalFieldName.begin() ; itField != m_listInternalFieldName.end() ; itField++)
  {
    ReadBratFieldRecord((*itField), iRecord);
  }

  // high resolution field processing
  ProcessHighResolution();

  // array processing : transform each array element to a field record
  /*
  if (m_expandArray)
  {
    ExpandArray();
  }
  */

  ExpandFieldsArray();

  m_recordCount += m_dataSet.size();

  if (iRecord >= (this->GetNumberOfRecords() - 1))
  {
    AddOffset(this->GetNumberOfRecords());
  }

}


//----------------------------------------
void CProduct::ReadBratFieldRecord(const string& key, int32_t iRecord)
{

  m_listFields.RemoveAll();

  FillListFields(key);

  GetRootType();

  m_tree.GetRootData()->SetCursor(m_cursor);

  m_tree.SetWalkDownRootPivot();

  CField* field = NULL;
  bool bFoundField = false;

  do
  {
    bFoundField = false;
    bool bOk = m_tree.SubTreeWalkDown();
    if (bOk == false) // No more child
    {
      break;
    }

    field = m_tree.GetCurrentData();

    //if (field->GetName().compare(m_fieldsToProcess.front()) == 0)
    if (field->GetKey().compare(m_fieldsToProcess.front()) == 0)
    {
      bool hasDataset = true;
      if (IsNetCdf())
      {
        hasDataset = false;
      }
      bFoundField = true;
      m_listFields.InsertField(field, hasDataset);
      m_fieldsToProcess.pop_front();
    }
  }
  while (m_fieldsToProcess.empty() == false);

  if (bFoundField == false)
  {
    string msg = CTools::Format("ERROR - CProduct::ReadBratField() - Unknown dataset name or field name '%s'"
                                "(internal name searched:'%s'",
                                m_fieldsToProcess.front().c_str(),
                                key.c_str());
    CProductException e(msg, m_currFileName, GetProductClass(), GetProductType(), BRATHL_INCONSISTENCY_ERROR);
    CTrace::Tracer("%s", e.what());
    //Dump(*CTrace::GetDumpContext());
    throw (e);
  }


  CField::CListField::iterator it = m_listFields.begin();

  // If m_listFields contains only one item, there is no record, all fields are at the first level.
  if (m_listFields.size() == 1)
  {
    m_dataSet.SetName("Dataset");
  }
  else
  {
    field = static_cast<CField*>(*(it));
    m_dataSet.SetName(field->GetName());
  }

  field = static_cast<CField*>(*(it));
  field->SetCurrentPos(iRecord);
  
  if ( field->End() )
  {
    field->SetCurrentPosToLast();
//    string msg = CTools::Format("ERROR in CProduct::ReadBratFieldRecord - record index %d out of range (max = %d)",
//                                iRecord,
//                                field->GetMaxPos());
//    CProductException e(msg, m_currFileName, GetProductClass(), GetProductType(), BRATHL_RANGE_ERROR);
//    CTrace::Tracer(e.what());
//    Dump(*CTrace::GetDumpContext());
//    throw (e);
  }


  ReadBratFieldRecord(it);

  return;

}
//----------------------------------------
void CProduct::ReadBratFieldRecord(CField::CListField::iterator it)
{
  bool skipRecord = false;
  ReadBratFieldRecord(it, skipRecord);
}
//----------------------------------------
void CProduct::ReadBratFieldRecord(CField::CListField::iterator it, bool& skipRecord)
{

  CField *field = static_cast<CField*>(*(it));

  SetCursor(field, skipRecord);

  it++;

  if (it == m_listFields.end())
  {
    CDoubleArray vect;
    string value;

    CObList parentFieldList(false);

    // find the topmost parent fixed record Array of this field
    // topmost parent is at the beginning of the list, lowermost parent is at the end of the list
    // find parent is for specifiec product (as Cryosat by now) because of their data structure (see time_orbit_data, for example)
    // It for reading CFieldRecord class data wih are an coda_array_class with fixed size and
    // contains CFieldBasic class data or CFieldArray class data
    // or CFieldRecord class data that contains CFieldbasic class data or CFieldArray class data or CFiedlRecord class data ...etc, etc.
    bool readParent = FindParentToRead(field, &parentFieldList);

    /*
    CField* parentField = m_tree.FindParent(field);
    if (parentField == NULL)
    {
      string msg = CTools::Format("CProduct::ReadBratFieldRecord - parentField of field '%s' is NULL - Read mustn't be called on root field",
                                       field->GetKey().c_str());
      CException e(msg, BRATHL_LOGIC_ERROR);
      throw (e);
    }

    CFieldRecord* parentFieldRecord = dynamic_cast<CFieldRecord*>(parentField);

    bool readParent =  (parentField->GetTypeClass() == coda_array_class)
                    && (parentField->IsFixedSize())
                    && (parentFieldRecord != NULL);
*/
    if (readParent)
    {
      //parentFieldRecord->PopCursor();
      //field->ReadParent(vect, parentFieldRecord);

      field->ReadParent(vect, &parentFieldList);
    }
    else
    {
      if (field->GetNativeType() == coda_native_type_string)
      {
        field->Read(value, skipRecord);
      }
      else
      {
        field->Read(vect, skipRecord);
      }
    }


    // date conversion for  coda_special_time data and if conversion is asked
    //if ( (field->GetSpecialType() == coda_special_time) && (field->GetConvertDate()) )
    // date conversion if conversion is asked
    if ( field->GetConvertDate() &&  field->UnitIsDate())
    {
      ConvertDate(vect);
    }

    CFieldSet *fieldSet = m_dataSet.GetFieldSet(field->GetKey());

    if (fieldSet == NULL)
    {
      fieldSet = field->CreateFieldSet(m_listFields);

      // If m_listFields contains only one item, there is no record, all fields are at the first level.
      // So recordset does not exist yet ==> create it
      if (m_dataSet.GetCurrentRecordSet() == NULL)
      {
        //InsertRecord(field->GetCurrentPos());
        InsertRecord(0);
      }

      m_dataSet.InsertFieldSet(field->GetKey(), fieldSet); //insert new CFieldSet in CRecordSet

    }

    if (field->GetNativeType() == coda_native_type_string)
    {
      fieldSet->Insert(value);
    }
    else
    {
      fieldSet->Insert(vect, true);
    }

    return;
  }


  if (m_dataSet.GetName().compare(field->GetName()) == 0) //reading a new item in brat dataset
  {
    InsertRecord(0);
    ReadBratFieldRecord(it, skipRecord);
  }
  else
  {
    ReadBratFieldRecord(it, skipRecord);
    if (! skipRecord)
    {
      field->PushPos();
      field->PopCursor();
    }
  }

}

//----------------------------------------
bool CProduct::FindParentToRead(CField* fromField, CObList* parentFieldList)
{
  return false;
}
/*
//----------------------------------------
void CProduct::ReadBratField(const string& key)
{

  m_listFields.RemoveAll();

  FillListFields(key);

  GetRootType();

  m_tree.GetRootData()->SetCursor(m_cursor);

  m_tree.SetWalkDownRootPivot();

  CField* field = NULL;
  bool bFoundField = false;

  do
  {
    bFoundField = false;
    bool bOk = m_tree.SubTreeWalkDown();
    if (bOk == false) // No more child
    {
      break;
    }

    field = m_tree.GetCurrentData();

    //if (field->GetName().compare(m_fieldsToProcess.front()) == 0)
    if (field->GetKey().compare(m_fieldsToProcess.front()) == 0)
    {
      bool hasDataset = true;
      if (IsHdf4() || IsNetCdf())
      {
        hasDataset = false;
      }
      bFoundField = true;
      m_listFields.InsertField(field, hasDataset);
      m_fieldsToProcess.pop_front();
    }
  }
  while (m_fieldsToProcess.empty() == false);

  if (bFoundField == false)
  {
    string msg = CTools::Format("ERROR - CProduct::ReadBratField() - Unknown dataset name or field name '%s'"
                                "(internal name searched:'%s'",
                                m_fieldsToProcess.front().c_str(),
                                key.c_str());
    CProductException e(msg, m_currFileName, GetProductClass(), GetProductType(), BRATHL_INCONSISTENCY_ERROR);
    CTrace::Tracer(e.what());
    //Dump(*CTrace::GetDumpContext());
    throw (e);
  }


  CField::CListField::iterator it = m_listFields.begin();

  // If m_listFields contains only one item, there is no record, all fields are at the first level.
  if (m_listFields.size() == 1)
  {
    m_dataSet.SetName("Dataset");
  }
  else
  {
    field = static_cast<CField*>(*(it));
    m_dataSet.SetName(field->GetName());
  }


  ReadBratField(it);

  return;

}
//----------------------------------------
void CProduct::ReadBratField(CField::CListField::iterator it)
{

  CField *field = static_cast<CField*>(*(it));


  SetCursor(field);

  it++;

  if (it == m_listFields.end())
  {
    CDoubleArray vect;
    field->Read(vect);
    // date conversion for  brat_special_time data
    if (field->GetSpecialType() == coda_special_time)
    {
      ConvertDate(vect);
    }

    CFieldSet *fieldSet = m_dataSet.GetFieldSet(field->GetKey());

    if (fieldSet == NULL)
    {
      fieldSet = field->CreateFieldSet(m_listFields);

      // If m_listFields contains only one item, there is no record, all fields are at the first level.
      // So recordset does not exist yet ==> create it
      if (m_dataSet.GetCurrentRecordSet() == NULL)
      {
        InsertRecord(field->GetCurrentPos());
      }

      m_dataSet.InsertFieldSet(field->GetKey(), fieldSet); //insert new CFieldSet in CRecordSet

    }

    fieldSet->Insert(vect);
    return;
  }

  do
  {
    if (m_dataSet.GetName().compare(field->GetName()) == 0) //reading a new item in brat dataset
    {
      InsertRecord(field->GetCurrentPos());
    }

    ReadBratField(it);
    field->PushPos();
    field->PopCursor();

  }while (field->End() == false);


  field->SetCurrentPos(0);

}
*/
//----------------------------------------
void CProduct::InsertRecord(int32_t pos)
{
  string recordSetName =  CTools::Format(20, "#%d", pos + m_recordCount + 1);

  //inserts a new recordset if not exists and sets the recordset as current recordset
  //if recordset already exists, only sets the recordset as current recordset
  m_dataSet.InsertRecord(recordSetName);
}
//----------------------------------------
void CProduct::InsertRecord(CDataSet& dataSet, int32_t pos)
{
  string recordSetName =  CTools::Format(20, "#%d", pos);

  //inserts a new recordset if not exists and sets the recordset as current recordset
  //if recordset already exists, only sets the recordset as current recordset
  dataSet.InsertRecord(recordSetName);
}
//----------------------------------------
void CProduct::ConvertDate(CDoubleArray& vect)
{
  CDate date;
  CDoubleArray::iterator it;

  for (it = vect.begin() ; it != vect.end() ; it++)
  {
    date.SetDate(*it, this->GetRefDate());
    *it = date.Value();
  }

}
//----------------------------------------
void CProduct::SetCursor(CField* field, bool& skipRecord)
{
  int32_t result = 0;

  if (skipRecord)
  {
    return;
  }

  if (typeid(*field) == typeid(CFieldIndex))
  {
    return;
  }

  if (typeid(*field) == typeid(CFieldIndexData))
  {
    return;
  }

  if (field == NULL)
  {
    CProductException e("CProduct::SetCursor - field is NULL", m_currFileName, GetProductClass(), GetProductType(), BRATHL_INCONSISTENCY_ERROR);
    CTrace::Tracer("%s", e.what());
    Dump(*CTrace::GetDumpContext());
    throw (e);
  }


  CField* parentField = m_tree.FindParent(field);
  if (parentField == NULL)
  {
    CProductException e("CProduct::SetCursor - parentField is NULL - SetCursor mustn't be called on root field", m_currFileName, GetProductClass(), GetProductType(), BRATHL_LOGIC_ERROR);
    CTrace::Tracer("%s", e.what());
    Dump(*CTrace::GetDumpContext());
    throw (e);

  }


  //ostream *fout = CTrace::GetDumpContext();

  //*fout << field->GetTypeClass() <<" " << field->GetCurrentPos() <<" " << field->GetName() <<" " << field->GetMaxPos() << endl;
  //*fout <<"its parent: " << parentField->GetTypeClass() <<" " << parentField->GetCurrentPos() <<" " << parentField->GetName() <<" " << parentField->GetMaxPos() << endl;


  //int32_t n_elements =0;
  //brat_cursor_get_num_elements(parentField->GetCursor(), &n_elements);
  //HandleBratError("brat_cursor_count_elements in CField::PushPos");

  //test the current type  cursor
  //if it's an array goto the first element before going to the record field
  //if (typeclass == brat_array_class) //test the type of the current cursor
  if (parentField->GetTypeClass() == coda_array_class) //test the type of the parent type class
  {
    result = coda_cursor_goto_array_element_by_index(parentField->GetCursor(), parentField->GetCurrentPos());
    HandleBratError("coda_cursor_goto_array_element_by_index");
    //result = brat_cursor_goto_first_array_element(&m_cursor);
    //HandleBratError("brat_cursor_goto_first_array_element");

    //brat_cursor_get_num_elements(parentField->GetCursor(), &n_elements);
    //HandleBratError("brat_cursor_count_elements in CField::PushPos");

  }

  //cout << "----------------- " << endl;
  //cout << "parentField->GetCurrentPos() " << parentField->GetName() << " - " << parentField->GetCurrentPos() << endl;
  //cout << "field->GetCurrentPos() " << field->GetName() << " - " << field->GetCurrentPos() << endl;
  //cout << "----------------- " << endl;

  //coda_Type *record_type;
  //result = coda_cursor_get_type(parentField->GetCursor(), &record_type);
  //HandleBratError("coda_cursor_get_type");

  if (field->IsUnion())
  {
      long indexUnion;
      result = coda_cursor_get_available_union_field_index(parentField->GetCursor(), &indexUnion);
      HandleBratError("coda_cursor_get_available_union_field_index");


      //const char *field_name;
      //result = coda_type_get_record_field_name(record_type, indexUnion, &field_name);
      //HandleBratError("coda_type_get_record_field_name");

      //cout << "parentField union index " << indexUnion << " field_name " << field_name << endl;
      if (indexUnion != field->GetIndex())
      {
        skipRecord = true;
        //cout << "skipRecord " << skipRecord << endl;
        return;
      }

      result = coda_cursor_goto_record_field_by_index(parentField->GetCursor(), indexUnion);
      HandleBratError("coda_cursor_goto_record_field_by_index");

  }
  else
  {
    result = coda_cursor_goto_record_field_by_name(parentField->GetCursor(), field->GetName().c_str());
    HandleBratError("coda_cursor_goto_record_field_by_name");
  }


  //brat_cursor_get_num_elements(parentField->GetCursor(), &n_elements);
  //HandleBratError("brat_cursor_count_elements in CField::PushPos");

  field->SetCursor(*(parentField->GetCursor()));

}


//----------------------------------------

string CProduct::GetTypeName()
{
  const char *str = NULL;
  coda_type_get_name(m_listInfo.Back()->m_type, &str);
  HandleBratError("coda_type_get_name");

  if (str == NULL)
  {
    return "";
  }

  return str;
}

//----------------------------------------

string CProduct::GetTypeDesc()
{
  return GetTypeDesc(m_listInfo.Back()->m_type);
}
//----------------------------------------

string CProduct::GetTypeDesc(coda_Type *type)
{
  const char *str = NULL;
  coda_type_get_description(type, &str);
  HandleBratError("coda_type_get_description");

  if (str == NULL)
  {
    return "";
  }

  return str;
}
//----------------------------------------

string CProduct::GetTypeUnit()
{
  const char *str = NULL;
  coda_type_get_unit(m_listInfo.Back()->m_type, &str);
  HandleBratError("coda_type_get_unit");

  if (str == NULL)
  {
    return "";
  }

  return str;
}


//----------------------------------------

string CProduct::GetRecordFieldName()
{
  CInfo* p = m_listInfo.PrevBack(false);

  if (p == NULL)
  {
    cerr << "WARNING - CProduct::GetRecordFieldName() - No record field name found" << endl;
    return "";
  }

  return p->m_fieldName;
}

//----------------------------------------

bool CProduct::TraverseData()
{
  // when returns, if bFieldAdded is true, Go one level up in the tree else stay on the node
  // bFieldAdded is false if we didn't added a node to the tree
  bool bFieldAdded = true;

  int32_t result = 0;

  result = coda_type_get_class(m_listInfo.Back()->m_type, &(m_listInfo.Back()->m_type_class));
  HandleBratError("coda_type_get_class");

  switch (m_listInfo.Back()->m_type_class)
  {
    //----------------------
    case coda_record_class:
    //----------------------
    {
      bFieldAdded = GetInfoRecord();
      break;
    }
    //----------------------
    case coda_array_class:
    //----------------------
    {
      bFieldAdded = GetInfoArray();
      break;
    }
    //----------------------
    case coda_integer_class:
    case coda_real_class:
    //----------------------
    {
      CFieldBasic *field = new CFieldBasic(-1,
                                           GetRecordFieldName(),
                                           GetTypeDesc(),
                                           GetTypeUnit());

      //If field has a specific unit, loads and sets it.
      SetFieldSpecificUnit(field);

      SetIndex(field);
      SetTypeClass(field);
      SetNativeType(field);

      field->SetDateRef(m_refDate);
      field->SetFieldHasDefaultValue(m_fieldsHaveDefaultValue);

      m_tree.AddChild(m_tree.GetWalkCurrent(), GetRecordFieldName(), field, true);

      SetHighResolution(field);

      break;
    }
    //----------------------
    case coda_text_class:
    //----------------------
    {
      long length;

      result = coda_type_get_string_length(m_listInfo.Back()->m_type, &length);
      HandleBratError("coda_type_get_string_length");


      CFieldBasic *field = new CFieldBasic(length,
                                           GetRecordFieldName(),
                                           GetTypeDesc(),
                                           GetTypeUnit());

      //If field has a specific unit, loads and sets it.
      SetFieldSpecificUnit(field);

      SetIndex(field);
      SetTypeClass(field);
      SetNativeType(field);

      field->SetDateRef(m_refDate);
      field->SetFieldHasDefaultValue(m_fieldsHaveDefaultValue);


      m_tree.AddChild(m_tree.GetWalkCurrent(), GetRecordFieldName(), field, true);

      SetHighResolution(field);

      break;
    }
    //----------------------
    case coda_raw_class:
    //----------------------
    {
      int64_t size;

      int32_t nbDims = 1;
      long dim[MAX_NUM_DIMS];

      result = coda_type_get_bit_size(m_listInfo.Back()->m_type, &size);
      HandleBratError("coda_type_get_bit_size");

      if (size >= 0)
      {
          dim[0] = static_cast<int>((size >> 3) + ((size & 0x7) != 0 ? 1 : 0));
      }
      else
      {
          dim[0] = -1;
      }


      CFieldArray* field = new CFieldArray(nbDims, dim, GetRecordFieldName(),GetTypeDesc(), GetTypeUnit());

      //If field has a specific unit, loads and sets it.
      SetFieldSpecificUnit(field);

      field->SetDateRef(m_refDate);
      field->SetFieldHasDefaultValue(m_fieldsHaveDefaultValue);

      SetIndex(field);
      SetTypeClass(field);
      SetNativeType(field);

      m_tree.AddChild(m_tree.GetWalkCurrent(),
                      GetRecordFieldName(),
                      field,
                      true);

      SetHighResolution(field);

      break;
    }
    //----------------------
    case coda_special_class:
    //----------------------
    {
      bFieldAdded = GetInfoSpecial();
      //bFieldAdded = false;
      break;
    }
    //----------------------
    default:
    //----------------------
    {
      string msg = CTools::Format("ERROR - CProduct::TraverseData() : unexpected Brat type class %d (%s) - FieldName:%s - File:%s",
                                  m_listInfo.Back()->m_type_class,
                                  coda_type_get_class_name(m_listInfo.Back()->m_type_class),
                                  GetRecordFieldName().c_str(),
                                  m_currFileName.c_str());
      CUnImplementException e(msg, BRATHL_UNIMPLEMENT_ERROR);
      throw(e);
      break;
    }

  } // end switch (m_type_class)

  return bFieldAdded;
}

//----------------------------------------
void CProduct::SetHighResolution(CField* field)
{
  if (field == NULL)
  {
    return;
  }

  field->SetHighResolution(this->IsHighResolutionField(field));
  field->SetNumHighResolutionMeasure(this->m_numHighResolutionMeasure);
}
//----------------------------------------

bool CProduct::GetInfoArray()
{
  int32_t result = 0;

  int32_t nbDims = 0;
  long dim[MAX_NUM_DIMS];

  //bool isFixedSize = true;

  memset(&dim, 0, sizeof(dim));

  bool bFieldAdded = true;

  string fieldName = GetRecordFieldName();

  // Get dimensions for fixed sized array
  result = coda_type_get_array_dim(m_listInfo.Back()->m_type, &nbDims, dim);
  HandleBratError("coda_type_get_array_dim");

  //--------------------------
  coda_Type* typeBckup = m_listInfo.Back()->m_type;
  //--------------------------

  result = coda_type_get_array_base_type(typeBckup, &(m_listInfo.Back()->m_type));
  HandleBratError("coda_type_get_array_base_type");

  coda_type_class type_class;

  result = coda_type_get_class(m_listInfo.Back()->m_type, &type_class);
  HandleBratError("coda_type_get_class");


  switch (type_class)
  {
    //----------------------
    case coda_record_class:
    //----------------------
    {
      bFieldAdded = GetInfoRecord(nbDims, dim);
      break;
    }
    //----------------------
    case coda_special_class:
    //----------------------
    {
      bFieldAdded = GetInfoSpecial(nbDims, dim);
      break;
    }
    //----------------------
    case coda_integer_class:
    case coda_real_class:
    //----------------------
    {
      string description = CTools::Format("%s\n%s",
                                          GetTypeDesc(typeBckup).c_str(),
                                          GetTypeDesc().c_str());
      CFieldArray* field = new CFieldArray(nbDims, dim, fieldName, description, GetTypeUnit());

      //If field has a specific unit, loads and sets it.
      SetFieldSpecificUnit(field);

      SetIndex(field);
      SetTypeClass(field);
      SetNativeType(field);

      bool transpose = m_fieldsToTranspose.Exists(fieldName, true);

      if (transpose)
      {
        field->TransposeDim();
      }

      field->SetDateRef(m_refDate);
      field->SetFieldHasDefaultValue(m_fieldsHaveDefaultValue);



      m_tree.AddChild(m_tree.GetWalkCurrent(), fieldName,
                      field,
                      true);

      SetHighResolution(field);


      CreateFieldIndexes(field);


      break;
    }
    //----------------------
    case coda_raw_class:
    //----------------------
    {
      int64_t size;

      result = coda_type_get_bit_size(m_listInfo.Back()->m_type, &size);
      HandleBratError("coda_type_get_bit_size");

      if (size >= 0)
      {
          dim[nbDims] = static_cast<int>((size >> 3) + ((size & 0x7) != 0 ? 1 : 0));
          nbDims++;
      }
      else
      {
          dim[nbDims] = -1;
      }


      CFieldArray* field = new CFieldArray(nbDims, dim, GetRecordFieldName(),GetTypeDesc(), GetTypeUnit());

      //If field has a specific unit, loads and sets it.
      SetFieldSpecificUnit(field);

      field->SetDateRef(m_refDate);
      field->SetFieldHasDefaultValue(m_fieldsHaveDefaultValue);

      SetIndex(field);
      SetTypeClass(field);
      SetNativeType(field);

      m_tree.AddChild(m_tree.GetWalkCurrent(),
                      GetRecordFieldName(),
                      field,
                      true);

      SetHighResolution(field);

      break;
    }

    //----------------------
    default:
    //----------------------
    {
      string msg = CTools::Format("ERROR - CProduct::GetInfoArray() : unexpected Brat type class %d (%s) - FieldName:%s - File:%s",
                                  m_listInfo.Back()->m_type_class,
                                  coda_type_get_class_name(m_listInfo.Back()->m_type_class),
                                  GetRecordFieldName().c_str(),
                                  m_currFileName.c_str());
      CUnImplementException e(msg, BRATHL_UNIMPLEMENT_ERROR);
      throw(e);

      break;
    }

  }

  return bFieldAdded;

}
//----------------------------------------
void CProduct::CreateFieldIndexes(CFieldArray* field)
{
  if (!m_createVirtualField)
  {
    return;
  }

  CObArray fieldIndexes(false);
  field->CreateFieldIndexes(fieldIndexes);

  CObArray::const_iterator it;


  for (it = fieldIndexes.begin() ; it != fieldIndexes.end() ; it++)
  {
    CFieldIndex* f = dynamic_cast<CFieldIndex*>(*it);
    if (f != NULL)
    {
      m_tree.GoLevelUp();
      m_tree.AddChild(m_tree.GetWalkCurrent(), f->GetName(),
                      f,
                      true);
    }
  }


}

//----------------------------------------
void CProduct::CreateFieldIndexData()
{

  string name = CField::m_BRAT_INDEX_DATA_NAME;
  string description = CField::m_BRAT_INDEX_DATA_DESC;
  CFieldIndexData* field = new CFieldIndexData(name, description);
  
  field->SetTypeClass(coda_real_class);
  field->SetNativeType(coda_native_type_double);
  field->SetOffset(m_offset);

  m_tree.AddChild(m_tree.GetWalkCurrent(), field->GetName(),
                  field,
                  false);

}
//----------------------------------------
bool CProduct::GetInfoSpecial(int32_t nbDims, const long dim[])
{
  int32_t result = 0;

  bool bFieldAdded = true;

  coda_special_type special_type;

  result = coda_type_get_special_type(m_listInfo.Back()->m_type, &special_type);
  HandleBratError("coda_type_get_special_type");


  switch (special_type)
  {
    //----------------------
    case coda_special_no_data: // ignore
    //----------------------
      bFieldAdded = false;
      break;
    //----------------------
    case coda_special_time:
    //----------------------
    {
      CFieldBasic *field = new CFieldBasic(-1,
                                           GetRecordFieldName(),
                                           GetTypeDesc(),
                                           GetTypeUnit());

      //If field has a specific unit, loads and sets it.
      SetFieldSpecificUnit(field);

      SetIndex(field);
      SetTypeClass(field);
      SetSpecialType(field);

      field->SetDateRef(m_refDate);
      field->SetFieldHasDefaultValue(m_fieldsHaveDefaultValue);

      m_tree.AddChild(m_tree.GetWalkCurrent(), GetRecordFieldName(), field, true);

      SetHighResolution(field);

      break;
     }
    //----------------------
    case coda_special_complex:
    case coda_special_vsf_integer:
    //----------------------
    {
      string msg = CTools::Format("ERROR - CProduct::GetInfoSpecial() : unexpected Brat special type %d (%s) -  FieldName:%s - File:%s",
                                  special_type,
                                  coda_type_get_special_type_name(special_type),
                                  GetRecordFieldName().c_str(),
                                  m_currFileName.c_str());
      CUnImplementException e(msg, BRATHL_UNIMPLEMENT_ERROR);
      throw(e);
      break;
    }
  }


  return bFieldAdded;

}


//----------------------------------------

bool CProduct::GetInfoRecord(int32_t nbDims, const long dim[])
{
  int32_t result = 0;

  long num_fields;

  result = coda_type_get_num_record_fields(m_listInfo.Back()->m_type, &num_fields);
  HandleBratError("coda_type_get_num_record_fields");

  string recordFieldName;

  if (m_tree.GetRoot() == NULL)
  {
    recordFieldName = m_treeRootName;
  }
  else
  {
    recordFieldName = GetRecordFieldName();
  }

  CFieldRecord *field = new CFieldRecord(nbDims, dim, num_fields, recordFieldName, GetTypeDesc(), GetTypeUnit());

  //If field has a specific unit, loads and sets it.
  SetFieldSpecificUnit(field);

  field->SetDateRef(m_refDate);
  field->SetFieldHasDefaultValue(m_fieldsHaveDefaultValue);

  SetIndex(field);
  SetTypeClass(field);
  SetUnion(field);

  if (m_tree.GetRoot() == NULL)
  {
    string desc = GetProductClass() + "/" + GetProductType();
    field->SetDescription(desc);

    m_tree.SetRoot(m_treeRootName, field, true);
  }
  else
  {
    m_tree.AddChild(m_tree.GetWalkCurrent(), GetRecordFieldName(),
                    field,
                    true);
  }

  if (!field->IsFixedSize())
  {
    CreateFieldIndexData();      
  }


  for (int32_t index = 0; index < num_fields; index++)
  {
    TraverseRecord(index);
  }

  return true;

}

//----------------------------------------

bool CProduct::TraverseRecord(int32_t indexFields)
{
  int32_t hidden;
  int32_t result;
  const char * name;

  result = coda_type_get_record_field_hidden_status(m_listInfo.Back()->m_type, indexFields, &hidden);
  HandleBratError("coda_type_get_record_field_hidden_status");

  if (hidden)
  {
      // skip this field
      return false;
  }

  int32_t available;

  // we do not traverse records that are globally not available
  // (i.e. not available for every element of our parent array(s))
  result = coda_type_get_record_field_available_status(m_listInfo.Back()->m_type, indexFields, &available);
  HandleBratError("coda_type_get_record_field_available_status");

  /*
  if (available == -1)
  {
      // traverse all occurences of this field to check whether at least one is available
      // find out whether there is at least one occurence where this field is available
      //if (!get_record_field_available_status(0, 0, 0))
     // {
     //     return;
      //}
    cerr << "WARNING get_record_field_available_status " << endl;
  }
  */

  result = coda_type_get_record_field_name(m_listInfo.Back()->m_type, indexFields, &name);
  HandleBratError("coda_type_get_record_field_name");


  CInfo* p = m_listInfo.Back();
  p->m_fieldName = name;

  //--------------------------
  coda_Type* typeBckup = m_listInfo.Back()->m_type;
  //--------------------------

  p = m_listInfo.AddNew(); // add a new CInfo Object to the list

  p->m_index = indexFields;

  result = coda_type_get_record_field_type(typeBckup, indexFields, &(m_listInfo.Back()->m_type));
  HandleBratError("coda_type_get_record_field_type");

  result = coda_type_get_record_union_status(typeBckup, &(m_listInfo.Back()->m_isUnion));
  HandleBratError("coda_type_get_record_union_status");

  bool bGoUp = TraverseData();

  m_listInfo.PopBack(); // remove last CInfo object


  if (bGoUp)
  {
    m_tree.GoLevelUp(); // Go one level up
  }

  return true;

}


//----------------------------------------

void CProduct::SetTypeClass(CField* field)
{
  field->SetTypeClass(m_listInfo.Back()->m_type_class);

}

//----------------------------------------
void CProduct::SetIndex(CField* field)
{
  field->SetIndex(m_listInfo.Back()->m_index);
}
//----------------------------------------
void CProduct::SetUnion(CField* field)
{
  field->SetUnion(m_listInfo.Back()->m_isUnion);
}



//----------------------------------------

void CProduct::SetNativeType(CField* field)
{
  int32_t result = 0;

  coda_native_type read_type;

  result = coda_type_get_read_type(m_listInfo.Back()->m_type, &read_type);
  HandleBratError("coda_type_get_read_type");

  field->SetNativeType(read_type);

}

//----------------------------------------
void CProduct::SetSpecialType(CField* field)
{
  int32_t result = 0;

  coda_special_type special_type;

  result = coda_type_get_special_type(m_listInfo.Back()->m_type, &special_type);
  HandleBratError("coda_type_get_special_type");

  field->SetSpecialType(special_type);

}


//----------------------------------------
void CProduct::GetRecords(CStringArray& array)
{
  string recordName;

  if (m_tree.GetRoot() == NULL)
  {
    return;
  }

  m_tree.SetWalkDownRootPivot();

  do
  {
    CField *field  = dynamic_cast<CField*>(m_tree.GetWalkCurrent()->GetData());
    if (field == NULL)
    {
      CProductException e("ERROR in CProduct::GetRecords - at least one of the tree object is not a CField object", m_currFileName, GetProductClass(), GetProductType(), BRATHL_INCONSISTENCY_ERROR);
      CTrace::Tracer("%s", e.what());
      //Dump(*CTrace::GetDumpContext());
      throw (e);
    }

    recordName = field->GetRecordName();

    if (recordName.empty() == false)
    {
      array.InsertUnique(recordName);
    }

  }
  while (m_tree.SubTreeWalkDown());
}


//----------------------------------------
bool CProduct::LoadTransposeFieldsValue(CStringArray& fieldToTranspose)
{

  const uint32_t MAX_LINE_LEN = 255;
  char line[MAX_LINE_LEN+1];

  char product[MAX_LINE_LEN+1];
  char field[MAX_LINE_LEN+1];

  fieldToTranspose.RemoveAll();


  string refFilePathName = CTools::FindDataFile(CProduct::m_transposeFieldValuesFileName);
  if (refFilePathName == "")
  {
    return false;
  }


  // reads file contains value reference
  CFile fileRef(refFilePathName, CFile::modeRead);

  if (fileRef.IsOpen() == false)
  {
    return false;
  }

  int32_t nbFields = EOF;

  int32_t size = fileRef.ReadLineData(line, MAX_LINE_LEN);

  while (size > 0)
  {
    nbFields = sscanf (line, "%s %s",
      	      	      	    product,
      	      	      	    field);

    if ( (nbFields < 2) )
    {
      continue;
    }

    /*printf("name %s, alias %s\n",
	      	      	    name,
      	      	      	    alias);
    */

    if (CTools::CompareNoCase(product, GetProductClassType().c_str()))
    {
      fieldToTranspose.InsertUnique(CTools::StringTrim(field));
    }

    // reads next data
    size = fileRef.ReadLineData(line, MAX_LINE_LEN);

  }

  fileRef.Close();

  return true;
}

//----------------------------------------
string CProduct::GetProductClassType()
{
  string str;

  str.append(GetProductClass());
  str.append("_");
  str.append(GetProductType());

  return str;
}
/*
//----------------------------------------
void CProduct::GetHighResolutionFieldCalculation(CStringList& listField)
{

  if (HasHighResolutionFieldCalculation() == false)
  {
    return;
  }

  listField.InsertUnique(m_highResolutionLatDiffFieldName);
  listField.InsertUnique(m_highResolutionLonDiffFieldName);
}
//----------------------------------------
void CProduct::GetInternalHighResolutionFieldCalculation(CStringList& listField)
{

  if (HasHighResolutionFieldCalculation() == false)
  {
    return;
  }

  listField.InsertUnique( MakeInternalFieldName(m_highResolutionLatDiffFieldName) );
  listField.InsertUnique( MakeInternalFieldName(m_highResolutionLonDiffFieldName) );
}
//----------------------------------------
void CProduct::AddHighResolutionFieldCalculation(CStringList& listField)
{
  GetHighResolutionFieldCalculation(listField);
}
//----------------------------------------
void CProduct::AddInternalHighResolutionFieldCalculation(CStringList& listField)
{
  GetInternalHighResolutionFieldCalculation(listField);
}
*/
//----------------------------------------
void CProduct::SetFieldSpecificUnit(CField* field)
{

  if (field == NULL)
  {
    return;
  }

  //If field has a specific unit, loads and sets it.
  string specificUnit = GetFieldSpecificUnit(field->GetFullName());
  if (!specificUnit.empty())
  {
    field->SetUnit(specificUnit);
  }

}
//----------------------------------------
void CProduct::SetFieldSpecificUnits(const CStringMap& fieldSpecificUnit)
{
  m_fieldSpecificUnit.RemoveAll();
  m_fieldSpecificUnit.Insert(fieldSpecificUnit, false);
}

//----------------------------------------
string CProduct::GetFieldSpecificUnit(const string& key)
{
  return m_fieldSpecificUnit.Exists(key);
}
//----------------------------------------
void CProduct::SetFieldSpecificUnit(const string& key, const string& value)
{
  m_fieldSpecificUnit.Erase(key);
  m_fieldSpecificUnit.Insert(key, value, false);

}

//----------------------------------------
void CProduct::SetListFieldOrigin(const CStringList& listFieldOrigin)
{
  m_listFieldOrigin.RemoveAll();
  m_listFieldOrigin.Insert(listFieldOrigin);
}

//----------------------------------------
int32_t CProduct::ReadData( 
	int32_t	nbFiles,
	char		**fileNames,
	const char	*recordName,
	const char	*selection,
	int32_t	nbData,
	char		**dataExpressions,
	char		**units,
	double		**results,
	int32_t	sizes[],
	int32_t	*actualSize,
	int		ignoreOutOfRange,
	int		statistics,
	double		defaultValue,
	CStringMap* fieldSpecificUnit /*  = NULL */
	)
{

	CBratAlgorithmBase::RegisterAlgorithms();

	// Load aliases dictionnary
	string errorMsg;
	CAliasesDictionary::LoadAliasesDictionary( &errorMsg, false );
	if ( !( errorMsg.empty() ) )
	{
		std::cerr << "WARNING: " << errorMsg << std::endl;
	}


	CProduct	*product	= NULL;

	int32_t	index;

	int32_t brathl_errno = BRATHL_SUCCESS;

	string	strRecordName( recordName );

	try
	{
		CExpression			select( CTools::IsEmpty( selection ) ? "1" : selection );
		vector<CExpression>		expressions;
		CUIntArray		Positions;
		CStringList			listFieldsToRead;
		vector<CUnit>		wantedUnits;

		CStringArray		FileList;

		if ( ignoreOutOfRange && statistics )
		{
			throw CException( "brathl_ReadData: Cannot ignore out of range when doing statistics",
				BRATHL_INCONSISTENCY_ERROR );
		}

		// Build file list
		for ( index=0; index < nbFiles; index++ )
		{
			if ( ! CTools::IsEmpty( fileNames[ index ] ) )
			{
				FileList.Insert( fileNames[ index ] );
			}
		}

		product	= CProduct::Construct( FileList );

		// Check expressions
		for ( index=0; index < nbData; index++ )
		{
			CExpression	expr;
			CUnit	unit;

			if ( CTools::IsEmpty( dataExpressions[ index ] ) )
			{
				expr.SetExpression( "DV" );
			}
			else
			{
				string str = CTools::ExpandVariables( dataExpressions[ index ], product->GetAliasesAsString(), NULL, true, '%', NULL, true, NULL );

				expr.SetExpression( str );
				listFieldsToRead.InsertUnique( expr.GetFieldNames() );
			}

			expressions.push_back( expr );

			if ( ( units == NULL ) || CTools::IsEmpty( units[ index ] ) )
			{
				unit	= "count";
			}
			else
			{
				unit	= units[ index ];
				unit.SetConversionFrom( unit.BaseUnit() );
			}

			wantedUnits.push_back( unit );

			if ( results == NULL )
			{
				if ( sizes != NULL )
				{
					throw CException( "brathl_ReadData: if 'results' is NULL, 'sizes' must also be NULL",
						BRATHL_INCONSISTENCY_ERROR );
				}
			}
			else
			{
				if ( sizes == NULL )
				{
					throw CException( "brathl_ReadData: if 'results' is not NULL, 'sizes' must also not be NULL",
						BRATHL_INCONSISTENCY_ERROR );
				}
				if ( sizes[ index ] != 0 )
				{
					if ( sizes[ index ] < 0 )
					{
						if ( results[ index ] != NULL )
							throw CException( "brathl_ReadData: if 'sizes' is negative, corresponding 'results' must be NULL",
							BRATHL_INCONSISTENCY_ERROR );
						results[ index ]	= static_cast<double *>( malloc( 4096 * sizeof( *( results[ index ] ) ) ) );
						if ( results[ index ] == NULL )
							throw CMemoryException( "brathl_ReadData: not enough memory for initial vectors" );
						sizes[ index ]	= -4096;
					}
					else if ( ( results[ index ] == NULL ) && ( sizes[ index ] > 0 ) )
					{
						throw CException( "brathl_ReadData: if 'sizes' is positive, corresponding 'results' must not be NULL",
							BRATHL_INCONSISTENCY_ERROR );
					}
					else if ( ( sizes[ index ] < NUMBER_OF_STATISTICS ) && statistics )
					{
						throw CException( CTools::Format( "brathl_ReadData: when statistics are asked, size of result must be at least %d, not %d",
							NUMBER_OF_STATISTICS,
							sizes[ index ] ),
							BRATHL_INCONSISTENCY_ERROR );
					}
					// Initialises statistics
					if ( statistics )
					{
						for ( int StatIndex=0; StatIndex < NUMBER_OF_STATISTICS; StatIndex++ )
						{
							results[ index ][ StatIndex ]	= CTools::m_defaultValueDOUBLE;
						}
						results[ index ][ COUNT_INDEX ]	= 0.0;
					}
				}
			}
		}

		listFieldsToRead.InsertUnique( select.GetFieldNames() );

		*actualSize	= 0;

		uint32_t nbFiles = FileList.size();
		uint32_t cptFile = 0;

		product->SetExpandArray( true );

		for ( CStringArray::iterator itFile = FileList.begin(); itFile != FileList.end(); itFile++ )
		{
			cptFile++;

			CTrace::Tracer( 1, "File %d/%d - Reading record data from %s ... and registering data ...",
				cptFile, nbFiles, ( *itFile ).c_str() );


			product->SetForceReadDataOneByOne( true );

			if ( fieldSpecificUnit != NULL )
			{
				product->SetFieldSpecificUnits( *fieldSpecificUnit );
			}

			product->Open( *itFile, strRecordName, listFieldsToRead );


			uint32_t nRecords = product->GetNumberOfRecords();


			for ( uint32_t iRecord = 0; iRecord < nRecords; iRecord++ )
			{
				CDataSet* dataSet = NULL;
				product->ReadBratRecord( iRecord );
				dataSet = product->GetDataSet();

				CProduct::ReadDataForOneMeasure( dataSet, strRecordName, select, expressions, wantedUnits, results, sizes, actualSize, ignoreOutOfRange, statistics, product );
			}

			product->Close();
		}

		if ( statistics )
		{
			*actualSize	= CProduct::NUMBER_OF_STATISTICS;
		}
		if ( sizes != NULL )
		{
			// Set expandable vectors to fixed (final) size
			for ( index=0; index < nbData; index++ )
			{
				double *vector	= results[ index ];
				sizes[ index ]	= abs( sizes[ index ] );
				// Finalize statistics
				if ( statistics && ( sizes[ index ] != 0 ) && ( vector[ CProduct::COUNT_INDEX ] != 0.0 ) )
				{// Compute final STDDEV
					CTools::FinalizeIncrementalStats( vector[ CProduct::COUNT_INDEX ],
						vector[ CProduct::MEAN_INDEX ],
						vector[ CProduct::STDDEV_INDEX ],
						vector[ CProduct::MIN_INDEX ],
						vector[ CProduct::MAX_INDEX ] );
				}
				else
				{
					for ( int32_t IndexVal=0; IndexVal < *actualSize; IndexVal++ )
					{
						if ( CTools::IsDefaultValue( vector[ IndexVal ] ) )
						{
							vector[ IndexVal ]	= defaultValue;
						}
					}
				}
			}
		}
	}
	catch ( CException &e )
	{
		brathl_errno = e.error();
		cerr << "ERROR brathl_ReadData:" << e.what() << endl;
	}
	catch ( ... )
	{
		brathl_errno = BRATHL_ERROR;
	}

	// Free resources
	try
	{
		delete product;
	}
	catch ( CException &e )
	{
		brathl_errno = e.error();
		cerr << "ERROR brathl_ReadData:" << e.what() << endl;
	}
	catch ( ... )
	{
		brathl_errno = BRATHL_ERROR;
	}

	return brathl_errno;
}
//----------------------------------------

void CProduct::ReadDataForOneMeasure(
	CDataSet			*dataSet,
	const string			&recordName,
	CExpression			&select,
	vector<CExpression>		&expressions,
	const vector<CUnit>		&wantedUnits,
	double				**results,
	int32_t			*sizes,
	int32_t			*actualSize,
	int				ignoreOutOfRange,
	int				statistics,
	CProduct* product /* = NULL */ )
{
	CExpressionValue	exprValue;


	for ( CObArray::iterator itDataSet = dataSet->begin(); itDataSet != dataSet->end(); itDataSet++ )
	{
		CRecord	*record		= dynamic_cast<CRecord*>( *itDataSet );
		CRecordSet	*recordSet	= record->GetRecordSet();

		recordSet->ExecuteExpression( select, recordName, exprValue, product );
		if ( exprValue.IsTrue() != 1 )
		{
			continue;	// Not selected
		}

		if ( results != NULL )
		{
			for ( uint32_t indexExpr = 0; indexExpr < expressions.size(); indexExpr++ )
			{
				recordSet->ExecuteExpression( expressions[ indexExpr ], recordName, exprValue, product );

				int32_t nbValues	= exprValue.GetNbValues();

				double value;

				if ( nbValues == 0 )
				{
					value	= CTools::m_defaultValueDOUBLE;
				}
				else if ( nbValues == 1 )
				{
					value	= exprValue.GetValues()[ 0 ];
				}
				else
				{
					throw CException( "Field value must be scalar not a vector or a matrix",
						BRATHL_LIMIT_ERROR );
				}

				CUnit unit = wantedUnits[ indexExpr ];
				double convertedValue = unit.Convert( value );

				if ( ( sizes[ indexExpr ] >= 0 ) && ( *actualSize >= sizes[ indexExpr ] ) )
				{
					if ( ! ( ( sizes[ indexExpr ] == 0 ) || ignoreOutOfRange ) )
					{
						if ( !statistics )
						{
							throw CException( CTools::Format( "Too much data to store for given space at least %d values needed",
								*actualSize ),
								BRATHL_RANGE_ERROR );
						}
						else
						{
							double *vector	= results[ indexExpr ];
							CTools::DoIncrementalStats( convertedValue,
								vector[ COUNT_INDEX ],
								vector[ MEAN_INDEX ],
								vector[ STDDEV_INDEX ],
								vector[ MIN_INDEX ],
								vector[ MAX_INDEX ] );
						}
					}
				}
				else
				{
					if ( ( sizes[ indexExpr ] < 0 ) && ( -sizes[ indexExpr ] <= *actualSize ) )
					{// Size of expandable vector reached, resize it by doubling its size
						sizes[ indexExpr ]	*= 2;
						void 	*NewPtr	= realloc( results[ indexExpr ],
							-sizes[ indexExpr ] * sizeof( *( results[ indexExpr ] ) ) );
						if ( NewPtr == NULL )
							throw CMemoryException( CTools::Format( "ReadDataForOneMeasure: Not enough memory to allocate vector of size %d",
							-sizes[ indexExpr ] ) );

						results[ indexExpr ]	= static_cast<double *>( NewPtr );
						memset( &results[ indexExpr ][ abs( sizes[ indexExpr ] ) / 2 ], 0, ( abs( sizes[ indexExpr ] ) / 2 ) * ( sizeof( *( results[ indexExpr ] ) ) ) );
					}
					if ( statistics )
					{
						double *vector	= results[ indexExpr ];
						CTools::DoIncrementalStats( convertedValue,
							vector[ COUNT_INDEX ],
							vector[ MEAN_INDEX ],
							vector[ STDDEV_INDEX ],
							vector[ MIN_INDEX ],
							vector[ MAX_INDEX ] );

					}
					else
					{
						results[ indexExpr ][ *actualSize ]	= convertedValue;
					}
				}
			}
		}
		( *actualSize )++;
	}
}
//----------------------------------------
CProduct* CProduct::Clone()
{
  CProduct* product = CProduct::Construct(this->GetProductList());

  if (product == NULL)
  {
    return product;
  }

  product->SetFieldSpecificUnits(*(this->GetFieldSpecificUnits()));
  product->SetCreateVirtualField(this->GetCreateVirtualField());
  product->SetForceReadDataOneByOne(this->GetForceReadDataOneByOne());
  product->SetExpandArray(this->GetExpandArray());
  product->SetDataSetNameToRead(this->GetDataSetNameToRead());
  product->SetListFieldOrigin(*(this->GetListFieldOrigin()));

  return product;
}


//----------------------------------------
void CProduct::AddSameFieldName(const string& fieldNameToSearch, CStringArray& arrayFieldsAdded)
{
  string internalFieldName;

  CField* fieldTest = NULL;

  GetDataDictionaryFieldNames();

  CIntArray indexes;
  m_dataDictionaryFieldNames.FindIndexes(fieldNameToSearch, indexes, true);

  if (indexes.size() > 1)
  {
    CStringArray possibleFieldNames;
    m_dataDictionaryFieldNamesWithDatasetName.GetValues(indexes, possibleFieldNames);
    CStringArray::const_iterator it;
    for (it = possibleFieldNames.begin() ; it != possibleFieldNames.end() ; it++)
    {
      fieldTest = FindFieldByName(*it, false, NULL, false);
      if (fieldTest == NULL)
      {
        continue;
      }

      internalFieldName = MakeInternalNameByAddingRoot(fieldTest->GetFullNameWithRecord());

      bool exists = m_listInternalFieldName.Exists(internalFieldName);

      if (!exists)
      {
        continue;
      }

      arrayFieldsAdded.InsertUnique(*it);

    }
  }
}
//----------------------------------------

void CProduct::DumpDictionary(ostream& fOut /* = cout */)
{
  m_tree.DumpDictionary(fOut);
}
//----------------------------------------

void CProduct::DumpDictionary(const string& outputFileName)
{
  m_tree.DumpDictionary(outputFileName);
}


//----------------------------------------
void CProduct::Dump(ostream& fOut /* = cerr */)
{
  if (CTrace::IsTrace() == false)
  {
    return;
  }


  fOut << "==> Dump a CProduct Object at "<< this << endl;


  fOut << "m_currFileName = " << m_currFileName << endl;

  fOut << "m_fileList = " << endl;
  m_fileList.Dump(fOut);

  fOut << "m_tree = " << endl;
  m_tree.Dump(fOut);

  if (m_productAliases != NULL)
  {
    fOut << "m_productAliases = " << endl;
    m_productAliases->Dump(fOut);
  }
  else
  {
    fOut << "m_productAliases = NULL" << endl;
  }
  
  fOut << "==> END Dump a CProduct Object at "<< this << endl;

  fOut << endl;

}

//-------------------------------------------------------------
//------------------- CInfo class --------------------
//-------------------------------------------------------------


CProduct::CInfo::CInfo()
{
  m_type = NULL;
  m_index = 0;
  m_fieldName = "";
  m_isUnion = 0;

}
//----------------------------------------

CProduct::CInfo::~CInfo()
{

}

//-------------------------------------------------------------
//------------------- CListInfo class --------------------
//-------------------------------------------------------------

CProduct::CInfo* CProduct::CListInfo::AddNew()
{
  CInfo* p = new CInfo;
  Insert(p);

  return p;

}
//----------------------------------------

CProduct::CInfo* CProduct::CListInfo::Back(bool withExcept)
{

  if (this->empty())
  {
    if (withExcept)
    {
      CException e("ERROR in CProduct::CListInfo::Back() - List is empty", BRATHL_LOGIC_ERROR);
      throw(e);
    }
    else
    {
      return NULL;
    }
  }

  CInfo* p = static_cast<CInfo *>(CObList::back());
  if ((p == NULL) && (withExcept))
  {
    CException e("ERROR in CProduct::CListInfo::Back() returns NULL pointer - List seems to be empty", BRATHL_LOGIC_ERROR);
    throw(e);
  }
  return p;
}

//----------------------------------------

CProduct::CInfo* CProduct::CListInfo::Front(bool withExcept)
{
  if (this->empty())
  {
    if (withExcept)
    {
      CException e("ERROR in CProduct::CListInfo::Front() - List is empty", BRATHL_LOGIC_ERROR);
      throw(e);
    }
    else
    {
      return NULL;
    }
  }

  CProduct::CInfo* p = static_cast<CProduct::CInfo *>(CObList::front());
  if ((p == NULL) && (withExcept))
  {
    CException e("ERROR in CProduct::CListInfo::Front() returns NULL pointer - List seems to be empty", BRATHL_LOGIC_ERROR);
    throw(e);
  }
  return p;
}
//----------------------------------------

CProduct::CInfo* CProduct::CListInfo::PrevBack(bool withExcept)
{
  if (this->size() <= 1)
  {
    if (withExcept)
    {
      CException e("ERROR in CProduct::CListInfo::PrevBack() - List has only one element", BRATHL_LOGIC_ERROR);
      throw(e);
    }
    else
    {
      return NULL;
    }
  }

  CObList::iterator it = --end(); //  --end() --> iterator of the last element
  it--;  // it --> the next to last element of the list

  CInfo* p = static_cast<CProduct::CInfo *>(*it);
  if ((p == NULL) && (withExcept))
  {
    CException e("ERROR in CProduct::CListInfo::PrevBack() returns NULL pointer", BRATHL_LOGIC_ERROR);
    throw(e);
  }
  return p;
}

//----------------------------------------



//-------------------------------------------------------------
//------------------- CMapProduct class --------------------
//-------------------------------------------------------------

CMapProduct::CMapProduct()
{
  Init();


}


//----------------------------------------
CMapProduct::~CMapProduct()
{
}

//----------------------------------------
CMapProduct& CMapProduct::GetInstance()
{
  static CMapProduct instance;

  return instance;
}
//----------------------------------------
void CMapProduct::Init()
{

  CProduct* product = NULL;

  product = new CProductAop();
  Insert(product->GetLabel(), product);

  product = new CProductCryosat();
  Insert(product->GetLabel(), product);

  product = new CProductEnvisat();
  Insert(product->GetLabel(), product);

  product = new CProductErs();
  Insert(product->GetLabel(), product);

  product = new CProductGfo();
  Insert(product->GetLabel(), product);

  product = new CProductJason();
  Insert(product->GetLabel(), product);

  product = new CProductJason2();
  Insert(product->GetLabel(), product);

  product = new CProductPodaac();
  product->GetProductList().m_productType = CProductPodaac::m_J1SSHA_PASS_FILE;
  product->InitCriteriaInfo();
  Insert(product->GetLabel(), product);

  product = new CProductPodaac();
  product->GetProductList().m_productType = CProductPodaac::m_J1SSHA_ATG_FILE;
  product->InitCriteriaInfo();
  Insert(product->GetLabel(), product);

  product = new CProductRads();
  Insert(product->GetLabel(), product);

  product = new CProductRiverLake();
  Insert(product->GetLabel(), product);

  product = new CProductTopex();
  product->GetProductList().m_productType = CProductTopex::m_PASS_FILE;
  product->InitCriteriaInfo();
  Insert(product->GetLabel(), product);

  product = new CProductTopex();
  product->GetProductList().m_productType = CProductTopex::m_XNG_FILE;
  product->InitCriteriaInfo();
  Insert(product->GetLabel(), product);

  product = new CProductTopexSDR();
  product->GetProductList().m_productType = CProductTopexSDR::m_SDR_PASS_FILE;
  product->InitCriteriaInfo();
  Insert(product->GetLabel(), product);

  product = new CProductNetCdf();
  Insert(product->GetLabel(), product);


}

//----------------------------------------
void CMapProduct::AddCriteriaToProducts()
{

  CObMap::iterator it;
  for (it = begin() ; it != end() ; it++)
  {
    CProduct* product = dynamic_cast<CProduct*>(it->second);
    if (product == NULL)
    {
      continue;
    }

    bool force = false;

    CProductNetCdf* productNetCdf = dynamic_cast<CProductNetCdf*>(it->second);
    if (productNetCdf != NULL)
    {
      if (typeid(*productNetCdf) != typeid(CProductJason2))
      {
        force = true;
      }
    }

    product->AddCriteria(force);

  }
}

//----------------------------------------
void CMapProduct::RemoveCriteriaFromProducts()
{

  CObMap::iterator it;
  for (it = begin() ; it != end() ; it++)
  {
    CProduct* product = dynamic_cast<CProduct*>(it->second);
    if (product == NULL)
    {
      continue;
    }

    product->RemoveCriteria();

  }
}


//----------------------------------------
void CMapProduct::GetProductKeysWithCriteria(CStringArray& keys)
{

  CObMap::iterator it;
  for (it = begin() ; it != end() ; it++)
  {
    CProduct* product = dynamic_cast<CProduct*>(it->second);
    if (product == NULL)
    {
      continue;
    }

    if (product->HasCriteriaInfo())
    {
      keys.Insert(it->first);
    }

  }
}


//----------------------------------------
void CMapProduct::Dump(ostream& fOut /* = cerr */)
{

   if (CTrace::IsTrace() == false)
   {
      return;
   }

   fOut << "==> Dump a CMapProduct Object at "<< this << " with " <<  size() << " elements" << endl;

   CMapProduct::iterator it;

   for (it = this->begin() ; it != this->end() ; it++)
   {
      CBratObject *ob = it->second;
      fOut << "CMapProduct Key is = " << (*it).first << endl;
      fOut << "CMapProduct Value is = " << endl;
      ob->Dump(fOut);
   }

   fOut << "==> END Dump a CMapProduct Object at "<< this << " with " <<  size() << " elements" << endl;

}



} // end namespace
